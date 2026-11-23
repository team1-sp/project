#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace engine {

// Text of at most N bytes (UTF-8) stored inside the object, so a component that holds it stays
// trivially copyable (an unmanaged component, D-14) and its pool can be copied as raw memory.
// Unused bytes are always zero, so two equal strings are also equal byte for byte.
template <std::size_t N>
class FixedString {
    static_assert(N > 0 && N <= 255, "the length is stored in one byte");

public:
    static constexpr std::size_t capacity = N;

    // Replaces the text. Text longer than N bytes is refused: the old text stays and the result
    // is false. The Editor refuses such names before they reach the engine.
    constexpr bool assign(std::string_view text) {
        if (text.size() > N) {
            return false;
        }
        chars_ = {};
        for (std::size_t i = 0; i < text.size(); ++i) {
            chars_[i] = text[i];
        }
        size_ = static_cast<std::uint8_t>(text.size());
        return true;
    }

    [[nodiscard]] constexpr std::string_view view() const { return {chars_.data(), size_}; }
    [[nodiscard]] constexpr std::size_t size() const { return size_; }
    [[nodiscard]] constexpr bool empty() const { return size_ == 0; }

    friend constexpr bool operator==(const FixedString&, const FixedString&) = default;

private:
    std::array<char, N> chars_{};
    std::uint8_t size_ = 0;
};

} // namespace engine
