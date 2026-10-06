#pragma once

namespace minicell
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error
    };

    void logInfo(const char* message);
    void logWarning(const char* message);
    void logError(const char* message);
}