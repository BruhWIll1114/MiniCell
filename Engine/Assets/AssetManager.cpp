#include <cstdio>
#include <utility>
#include <vector>
#include "Types.h"
#include "Logger.h"
#include "AssetManager.h"

namespace minicell
{
    // copies the raw bytes once into a new asset and returns it by implicit move
    LoadedAsset AssetManager::loadFromMemory(const u8* data, usize size)
    {
        LoadedAsset asset;
        asset.bytes.assign(data, data + size);      // copy size bytes from memory into the asset's own heap buffer
        return asset;
    }
    
    usize AssetManager::cachedCount() const
    {
        return m_textures.size();
    }

    TextureHandle AssetManager::createTexture(LoadedAsset&& asset)      // Takes in a rvalue reference and pass it to the cache, caller's asset left empty
    {
        const TextureHandle h{ static_cast<u32>(m_textures.size()) };
        m_textures.push_back(std::move(asset));
        return h;
    }

    const LoadedAsset* AssetManager::tryGet(TextureHandle h) const
    {
        if (!h.isValid() || h.id >= m_textures.size()) return nullptr;

        //pointer invalid after the next createTexture() if the vector reallocates
        return (&m_textures[h.id]);

    }

    AssetManager::~AssetManager() 
    {
        char msg[64];
        std::snprintf(msg, sizeof(msg), "AssetManager destroyed: cached=%zu", cachedCount());
        minicell::logInfo(msg);
    }
}