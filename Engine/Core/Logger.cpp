#include "Logger.h"
#include <iostream>

namespace minicell
{
    void logInfo(const char* message)
    {
        std::cout << "[INFO] " << message << '\n';
    }

    void logWarning(const char* message)
    {
        std::cout << "[WARNING] " << message << '\n';
    }

    void logError(const char* message)
    {
        std::cerr << "[ERROR] " << message << '\n';
    }
}