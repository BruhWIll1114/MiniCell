#pragma once

#include <string>
#include "Types.h"
#include "IPlatform.h"

namespace minicell {

    // Windows implementation of IPlatform;
    class Win32Platform final : public IPlatform{   // final: nothing can inherit from Win32Platform
        public:
            u64 getTicks() const override;
            u64 getTicksPerSecond() const override;
            std::string getExeDir() const override;
    };
}