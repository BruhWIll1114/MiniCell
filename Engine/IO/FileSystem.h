#pragma once

#include <string>
#include <vector>
#include "Types.h"

namespace minicell {
    enum class FileResult 
    {
        Pass,
        NotFound,
        ReadError,
        WriteError
    };

    [[nodiscard]] FileResult readTextFile(const std::string& path, std::string& outContents);
    [[nodiscard]] FileResult readBinaryFile(const std::string& path, std::vector<u8>& outBytes);
    [[nodiscard]] FileResult writeBinaryFile(const std::string& path, const u8* data, usize size);
}