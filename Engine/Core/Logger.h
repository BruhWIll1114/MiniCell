#pragma once

namespace minicell
{
    enum class LogLevel
    {
        INFO,
        WARNING,
        ERROR
    };

    void logInfo(const char* message);
    void logWarning(const char* message);
    void logError(const char* message);
}