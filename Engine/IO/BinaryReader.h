#pragma once

#include "Types.h"

namespace minicell {
    class BinaryReader {
        private:
            const u8*   m_data;
            usize       m_size;
            usize       m_offset = 0;

            bool canRead(usize count) const {
                return m_size - m_offset >= count;
            }
        
        public:
            BinaryReader(const u8* data, usize size)
                : m_data(data), m_size(size)
                {
                }

            //little-Endian {0xCD, 0xAB} -> 0xABCD
            bool readU16(u16& out) {
                if (!canRead(2)) { return false; }  // Not enough bytes left

                const u16 b0 = m_data[m_offset];
                const u16 b1 = m_data[m_offset + 1];

                out = (b0 | (b1 << 8));
                m_offset += 2;
                return true;
            }

            bool readU32(u32& out) {
                if (!canRead(4)) return false;

                const u32 b0 = m_data[m_offset];
                const u32 b1 = m_data[m_offset + 1];
                const u32 b2 = m_data[m_offset + 2];
                const u32 b3 = m_data[m_offset + 3];

                out = (b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
                m_offset += 4;
                return true;
            }

            bool readBytes(u8* dst, usize count) {
                if (!canRead(count)) return false;

                for (usize i=0; i < count; i++) {
                    dst[i] = m_data[m_offset + i];   
                }
                m_offset += count;
                return true;
            }

            usize offset() const { return m_offset; }
            usize remaining() const { return m_size - m_offset; }
    };
}