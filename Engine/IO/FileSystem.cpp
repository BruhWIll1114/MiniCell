
#include <fstream>
#include <filesystem>
#include <iterator>
#include "FileSystem.h"

namespace minicell {

    FileResult readTextFile(const std::string& path, std::string& outContents) 
    {
        std::error_code ec;

        // error_code overload: reports failure in ec instead of throwing
        if (!std::filesystem::exists(path, ec)) return FileResult::NotFound;    
        std::ifstream inf { path };
        if (!inf) return FileResult::ReadError;     // unable to access the file (e.g. access denied / file locking)

        // extract the raw characters (preserve format) into a string
        std::string contents { std::istreambuf_iterator<char>(inf), std::istreambuf_iterator<char>() };
        if (inf.bad()) return FileResult::ReadError;

        outContents = std::move(contents);
        return FileResult::Pass;
    }

    FileResult readBinaryFile(const std::string& path, std::vector<u8>& outBytes)
    {
        std::error_code ec;
        if (!std::filesystem::exists(path, ec)) return FileResult::NotFound;

        std::ifstream inf { path, std::ios::binary | std::ios::ate };
        
        const std::streampos end = inf.tellg();
        if (end < 0) return FileResult::ReadError;

        const usize size = static_cast<usize>(end);
        inf.seekg(0);                                   // move to the start pos

        std::vector<u8> buffer(size);
        //if (!inf) return FileResult::ReadError;
        inf.read(reinterpret_cast<char*>(buffer.data()), size);
        if (!inf) return FileResult::ReadError;

        outBytes = std::move(buffer);
        return FileResult::Pass;
    }

    FileResult writeBinaryFile(const std::string& path, const u8* data, usize size) 
    {
        std::ofstream outf { path, std::ios::binary };

        outf.write(reinterpret_cast<const char*>(data), size);
        if (!outf) return FileResult::WriteError;

        return FileResult::Pass;
    }
}