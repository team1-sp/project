#include "aegis/assistant/grounded/content_hash.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

// SHA-256 (FIPS 180-4). A small local implementation avoids a cryptography dependency for a
// content fingerprint (S-5); it is checked against the FIPS test vectors in the unit tests.
namespace aegis::assistant {
namespace {

constexpr std::array<std::uint32_t, 64> roundConstants = {
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U, 0x3956c25bU, 0x59f111f1U, 0x923f82a4U,
    0xab1c5ed5U, 0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U, 0x72be5d74U, 0x80deb1feU,
    0x9bdc06a7U, 0xc19bf174U, 0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU, 0x2de92c6fU,
    0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU, 0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U, 0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU,
    0x53380d13U, 0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U, 0xa2bfe8a1U, 0xa81a664bU,
    0xc24b8b70U, 0xc76c51a3U, 0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U, 0x19a4c116U,
    0x1e376c08U, 0x2748774cU, 0x34b0bcb5U, 0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U, 0x90befffaU, 0xa4506cebU, 0xbef9a3f7U,
    0xc67178f2U};

[[nodiscard]] constexpr std::uint32_t rotateRight(std::uint32_t value, unsigned int bits) {
    return (value >> bits) | (value << (32U - bits));
}

void processBlock(std::array<std::uint32_t, 8>& state, const std::uint8_t* block) {
    std::array<std::uint32_t, 64> words{};
    for (std::size_t index = 0; index < 16; ++index) {
        words[index] = (std::uint32_t{block[index * 4]} << 24U) |
                       (std::uint32_t{block[index * 4 + 1]} << 16U) |
                       (std::uint32_t{block[index * 4 + 2]} << 8U) |
                       std::uint32_t{block[index * 4 + 3]};
    }
    for (std::size_t index = 16; index < 64; ++index) {
        const std::uint32_t s0 = rotateRight(words[index - 15], 7) ^
                                 rotateRight(words[index - 15], 18) ^ (words[index - 15] >> 3U);
        const std::uint32_t s1 = rotateRight(words[index - 2], 17) ^
                                 rotateRight(words[index - 2], 19) ^ (words[index - 2] >> 10U);
        words[index] = words[index - 16] + s0 + words[index - 7] + s1;
    }

    std::array<std::uint32_t, 8> work = state;
    for (std::size_t index = 0; index < 64; ++index) {
        const std::uint32_t sum1 =
            rotateRight(work[4], 6) ^ rotateRight(work[4], 11) ^ rotateRight(work[4], 25);
        const std::uint32_t choice = (work[4] & work[5]) ^ (~work[4] & work[6]);
        const std::uint32_t temp1 = work[7] + sum1 + choice + roundConstants[index] + words[index];
        const std::uint32_t sum0 =
            rotateRight(work[0], 2) ^ rotateRight(work[0], 13) ^ rotateRight(work[0], 22);
        const std::uint32_t majority =
            (work[0] & work[1]) ^ (work[0] & work[2]) ^ (work[1] & work[2]);
        const std::uint32_t temp2 = sum0 + majority;
        work[7] = work[6];
        work[6] = work[5];
        work[5] = work[4];
        work[4] = work[3] + temp1;
        work[3] = work[2];
        work[2] = work[1];
        work[1] = work[0];
        work[0] = temp1 + temp2;
    }
    for (std::size_t index = 0; index < 8; ++index) {
        state[index] += work[index];
    }
}

} // namespace

std::string contentHash(std::string_view content) {
    std::array<std::uint32_t, 8> state = {0x6a09e667U, 0xbb67ae85U, 0x3c6ef372U, 0xa54ff53aU,
                                          0x510e527fU, 0x9b05688cU, 0x1f83d9abU, 0x5be0cd19U};

    std::vector<std::uint8_t> message(content.begin(), content.end());
    const std::uint64_t bitLength = static_cast<std::uint64_t>(content.size()) * 8U;
    message.push_back(0x80U);
    while (message.size() % 64U != 56U) {
        message.push_back(0x00U);
    }
    for (int shift = 56; shift >= 0; shift -= 8) {
        message.push_back(static_cast<std::uint8_t>(bitLength >> static_cast<unsigned int>(shift)));
    }
    for (std::size_t offset = 0; offset < message.size(); offset += 64U) {
        processBlock(state, message.data() + offset);
    }

    constexpr std::string_view digits = "0123456789abcdef";
    std::string hash = "sha256:";
    for (const std::uint32_t word : state) {
        for (int shift = 28; shift >= 0; shift -= 4) {
            hash += digits[(word >> static_cast<unsigned int>(shift)) & 0xFU];
        }
    }
    return hash;
}

} // namespace aegis::assistant
