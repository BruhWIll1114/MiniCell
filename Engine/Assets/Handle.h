#pragma once

#include <limits>
#include "Types.h"

namespace minicell {
    struct TextureTag {};
    struct MeshTag {};

    template <typename Tag>
    struct Handle 
    {
        // assigning the maximum possible value for a 32-bit unsigned integer
        static constexpr u32 kInvalid = std::numeric_limits<u32>::max();

        u32 id = kInvalid;
        bool isValid() const
        {
            return id != kInvalid;
        }

        bool operator==(const Handle&) const = default;
    };

    using TextureHandle = Handle<TextureTag>;
    using MeshHandle = Handle<MeshTag>;

    static_assert(sizeof(TextureHandle) == sizeof(u32));
}