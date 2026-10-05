#include <cstdio>
#include <string>
#include <cstring>
#include <memory>
#include "Logger.h"
#include "MCAssert.h"
#include "Types.h"
#include "RingBuffer.h"
#include "ScopeGuard.h"
#include "BinaryReader.h"
#include "LinearAllocator.h"
#include "AssetManager.h"

enum class ParseResult {
    PASS,
    TRUNCATED,
    BAD_MAGIC,
};

ParseResult parseAssetHeader(const minicell::u8* data, minicell::usize size, minicell::u16& version, minicell::u32& payloadSize) {
    minicell::BinaryReader reader (data, size);
    minicell::u8 magic[4];

    if (!reader.readBytes(magic, 4)) return ParseResult::TRUNCATED;
    if (std::memcmp(magic, "MCLL", sizeof(magic)) != 0) return ParseResult::BAD_MAGIC;
    if (!reader.readU16(version)) return ParseResult::TRUNCATED;
    if (!reader.readU32(payloadSize)) return ParseResult::TRUNCATED;
    return ParseResult::PASS;
}

void testAsset(const minicell::u8* data, minicell::usize size) {
    minicell::u16 version       = 0;
    minicell::u32 payloadSize   = 0;

    const ParseResult res = parseAssetHeader(data, size, version, payloadSize);

    char msg[64];
    if (res == ParseResult::PASS) {
        std::snprintf(msg, sizeof(msg), "asset version=%u payloadSize=%u", static_cast<unsigned>(version), static_cast<unsigned>(payloadSize));
        minicell::logInfo(msg);    
        
    } else if (res == ParseResult::BAD_MAGIC) {
        minicell::logError("asset bad magic");
    } else {
        minicell::logError("asset truncated");
    }
}

void runArenaFrames(minicell::LinearAllocator& arena) {
    for (minicell::u32 frame = 0; frame < 100; ++frame) {
        void* first = nullptr;
        for (minicell::usize i = 0; i < 10; ++i) {
            void* p = arena.allocate(32);
            if (i == 0) { first = p; }
        }

        if (frame % 25 == 0) {
            char msg[64];
            std::snprintf(msg, sizeof(msg), "frame=%u first=%p used=%zu", frame, first, arena.used());
            minicell::logInfo(msg);
        }

        arena.reset();
    }
}

int main()
{
    constexpr minicell::u32 totalFrames = 120;
    constexpr minicell::u32 logInterval = 60;
    
    constexpr minicell::usize capacity = 64;
    minicell::RingBuffer<f32, capacity> buffer;

    // constexpr minicell::u8 kAsset[]    = { 'M','C','L','L', 1, 0, 0x10, 0, 0, 0 };
    // constexpr minicell::u8 kBadMagic[] = { 'X','C','L','L', 1, 0, 0x10, 0, 0, 0 };

    {
        minicell::ScopeGuard guard([]() {
            minicell::logInfo("leave scope"); 
        });
        minicell::logInfo("enter scope");
    }

    minicell::logInfo("MiniCell starting...");

    // testAsset(kAsset, sizeof(kAsset));
    // testAsset(kBadMagic, sizeof(kBadMagic));

    // minicell::LinearAllocator arena;
    // runArenaFrames(arena);

    const minicell::u8 array[] = {1, 2, 3, 4};
    auto assets = std::make_unique<minicell::AssetManager>();
    minicell::LoadedAsset a = assets->loadFromMemory(array, sizeof(array));

    char msg[64];
    std::snprintf(msg, sizeof(msg), "asset bytes=%zu", a.byteSize());
    minicell::logInfo(msg);

    std::snprintf(msg, sizeof(msg), "before move data=%p", a.bytes.data());
    minicell::logInfo(msg);

    minicell::LoadedAsset b = std::move(a);
    std::snprintf(msg, sizeof(msg), "after move data=%p; sourceBytes=%zu", b.bytes.data(), a.byteSize());
    minicell::logInfo(msg);

    assets->keep(std::move(a));

    for (minicell::u32 frame = 0; frame < totalFrames; ++frame) {

        buffer.push(static_cast<f32>(frame));

        if ((frame + 1) % logInterval != 0) {
            continue;
        }

        f32 min = buffer[0];
        f32 max = buffer[0];
        for (minicell::usize i = 1; i < buffer.size(); ++i) {
            const f32 v = buffer[i];
            if (v < min) {
                min = v;
            }
            if (v > max) {
                max = v;
            }
        }

        char msg[64];
        std::snprintf(msg, sizeof(msg), "min: %.1f max: %.1f", static_cast<f64>(min), static_cast<f64>(max));
        minicell::logInfo(msg);    
    }

    assets.reset();

    minicell::logInfo("MiniCell shutting down...");
    return 0;
}