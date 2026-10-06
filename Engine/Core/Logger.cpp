#include <iostream>
#include "Logger.h"

namespace minicell
{
    namespace 
    {
        void logMessage(LogLevel level, const char* message)
        {
            switch (level)
            {
            case LogLevel::Info:
                std::cout << "[INFO] " << message << '\n';
                break;
            case LogLevel::Warning:
                std::cout << "[WARNING] " << message << '\n';
                break;
            case LogLevel::Error:
                std::cerr << "[ERROR] " << message << '\n';
                break;
            }
        }    
    }
    
    void logInfo(const char* message)
    {
        logMessage(LogLevel::Info, message);
    }

    void logWarning(const char* message)
    {
        logMessage(LogLevel::Warning, message);
    }

    void logError(const char* message)
    {
        logMessage(LogLevel::Error, message);
    }
}