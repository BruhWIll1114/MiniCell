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
            case LogLevel::INFO:
                std::cout << "[INFO] " << message << '\n';
                break;
            case LogLevel::WARNING:
                std::cout << "[WARNING] " << message << '\n';
                break;
            case LogLevel::ERROR:
                std::cerr << "[ERROR] " << message << '\n';
                break;
            }
        }    
    }
    
    void logInfo(const char* message)
    {
        logMessage(LogLevel::INFO, message);
    }

    void logWarning(const char* message)
    {
        logMessage(LogLevel::WARNING, message);
    }

    void logError(const char* message)
    {
        logMessage(LogLevel::ERROR, message);
    }
}