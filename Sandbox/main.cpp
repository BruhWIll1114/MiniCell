#include <cstdio>
#include "Logger.h"
#include "Assert.h"
#include "Types.h"

int main()
{
    constexpr minicell::u32 kTotalFrames = 120;
    constexpr minicell::u32 kLogInterval = 30;
    
    minicell::logInfo("MiniCell starting...");

    for (minicell::u32 frame = 0; frame < kTotalFrames; ++frame) {
        if (frame % kLogInterval == 0) {
            char buffer[64];
            std::snprintf(buffer, sizeof(buffer), "Frame %d", static_cast<unsigned>(frame));
            minicell::logInfo(buffer);
        }
    }

    minicell::logInfo("MiniCell shutting down...");
    return 0;
}