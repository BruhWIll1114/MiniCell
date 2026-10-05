#pragma once

#include "Types.h"

namespace minicell {
    struct LoadedAsset {
        public:
            std::vector<u8> bytes;

            usize byteSize() const {
                return bytes.size(); 
            }

            LoadedAsset() = default;                                // default constructor
            LoadedAsset(const LoadedAsset&) = delete;               // copy constructor e.g. LoadedAsset b(a);
            LoadedAsset& operator=(const LoadedAsset&) = delete;    // copy assignment  e.g. LoadedAsset b; b = a;
            LoadedAsset(LoadedAsset&&) = default;                   // move constructor e.g. LoadedAsset b = std::move(a);
            LoadedAsset& operator=(LoadedAsset&&) = default;        // move assignment  e.g. LoadedAsset b; b = std::move(a);
        };
}
