#include <cstdio>
#include <utility>
#include <vector>
#include "Types.h"
#include "Logger.h"
#include "AssetManager.h"

namespace minicell
{
    // copies the raw bytes once into a new asset and returns it by implicit move
    LoadedAsset AssetManager::loadFromMemory(const u8* data, usize size)    {
        LoadedAsset asset;
        asset.bytes.assign(data, data + size);      // copy size bytes from memory into the asset's own heap buffer
        return asset;
    }

    void AssetManager::keep(LoadedAsset&& asset)    // Takes in a rvalue reference and pass it to the cache, caller's asset left empty
    {
        m_cache.push_back(std::move(asset));
    }
    
    usize AssetManager::cachedCount() const
    {
        return m_cache.size();
    }

    AssetManager::~AssetManager() 
    {
        char msg[64];
        std::snprintf(msg, sizeof(msg), "AssetManager destroyed: cached=%zu", cachedCount());
        minicell::logInfo(msg);
    }
}