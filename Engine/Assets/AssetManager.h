#pragma once

#include <vector>
#include "Types.h"
#include "Handle.h"
#include "LoadedAsset.h"

namespace minicell {
    class AssetManager {
        private:
            std::vector<LoadedAsset> m_textures;

        public:
            LoadedAsset loadFromMemory(const u8* data, usize size);
            usize cachedCount() const;

            TextureHandle createTexture(LoadedAsset&& asset);
            const LoadedAsset* tryGet(TextureHandle h) const;

            ~AssetManager();
    };
}