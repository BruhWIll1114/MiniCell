#pragma once

#include <memory>
#include <string>
#include "Types.h"

namespace minicell {
    class IPlatform {
        public:
            virtual ~IPlatform() = default;

            virtual u64 getTicks() const = 0;
            virtual u64 getTicksPerSecond() const = 0;
            virtual std::string getExeDir() const = 0;
    };

    std::unique_ptr<IPlatform> createPlatform();
    // Using unique_ptr instead of raw pointer provides ownership to the caller
    // in respond to delete the platform when the unique_ptr goes out of scope.
}