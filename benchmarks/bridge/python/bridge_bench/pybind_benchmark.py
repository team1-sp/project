import argparse
import json
import math
import sys
import threading
import time

import aegis_bridge_pybind


def percentile(sorted_values: list[float], quantile: float) -> float:
    if not sorted_values:
        return 0.0
    position = quantile * (len(sorted_values) - 1)
    lower = math.floor(position)
    upper = math.ceil(position)
    fraction = position - lower
    return (
        sorted_values[lower] + (sorted_values[upper] - sorted_values[lower]) * fraction
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("copy", "view"), required=True)
    parser.add_argument("--clients", type=int, default=1)
    parser.add_argument("--iterations", type=int, default=10_000)
    parser.add_argument("--warmup", type=int, default=1_000)
    parser.add_argument("--payload-bytes", type=int, default=256)
    parser.add_argument("--timeout-ms", type=int, default=2_000)
    args = parser.parse_args()
    if min(args.clients, args.iterations, args.warmup, args.payload_bytes) < 1:
        parser.error("counts and payload size must be positive")

    exchange = (
        aegis_bridge_pybind.exchange_copy
        if args.mode == "copy"
        else aegis_bridge_pybind.exchange_view
    )
    ready = threading.Barrier(args.clients + 1)
    start = threading.Barrier(args.clients + 1)
    latencies: list[float] = []
    errors: list[str] = []
    result_lock = threading.Lock()

    def client(client_index: int) -> None:
        game_instance_id = client_index + 1
        payload = bytes(
            (index + client_index) % 251 for index in range(args.payload_bytes)
        )
        local_latencies: list[float] = []
        local_errors: list[str] = []
        try:
            for sequence in range(args.warmup):
                reply = exchange(game_instance_id, sequence, payload)
                if reply != (game_instance_id, sequence, 7):
                    local_errors.append(
                        "warm-up response association or action mismatch"
                    )
                    break
        except Exception as exception:  # noqa: BLE001 - benchmark reports backend failures
            local_errors.append(f"warm-up: {exception}")
        ready.wait()
        start.wait()
        if not local_errors:
            for iteration in range(args.iterations):
                sequence = args.warmup + iteration
                before = time.perf_counter_ns()
                try:
                    reply = exchange(game_instance_id, sequence, payload)
                except Exception as exception:  # noqa: BLE001 - benchmark reports backend failures
                    local_errors.append(str(exception))
                    continue
                after = time.perf_counter_ns()
                if reply != (game_instance_id, sequence, 7):
                    local_errors.append("response association or action mismatch")
                    continue
                local_latencies.append((after - before) / 1_000.0)
        with result_lock:
            latencies.extend(local_latencies)
            errors.extend(local_errors)

    threads = [
        threading.Thread(target=client, args=(index,)) for index in range(args.clients)
    ]
    for thread in threads:
        thread.start()
    ready.wait()
    measured_start = time.perf_counter_ns()
    start.wait()
    for thread in threads:
        thread.join()
    elapsed_seconds = (time.perf_counter_ns() - measured_start) / 1_000_000_000.0
    latencies.sort()
    result = {
        "transport": f"pybind11-{args.mode}",
        "endpoint": "in-process",
        "clients": args.clients,
        "payload_bytes": args.payload_bytes,
        "warmup_iterations_per_client": args.warmup,
        "measured_iterations_per_client": args.iterations,
        "successful_exchanges": len(latencies),
        "errors": len(errors),
        "elapsed_seconds": elapsed_seconds,
        "throughput_exchanges_per_second": len(latencies) / elapsed_seconds,
        "median_rtt_microseconds": percentile(latencies, 0.50),
        "p95_rtt_microseconds": percentile(latencies, 0.95),
        "p99_rtt_microseconds": percentile(latencies, 0.99),
    }
    print(json.dumps(result, indent=2))
    for error in errors:
        print(f"exchange failed: {error}", file=sys.stderr)
    return 0 if latencies and not errors else 2


if __name__ == "__main__":
    raise SystemExit(main())
