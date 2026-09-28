#include <iostream>
#include "Logger.h"

int main()
{
    minicell::logInfo("MiniCell starting...");
    std::cout << "Hello from MiniCell!\n";
    minicell::logInfo("MiniCell shutting down...");
    return 0;
}