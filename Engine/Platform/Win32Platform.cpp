#define NOMINMAX                //trims rarely used APIs and stops min/max macros clashing with std::min/max
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <filesystem>
#include "Types.h"
#include "Logger.h"
#include "Win32Platform.h"

namespace minicell {

    // Reads the Window performance counter
    u64 Win32Platform::getTicks() const 
    {
        LARGE_INTEGER i;
        QueryPerformanceCounter(&i);
        return static_cast<u64>(i.QuadPart);
    }

    // Reads the performance counter's fixed frequency (ticks per seconds); ms = (end - start) * 1000.0 / ticksPerSecond
    u64 Win32Platform::getTicksPerSecond() const 
    {
        LARGE_INTEGER i;
        auto res = QueryPerformanceFrequency(&i);
        return static_cast<u64>(i.QuadPart);
    }

    // Gets the full .exe path from Windows and strips the filename
    std::string Win32Platform::getExeDir() const 
    {
        wchar_t buffer[MAX_PATH];
        DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        if (len == 0) 
        {
            minicell::logError("No file path return");
            return "";
        }
        auto path = std::filesystem::path(buffer);
        return (path.parent_path().string());
    }

    std::unique_ptr<IPlatform> createPlatform() 
    {
        return std::make_unique<Win32Platform>();
    }

}