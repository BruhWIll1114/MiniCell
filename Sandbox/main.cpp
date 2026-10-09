#include <cstdio>
#include <string>
#include <cstring>
#include <memory>
#include <vector>
#include <array>
#include <atomic>
#include "Logger.h"
#include "MCAssert.h"
#include "Types.h"
#include "RingBuffer.h"
#include "ScopeGuard.h"
#include "BinaryReader.h"
#include "LinearAllocator.h"
#include "AssetManager.h"
#include "IPlatform.h"
#include "FileSystem.h"
#include "Handle.h"
#include "JobSystem.h"

// ---- Test section switches ----
constexpr bool kTestScopeGuard    = true;
constexpr bool kTestAssetHeader   = true;
constexpr bool kTestFileSystem    = false;
constexpr bool kTestTextureFile   = true;
constexpr bool kTestTextFile      = true;
constexpr bool kTestWin32Platform = false;
constexpr bool kTestAssetManager  = true;
constexpr bool kTestRingBuffer    = false;
constexpr bool kTestHandle        = false;
constexpr bool kRunRacyExperiment = false;

void testParallelSum()
{
    constexpr minicell::usize kWorkerCount = 4;

    std::array<minicell::u8, 10000> data{};
    for (minicell::usize i = 0; i < data.size(); ++i) {
        data[i] = static_cast<minicell::u8>(i % 256);
    }

    minicell::u64 referenceTotal = 0;
    for (minicell::u8 value : data) {
        referenceTotal += value;
    }

    std::unique_ptr<minicell::IPlatform> platform = minicell::createPlatform();
    minicell::JobSystem jobs(kWorkerCount);
    const minicell::u64 ticksPerSecond = platform->getTicksPerSecond();

    // Wake every worker before measuring so thread startup does not skew one version.
    for (minicell::usize i = 0; i < kWorkerCount; ++i) {
        jobs.submit([] {});
    }
    jobs.waitIdle();

    std::array<minicell::u64, kWorkerCount> partials{};
    const minicell::u64 partialStart = platform->getTicks();
    for (minicell::usize chunk = 0; chunk < kWorkerCount; ++chunk) {
        jobs.submit([chunk, &data, &partials] {
            const minicell::usize chunkSize  = data.size() / partials.size();
            const minicell::usize begin      = chunk * chunkSize;
            const minicell::usize end        = begin + chunkSize;

            minicell::u64 sum = 0;
            for (minicell::usize i = begin; i < end; ++i) {
                sum += data[i];
            }
            partials[chunk] = sum;
        });
    }
    jobs.waitIdle();
    const minicell::u64 partialEnd = platform->getTicks();

    minicell::u64 partialTotal = 0;
    for (minicell::u64 partial : partials) {
        partialTotal += partial;
    }

    std::atomic<minicell::u64> atomicTotal{0};
    const minicell::u64 atomicStart = platform->getTicks();
    for (minicell::usize chunk = 0; chunk < kWorkerCount; ++chunk) {
        jobs.submit([chunk, &data, &atomicTotal] {
            const minicell::usize chunkSize = data.size() / kWorkerCount;
            const minicell::usize begin = chunk * chunkSize;
            const minicell::usize end = begin + chunkSize;

            for (minicell::usize i = begin; i < end; ++i) {
                atomicTotal.fetch_add(data[i], std::memory_order_relaxed);
            }
        });
    }
    jobs.waitIdle();
    const minicell::u64 atomicEnd = platform->getTicks();

    const double partialMilliseconds =
        (partialEnd - partialStart) * 1000.0 / ticksPerSecond;
    const double atomicMilliseconds =
        (atomicEnd - atomicStart) * 1000.0 / ticksPerSecond;

    std::printf(
        "partials=%llu reference=%llu time=%.3f ms\n",
        static_cast<unsigned long long>(partialTotal),
        static_cast<unsigned long long>(referenceTotal),
        partialMilliseconds);
    std::printf(
        "atomic=%llu reference=%llu time=%.3f ms\n",
        static_cast<unsigned long long>(atomicTotal.load(std::memory_order_relaxed)),
        static_cast<unsigned long long>(referenceTotal),
        atomicMilliseconds);

    if constexpr (kRunRacyExperiment) {
        for (minicell::usize trial = 0; trial < 10; ++trial) {
            minicell::u64 racyTotal = 0;
            for (minicell::usize chunk = 0; chunk < kWorkerCount; ++chunk) {
                jobs.submit([chunk, &data, &racyTotal] {
                    const minicell::usize chunkSize = data.size() / kWorkerCount;
                    const minicell::usize begin = chunk * chunkSize;
                    const minicell::usize end = begin + chunkSize;

                    for (minicell::usize i = begin; i < end; ++i) {
                        racyTotal += data[i]; // Intentional data race for this experiment.
                    }
                });
            }
            jobs.waitIdle();
            std::printf(
                "racy trial %zu: %llu (undefined behavior)\n",
                trial + 1,
                static_cast<unsigned long long>(racyTotal));
        }
    }

    std::printf(
        "Per-chunk partials avoid contention on one shared atomic variable.\n");
}

// -- AssetManager Section --
static int g_amFailures = 0;

static void amCheck(bool ok, const char* name) {
    char msg[128];
    std::snprintf(msg, sizeof(msg), "%s: %s", ok ? "PASS" : "FAIL", name);
    if (ok) minicell::logInfo(msg);
    else { minicell::logError(msg); ++g_amFailures; }
}

void testAssetManager(const std::string& assetDir) {
    using minicell::AssetManager;
    using minicell::LoadedAsset;
    using minicell::TextureHandle;
    using minicell::FileResult;

    auto assets = std::make_unique<AssetManager>();

    std::vector<minicell::u8> bytes;
    const FileResult r = minicell::readBinaryFile(assetDir + "tex.mc", bytes);
    amCheck(r == FileResult::Pass, "read tex.mc");
    amCheck(bytes.size() == 18, "tex.mc is 18 bytes");

    LoadedAsset asset = assets->loadFromMemory(bytes.data(), bytes.size());
    TextureHandle h = assets->createTexture(std::move(asset));

    char msg[64];
    std::snprintf(msg, sizeof(msg), "texture handle=%u bytes=18", static_cast<unsigned>(h.id));
    minicell::logInfo(msg);

    const LoadedAsset* got = assets->tryGet(h);
    amCheck(got != nullptr, "valid handle is non-null");
    amCheck(got != nullptr && got->byteSize() == 18, "valid handle byteSize==18");
    amCheck(h.id == 0, "first handle id is 0");

    amCheck(assets->tryGet(TextureHandle{}) == nullptr, "default handle returns nullptr");
    amCheck(assets->tryGet(TextureHandle{99}) == nullptr, "handle 99 returns nullptr");

    std::snprintf(msg, sizeof(msg), "asset manager tests done, failures=%d", g_amFailures);
    minicell::logInfo(msg);
}

// -- Handle Section --
static int g_handleFailures = 0;

static void handleCheck(bool ok, const char* name) {
    char msg[128];
    std::snprintf(msg, sizeof(msg), "%s: %s", ok ? "PASS" : "FAIL", name);
    if (ok) minicell::logInfo(msg);
    else { minicell::logError(msg); ++g_handleFailures; }
}

void testHandle() {
    using minicell::TextureHandle;
    using minicell::MeshHandle;

    TextureHandle a{};
    handleCheck(a.isValid() == false, "default TextureHandle is invalid");
    handleCheck(a.id == TextureHandle::kInvalid, "default id is kInvalid");

    TextureHandle b{0};
    handleCheck(b.isValid() == true, "id 0 is valid");

    TextureHandle c{5};
    handleCheck(c.isValid() == true, "id 5 is valid");
    handleCheck(b == c ? false : true, "id 0 != id 5");
    handleCheck(TextureHandle{5} == c, "same id compares equal");

    MeshHandle m{};
    handleCheck(m.isValid() == false, "default MeshHandle is invalid");

    char msg[64];
    std::snprintf(msg, sizeof(msg), "handle tests done, failures=%d", g_handleFailures);
    minicell::logInfo(msg);
}

// -- File System Section --
static int g_fsFailures = 0;

static void fsCheck(bool ok, const char* name) {
    char msg[128];
    std::snprintf(msg, sizeof(msg), "%s: %s", ok ? "PASS" : "FAIL", name);
    if (ok) {
        minicell::logInfo(msg);
    } else {
        minicell::logError(msg);
        ++g_fsFailures;
    }
}

void testFileSystem(const std::string& dir) {
    using minicell::FileResult;

    {
        std::string text = "sentinel";
        const FileResult r = minicell::readTextFile(dir + "/does_not_exist.txt", text);
        fsCheck(r == FileResult::NotFound, "text: missing file returns NotFound");
        fsCheck(text == "sentinel", "text: out-param unchanged on failure");
    }

    {
        std::vector<minicell::u8> bytes = { 0xAB };
        const FileResult r = minicell::readBinaryFile(dir + "/does_not_exist.bin", bytes);
        fsCheck(r == FileResult::NotFound, "binary: missing file returns NotFound");
        fsCheck(bytes.size() == 1 && bytes[0] == 0xAB, "binary: out-param unchanged on failure");
    }

    {
        const minicell::u8 data[10] = { 0x00, 0x0A, 0x0D, 0xFF, 1, 2, 3, 4, 5, 0 };
        const std::string path = dir + "/fs_roundtrip.bin";

        fsCheck(minicell::writeBinaryFile(path, data, sizeof(data)) == FileResult::Pass, "write 10 bytes returns Pass");

        std::vector<minicell::u8> back;
        fsCheck(minicell::readBinaryFile(path, back) == FileResult::Pass, "read 10 bytes returns Pass");
        fsCheck(back.size() == sizeof(data), "read-back size is 10");
        fsCheck(back.size() == sizeof(data) && std::memcmp(back.data(), data, sizeof(data)) == 0,
                "read-back memcmp == 0");
    }

    {
        std::vector<minicell::u8> big(300);
        for (minicell::usize i = 0; i < big.size(); ++i) big[i] = static_cast<minicell::u8>(i);
        const std::string path = dir + "/fs_big.bin";

        minicell::writeBinaryFile(path, big.data(), big.size());
        std::vector<minicell::u8> back;
        minicell::readBinaryFile(path, back);
        fsCheck(back.size() == 300, "300-byte file reads back 300 bytes");
    }

    {
        const char hello[] = "hello minicell";
        const std::string path = dir + "/fs_text.txt";
        minicell::writeBinaryFile(path, reinterpret_cast<const minicell::u8*>(hello), sizeof(hello) - 1);

        std::string text;
        fsCheck(minicell::readTextFile(path, text) == FileResult::Pass, "text: valid file returns Pass");
        fsCheck(!text.empty(), "text: contents non-empty");
        fsCheck(text == "hello minicell", "text: contents match exactly (length 14)");
    }

    {
        const std::string path = dir + "/fs_empty.bin";
        minicell::writeBinaryFile(path, nullptr, 0);
        std::vector<minicell::u8> back = { 1, 2, 3 };
        fsCheck(minicell::readBinaryFile(path, back) == FileResult::Pass, "empty file returns Pass");
        fsCheck(back.empty(), "empty file reads back 0 bytes");
    }

    char msg[64];
    std::snprintf(msg, sizeof(msg), "filesystem tests done, failures=%d", g_fsFailures);
    minicell::logInfo(msg);
}

void writeTextureFixtures(const minicell::u8* array, minicell::usize size) {
    minicell::FileResult w1 = minicell::writeBinaryFile(MINICELL_SOURCE_DIR "/Sandbox/assets/tex.mc", array, size);
    minicell::FileResult w2 = minicell::writeBinaryFile(MINICELL_SOURCE_DIR "/Sandbox/assets/tex_truncated.mc", array, 10);

    if (w1 != minicell::FileResult::Pass) 
    {
        minicell::logError("write fail");
    } 
    else 
    {
        minicell::logInfo("w1 write success");
    }

    if (w2 != minicell::FileResult::Pass) 
    {
        minicell::logError("write fail");
    } 
    else 
    {
        minicell::logInfo("w2 write success");
    }
}

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

    // -- ScopeGuard Section --
    if constexpr (kTestScopeGuard)
    {
        minicell::ScopeGuard guard([]() {
            minicell::logInfo("leave scope"); 
        });
        minicell::logInfo("enter scope");
    }

    minicell::logInfo("MiniCell starting...");

    testParallelSum();

    // -- Handle Section --
    if constexpr (kTestHandle) {
        testHandle();

        // -- must fail to compile --
        // auto takeTexture = [](minicell::TextureHandle){};
        // takeTexture(minicell::MeshHandle{});
    }

    // -- Asset Header Section --
    if constexpr (kTestAssetHeader)
    {
        constexpr minicell::u8 kAsset[]    = { 'M','C','L','L', 1, 0, 0x10, 0, 0, 0 };
        constexpr minicell::u8 kBadMagic[] = { 'X','C','L','L', 1, 0, 0x10, 0, 0, 0 };
        testAsset(kAsset, sizeof(kAsset));
        testAsset(kBadMagic, sizeof(kBadMagic));
    }

    {
        std::unique_ptr<minicell::IPlatform> platform = minicell::createPlatform();
        const minicell::u64 freq = platform->getTicksPerSecond();
        const std::string exeDir = platform->getExeDir();
        const std::string assetDir = exeDir + "/assets/";

        // -- AssetManager Section --
        if constexpr (kTestAssetManager) {
            testAssetManager(assetDir);
        }

        // -- FileSystem Section --
        if constexpr (kTestFileSystem)
        {
            testFileSystem(exeDir);
        }

        // -- Texture File Section --
        if constexpr (kTestTextureFile)
        {        
            // version = 1; width = 256(00 00 01 00); height = 128(00 00 00 80); format = 0;
            constexpr minicell::u8 kTex[] = { 'M', 'C', 'L', 'L', 0x01, 0, 0x00, 0x01, 0, 0, 0x80, 0, 0, 0, 0x00, 0, 0, 0 };
            constexpr bool kWriteFixtures = false;
            if (kWriteFixtures) 
            {
                writeTextureFixtures(kTex, sizeof(kTex));
            }

            char msg[256];
            std::vector<minicell::u8> bytes;
            if (minicell::readBinaryFile(assetDir + "tex.mc", bytes) == minicell::FileResult::Pass) 
            {
                testAsset(bytes.data(), bytes.size());
            } 
            else 
            {
                std::snprintf(msg, sizeof(msg), "Failed reading the file: %s", (assetDir + "tex.mc").c_str());
                minicell::logError(msg);
            }

            if (minicell::readBinaryFile(assetDir + "tex_truncated.mc", bytes) == minicell::FileResult::Pass) 
            {
                testAsset(bytes.data(), bytes.size());
            } 
            else 
            {
                std::snprintf(msg, sizeof(msg), "Failed reading the file: %s", (assetDir + "tex_truncated.mc").c_str());
                minicell::logError(msg);
            }
        }

        // -- Text File Section --
        if constexpr (kTestTextFile)
        {
            char msg[256];

            std::string text;
            const std::string missing = assetDir + "missing.txt";
            if (minicell::readTextFile(missing, text) == minicell::FileResult::NotFound) 
            {
                std::snprintf(msg, sizeof(msg), "file not found: %s", missing.c_str());
                minicell::logError(msg);
            }

            if (minicell::readTextFile(assetDir + "test.txt", text) == minicell::FileResult::Pass) 
            {
                std::snprintf(msg, sizeof(msg), "file length=%zu", text.size());
                minicell::logInfo(msg);
            } 
            else 
            {
                minicell::logError("Failed reading the file");
            }
        }

        // -- Win32Platform Section --
        if constexpr (kTestWin32Platform)
        {
            char msg[256];
            std::snprintf(msg, sizeof(msg), "ticksPerSecond=%llu", static_cast<unsigned long long>(freq));
            minicell::logInfo(msg);
            std::snprintf(msg, sizeof(msg), "exeDir=%s", exeDir.c_str());
            minicell::logInfo(msg);

            minicell::LinearAllocator arena;
            minicell::u64 start = platform->getTicks();
            runArenaFrames(arena);
            minicell::u64 end = platform->getTicks();
            double res = (end - start) * 1000.0 / platform->getTicksPerSecond();
            std::snprintf(msg, sizeof(msg), "arena frames took %.3f ms", res);
            minicell::logInfo(msg);
        }
    }

    // -- RingBuffer Section --
    if constexpr (kTestRingBuffer)
    {
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
    }

    minicell::logInfo("MiniCell shutting down...");
    return 0;
}
