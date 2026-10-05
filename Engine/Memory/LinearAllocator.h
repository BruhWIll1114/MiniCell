#pragma once

#include <array>
#include "Types.h"
#include "MCAssert.h"
#include "Logger.h"

namespace minicell {
    class LinearAllocator : NonCopyable {
        private:
            static constexpr usize kCapacity = 4096;
            static constexpr usize kMaxAlignment = 16;
            alignas(kMaxAlignment) std::array<u8, kCapacity> m_data{};
            usize m_offset = 0;

        public:
            void* allocate(usize bytes, usize alignment = 8) {
                if ((alignment & (alignment - 1)) != 0) {
                    logError("Alignment must be a power of two");
                    return nullptr;
                }

                if ((alignment == 0) || (alignment > kMaxAlignment)) {
                    logError("Alignment under/exceed limit");
                    return nullptr;
                }

                const usize aligned = (m_offset + alignment - 1) & ~(alignment - 1);    // Round up the offset to the next usable position
                if ((aligned <= kCapacity) && (kCapacity - aligned >= bytes)) {         // If remain spacing can fulfill bytes
                    m_offset = aligned + bytes;
                    return m_data.data() + aligned;
                }
                logError("No spacing left");
                return nullptr;
            }   

            void reset() { m_offset = 0; }

            usize used() const { return m_offset; }
            usize capacity() const { return kCapacity; }
    };
}