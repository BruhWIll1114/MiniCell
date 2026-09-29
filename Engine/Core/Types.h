#pragma once

#include <cstdint>  // for fixed-width integers
#include <cstddef>

namespace minicell
{
    using u8  = std::uint8_t;   // unsigned 8-bit integer (0 to 255)
    using u16 = std::uint16_t;
    using u32 = std::uint32_t;
    using u64 = std::uint64_t;

    using i8  = std::int8_t;    // signed 8-bit integer (-128 to 127)
    using i16 = std::int16_t;
    using i32 = std::int32_t;
    using i64 = std::int64_t;

    using usize = std::size_t;
}

using f32 = float;  // float has size of 32 bits
using f64 = double; // double has size of 64 bits
struct NonCopyable {
    protected:
        NonCopyable() = default;
    public:
        NonCopyable(const NonCopyable&) = delete;
        NonCopyable& operator=(const NonCopyable&) = delete;
};