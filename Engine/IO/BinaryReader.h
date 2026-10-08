#pragma once

#include <type_traits>
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

            template <typename T>
            bool read(T& out) 
            {
                static_assert(std::is_unsigned_v<T>, "read<T> needs an unsigned integer");
                if (!canRead(sizeof(T))) { return false; }

                out = 0;
                for (usize i=0; i < sizeof(T); ++i) 
                {
                    //the inner static_cast widen the byte to T before <<. The outer one cast the shifted int back to T.
                    out |= static_cast<T>(static_cast<T>(m_data[m_offset + i]) << (8 * i));
                };

                m_offset += sizeof(T);
                return true;
            }

            //little-Endian {0xCD, 0xAB} -> 0xABCD
            bool readU16(u16& out)
            {
                return read(out);

                // Procedure of old readU16
                // if (!canRead(2)) { return false; }  // Not enough bytes left

                // const u16 b0 = m_data[m_offset];
                // const u16 b1 = m_data[m_offset + 1];

                // out = (b0 | (b1 << 8));
                // m_offset += 2;
                // return true;
            }

            bool readU8(u8& out)
            {
                return read(out);
            }

            bool readU32(u32& out)
            {
                return read(out);
            }

            bool readU64(u64& out)
            {
                return read(out);
            }

            bool readBytes(u8* dst, usize count)
            {
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