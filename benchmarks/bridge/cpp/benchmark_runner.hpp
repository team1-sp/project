#pragma once

#include <algorithm>
#include <barrier>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace bridge_benchmark {

struct Configuration {
    std::string endpoint;
    std::size_t clients = 1;
    std::size_t iterations = 10'000;
    std::size_t warmupIterations = 1'000;
    std::size_t payloadBytes = 256;
    int timeoutMilliseconds = 2'000;
};

struct Reply {
    std::uint64_t gameInstanceId = 0;
    std::uint64_t sequence = 0;
    std::int32_t actionId = 0;
    std::string error;
};

using Exchange = std::function<Reply(std::uint64_t, std::uint64_t, const std::vector<std::byte>&)>;
using ExchangeFactory = std::function<Exchange(std::size_t)>;

inline std::size_t parseSize(std::string_view value, std::string_view option) {
    std::size_t consumed = 0;
    const auto parsed = std::stoull(std::string(value), &consumed);
    if (consumed != value.size() || parsed == 0) {
        throw std::invalid_argument(std::string(option) + " must be a positive integer");
    }
    return static_cast<std::size_t>(parsed);
}

inline Configuration parseArguments(int argc, char** argv, std::string defaultEndpoint) {
    Configuration config;
    config.endpoint = std::move(defaultEndpoint);
    for (int index = 1; index < argc; ++index) {
        const std::string_view option(argv[index]);
        if (index + 1 >= argc) {
            throw std::invalid_argument("missing value for " + std::string(option));
        }
        const std::string_view value(argv[++index]);
        if (option == "--endpoint") {
            config.endpoint = value;
        } else if (option == "--clients") {
            config.clients = parseSize(value, option);
        } else if (option == "--iterations") {
            config.iterations = parseSize(value, option);
        } else if (option == "--warmup") {
            config.warmupIterations = parseSize(value, option);
        } else if (option == "--payload-bytes") {
            config.payloadBytes = parseSize(value, option);
        } else if (option == "--timeout-ms") {
            config.timeoutMilliseconds = static_cast<int>(parseSize(value, option));
        } else {
            throw std::invalid_argument("unknown option: " + std::string(option));
        }
    }
    return config;
}

inline double percentile(const std::vector<double>& sorted, double quantile) {
    if (sorted.empty()) {
        return 0.0;
    }
    const double position = quantile * static_cast<double>(sorted.size() - 1);
    const auto lower = static_cast<std::size_t>(std::floor(position));
    const auto upper = static_cast<std::size_t>(std::ceil(position));
    const double fraction = position - static_cast<double>(lower);
    return sorted[lower] + ((sorted[upper] - sorted[lower]) * fraction);
}

inline int run(const std::string& transport, const Configuration& config,
               const ExchangeFactory& createExchange) {
    using Clock = std::chrono::steady_clock;
    std::vector<double> latenciesMicroseconds;
    latenciesMicroseconds.reserve(config.clients * config.iterations);
    std::vector<std::string> errors;
    std::mutex resultMutex;
    std::barrier ready(static_cast<std::ptrdiff_t>(config.clients + 1));
    std::barrier start(static_cast<std::ptrdiff_t>(config.clients + 1));
    std::vector<std::thread> workers;
    workers.reserve(config.clients);

    for (std::size_t clientIndex = 0; clientIndex < config.clients; ++clientIndex) {
        workers.emplace_back([&, clientIndex] {
            std::vector<double> localLatencies;
            localLatencies.reserve(config.iterations);
            std::vector<std::string> localErrors;
            const std::uint64_t gameInstanceId = static_cast<std::uint64_t>(clientIndex + 1);
            std::vector<std::byte> payload(config.payloadBytes);
            for (std::size_t byteIndex = 0; byteIndex < payload.size(); ++byteIndex) {
                payload[byteIndex] = static_cast<std::byte>((byteIndex + clientIndex) % 251);
            }

            try {
                Exchange exchange = createExchange(clientIndex);
                for (std::size_t iteration = 0; iteration < config.warmupIterations; ++iteration) {
                    const Reply reply = exchange(gameInstanceId, iteration, payload);
                    if (!reply.error.empty()) {
                        localErrors.push_back("warm-up: " + reply.error);
                        break;
                    }
                }

                ready.arrive_and_wait();
                start.arrive_and_wait();
                for (std::size_t iteration = 0; iteration < config.iterations; ++iteration) {
                    const std::uint64_t sequence =
                        static_cast<std::uint64_t>(config.warmupIterations + iteration);
                    const auto before = Clock::now();
                    const Reply reply = exchange(gameInstanceId, sequence, payload);
                    const auto after = Clock::now();
                    if (!reply.error.empty()) {
                        localErrors.push_back(reply.error);
                        continue;
                    }
                    if (reply.gameInstanceId != gameInstanceId || reply.sequence != sequence ||
                        reply.actionId != 7) {
                        localErrors.emplace_back("response association or action mismatch");
                        continue;
                    }
                    localLatencies.push_back(
                        std::chrono::duration<double, std::micro>(after - before).count());
                }
            } catch (const std::exception& exception) {
                ready.arrive_and_drop();
                start.arrive_and_drop();
                localErrors.push_back(exception.what());
            }

            std::scoped_lock lock(resultMutex);
            latenciesMicroseconds.insert(latenciesMicroseconds.end(), localLatencies.begin(),
                                         localLatencies.end());
            errors.insert(errors.end(), localErrors.begin(), localErrors.end());
        });
    }

    ready.arrive_and_wait();
    const auto measuredStart = Clock::now();
    start.arrive_and_wait();
    for (auto& worker : workers) {
        worker.join();
    }
    const auto measuredEnd = Clock::now();

    std::sort(latenciesMicroseconds.begin(), latenciesMicroseconds.end());
    const double elapsedSeconds =
        std::chrono::duration<double>(measuredEnd - measuredStart).count();
    const double throughput =
        elapsedSeconds > 0.0 ? static_cast<double>(latenciesMicroseconds.size()) / elapsedSeconds
                             : 0.0;
    std::cout << "{\n"
              << "  \"transport\": \"" << transport << "\",\n"
              << "  \"endpoint\": \"" << config.endpoint << "\",\n"
              << "  \"clients\": " << config.clients << ",\n"
              << "  \"payload_bytes\": " << config.payloadBytes << ",\n"
              << "  \"warmup_iterations_per_client\": " << config.warmupIterations << ",\n"
              << "  \"measured_iterations_per_client\": " << config.iterations << ",\n"
              << "  \"successful_exchanges\": " << latenciesMicroseconds.size() << ",\n"
              << "  \"errors\": " << errors.size() << ",\n"
              << "  \"elapsed_seconds\": " << elapsedSeconds << ",\n"
              << "  \"throughput_exchanges_per_second\": " << throughput << ",\n"
              << "  \"median_rtt_microseconds\": " << percentile(latenciesMicroseconds, 0.50)
              << ",\n"
              << "  \"p95_rtt_microseconds\": " << percentile(latenciesMicroseconds, 0.95) << ",\n"
              << "  \"p99_rtt_microseconds\": " << percentile(latenciesMicroseconds, 0.99)
              << "\n}\n";
    for (const std::string& error : errors) {
        std::cerr << "exchange failed: " << error << '\n';
    }
    return errors.empty() && !latenciesMicroseconds.empty() ? 0 : 2;
}

} // namespace bridge_benchmark
