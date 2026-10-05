#pragma once

#include <vector>
#include "Types.h"
#include "LoadedAsset.h"

namespace minicell {
    class AssetManager {
        private:
            std::vector<LoadedAsset> m_cache;

        public:
            LoadedAsset loadFromMemory(const u8* data, usize size);
            void keep(LoadedAsset&& asset);
            usize cachedCount() const;

            ~AssetManager();
    };
}