#pragma once

#include <functional>
#include <utility>      // for std::move

namespace minicell {
    class ScopeGuard {

        private:
            std::function<void()> m_func;

        public:
            ScopeGuard(std::function<void()> func)
                : m_func(std::move(func))
            {
                
            }

            ~ScopeGuard()
            { 
                if (m_func) {
                    m_func();
                }
            }
    };
}