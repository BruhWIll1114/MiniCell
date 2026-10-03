#include <cstdio>
#include <string>
#include "Logger.h"
#include "Assert.h"
#include "Types.h"
#include "RingBuffer.h"
#include "ScopeGuard.h"

int main()
{
    constexpr minicell::u32 totalFrames = 120;
    constexpr minicell::u32 logInterval = 60;
    
    constexpr minicell::usize capacity = 64;
    minicell::RingBuffer<f32, capacity> buffer;

    {
        minicell::ScopeGuard guard([]() {
            minicell::logInfo("leave scope"); 
        });
        minicell::logInfo("enter scope");
    }

    minicell::logInfo("MiniCell starting...");

    for (minicell::u32 frame = 0; frame < totalFrames; ++frame) {

        buffer.push(static_cast<f32>(frame));

        if ((frame + 1) % logInterval != 0) {
            continue;
        }

        f32 min = buffer[0];
        f32 max = buffer[0];
        for (minicell::usize i = 1; i < buffer.size(); ++i) {
            const f32 v = buffer[i];
            if (v < min) {
                min = v;
            }
            if (v > max) {
                max = v;
            }
        }

        char msg[64];
        std::snprintf(msg, sizeof(msg), "min: %.1f max: %.1f", static_cast<f64>(min), static_cast<f64>(max));
        minicell::logInfo(msg);    
    }

    minicell::logInfo("MiniCell shutting down...");
    return 0;
}