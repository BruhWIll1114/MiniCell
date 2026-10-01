#pragma once

#include <array>
#include "Types.h"

namespace minicell {
    template <typename T, usize Capacity>
    struct RingBuffer {
        private:
            usize m_head = 0;
            usize m_count = 0;
            std::array<T, Capacity> m_data{};
        
        public:
            void push(T value) {
                m_data[m_head] = value;
                m_head = (m_head + 1) % Capacity;
                if (m_count < Capacity) {
                    ++m_count;
                }
            }

            usize size() const {
                return m_count;
            }

            bool empty() const {
                return m_count == 0;
            }

            T operator[](usize i) const {
                const usize start = (m_count == Capacity) ? m_head : 0;
                return m_data[(start + i) % Capacity];
            }
    };
}