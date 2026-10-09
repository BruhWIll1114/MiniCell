# MiniCell STUDY LOG

### Day 1 - 2026-09-28

- **LearnCpp:** 
  - 0 - 3 (basic, namespace (#indef, #ifndef, #endif), preprocessor (#include), header file/guards);

- **MiniCell:** 
  - Basic setup; CMake build;

### Day 2 - 2026-09-29

- **LearnCpp:** 
  - 4.1 - 4.6, 4.10 (datatype, signed/unsigned, fixed-width int, floating point num, char);
  - 7.1 - 7.8 (namespace, internal/external linkage, variable forward declarations);
  - 8.1 - 8.11 (control flow);

- **MiniCell:** 
  - Assert header file added (debug purpose); basic frame loop added (simulation);

### Day 3 - 2026-09-30

- **LearnCpp:** 
  - 12.1 - 12.13 (lvalue, rvalue, reference, pointer);
  - 13.6 - 13.12 (enum, struct);

- **MiniCell:** 
  - Simple simulation of ring buffer added;

### Day 4 - 2026-10-01

- **LearnCpp:** 
  - 14.1 - 14.15 (OOP, constructor/destructor);

- **MiniCell:** 
  - Logger refactor; Scope guard added;

### Day 5 - 2026-10-03

- **LearnCpp:** 
  - 15.1 - 15.7 (destructor, class&header file, this pointer);

- **MiniCell:** 
  - Binary reader added, includes little-endian concept, binary shifting, header checking;

### Day 6 - 2026-10-04

- **LearnCpp:** 
  - 19.1 - 19.2 (new/delete, dynamic array);
  - 20.1 - 20.7 (function pointer, stack & heap, lambdas);

- **MiniCell:** 
  - Linear allocator added;
  - **Alignment:** an allocation's start address must be a multiple of `alignment`. (alignment=1) can start anywhere; (alignment=4) starts at offset 0, 4, 8, 12 and so on. Needed because CPU reads a type correctly/fastest only at an address that is a multiple of its size.
  - **allocate(bytes, alignment)** consist of 5 steps:
    - 1. Check `alignment` is a power of two, non-zero, and <= 16 (the buffer is `alignas(16)`); otherwise log an error and return `nullptr`.
    - 2. Round the offset up to the next multiple of `alignment`.
    - 3. Check the space left after that position can fit `bytes`; otherwise log and return `nullptr`, leaving the offset unchanged.
    - 4. Move the offset to `aligned + bytes`.
    - 5. Return a pointer to `m_data[aligned]`, the start of the reserved bytes.
  - **runArenaFrames()** simulates per-frame scratch memory for 100 frames: each frame does 10 × `allocate(32)`, logs the first pointer and `used()` every 25 frames, then calls `reset()`.
  - **Arena vs new/delete:** with `new`/`delete`, 100 frames × 10 temp allocations means 1000 heap searches and 1000 `delete` calls; with the arena each allocation is just an offset bump, and one `reset()` per frame frees all at once, so nothing can leak. However every pointer from the arena dangles after the next `reset()`, and `reset()` runs no destructors, so it suits short-lived plain data only.

### Day 7 - 2026-10-05

- **LearnCpp:** 
  - 22.1 - 22.5 (smart pointer, move semantic, r-value reference, std::move, unique_ptr);

- **MiniCell:** 
  - LoadedAsset module added; AssetManager added.
  - **std::unique_ptr<T>** is a type. It's a smart pointer that owns one heap object and deletes it automatically when the pointer is destroyed or reset(). It has exactly one owner, so it can't be copied, only moved.
  - **std::make_unique<T>(args...)** is a function. It creates a T on the heap, passing args to T's constructor, and returns a std::unique_ptr<T> that owns it.
  - **Shared_ptr** make sense when several systems share ownership of one asset until last of them let go. Engines usually avoid it because:
    - every copy updates an atomic reference count, which costs time;
    - it needs extra heap to store the count;
    - it isn't clear who frees the asset or when;
    - two objects point at each other never get freed;

### Day 8 - 2026-10-06

- **LearnCpp:** 
  - 25.1 - 25.4 (virtual function, override/final);

- **MiniCell:** 
  - **IPlatform.h** is the parent of all the platform created, act as a base case.
  - **Win32Platform.h** is the window class declaration inherits from IPlatform.
  - **Win32Platform.cpp** is the window implementation.

### Day 9 - 2026-10-07

- **LearnCpp:** 
  - 28.1, 28.6 - 28.7 (file I/O, istream/ofstream, seekg/tellg);

- **MiniCell:** 
  - **File System** added with file reading, accessing and writing;
  - Assets copied next to the exe by POST_BUILD; path built from `getExeDir()`;

### Day 10 - 2026-10-08

- **LearnCpp:** 
  - 11.6 - 11.7 (function templates and instantiation);
  - 26.1 - 26.2 (class templates);

- **MiniCell:** 
  - **Binary Reader** updated to add `BinaryReader::read<T>` for better management and flexibility on expending the read type of the function.
  - Strongly typed `TextureHandler` and `MeshHandler` added to have better differentiation between various asset types. Here is the workflow:
      - `readBinaryFile()` reads the asset file and load the data into memory
      - `loadFromMemory()` copies the bytes into a `LoadedAsset`
      - `createTexture()` moves the asset into storage and return its ID as a `TextureHandle`
      - `tryGet()` then vakudates the handle and return the stored texture, or `nullptr` if the handle is invalid

### Day 11 - 2026-10-09

- **MiniCell:**
  - **JobSystem:** added with fixed worker pool. 
  - **Parallel Sum Simulation:** Three version is being tested.
    - **Per-chunk partials** (`partials[chunk]`);
    - Output `partials=1273080`, `reference=1273080` with `runtime of 0.034ms`;
    - Each job writes only its own slot. Reads of data do not race. 
    - Each worker does a plain += into a local sum, then one store to partials[chunk]. Output is correct and stable;

    - **Shared atomic** (`atomicTotal.fetch_add(data[i])`)
    - Output `partials=1273080`, `reference=1273080` with `runtime of 0.156ms`;
    - Every bytes hits one cache line. More cores fight the same cache line, so parallelism does not buy throughput. Output is correct but slower;

    - **Shared racy** (no lock)
    - Output  racy trial 1: 621273 (undefined behavior); racy trial 2: 1146754 (undefined behavior);
              racy trial 3: 590701 (undefined behavior); racy trial 4: 635769 (undefined behavior);
              racy trial 5: 705116 (undefined behavior); racy trial 6: 874957 (undefined behavior);
              racy trial 7: 819573 (undefined behavior); racy trial 8: 803797 (undefined behavior);
              racy trial 9: 522034 (undefined behavior); racy trial 10: 714584 (undefined behavior);
    - Two workers can read the same old value. Data race happen. Output is incorrect.
