# MiniCell STUDY LOG

### Day 1 - 2026-09-28

- **LearnCpp:** 0 - 3 (basic, namespace (#indef, #ifndef, #endif), preprocessor (#include), header file/guards);

- **MiniCell:** Basic setup; CMake build;

### Day 2 - 2026-09-29

- **LearnCpp:** 4.1 - 4.6, 4.10 (datatype, signed/unsigned, fixed-width int, floating point num, char);
                7.1 - 7.8 (namespace, internal/external linkage, variable forward declarations);
                8.1 - 8.11 (control flow);

- **MiniCell:** Assert header file added (debug purpose); basic frame loop added (simulation);

### Day 3 - 2026-09-30

- **LearnCpp:** 12.1 - 12.13 (lvalue, rvalue, reference, pointer);
                13.6 - 13.12 (enum, struct);

- **MiniCell:** Simple simulation of ring buffer added;

### Day 4 - 2026-10-01

- **LearnCpp:** 14.1 - 14.15 (OOP, constructor/destructor);

- **MiniCell:** Logger refactor; Scope guard added;

### Day 5 - 2026-10-03

- **LearnCpp:** 15.1 - 15.7 (destructor, class&header file, this pointer);

- **MiniCell:** Binary reader added, includes little-endian concept, binary shifting, header checking;

### Day 6 - 2026-10-04

- **LearnCpp:** 19.1 - 19.2 (new/delete, dynamic array);
                20.1 - 20.7 (function pointer, stack & heap, lambdas);

- **MiniCell:** - Linear allocator added;
                - **Alignment:** an allocation's start address must be a multiple of `alignment`. (alignment=1) can start anywhere; (alignment=4) starts at offset 0, 4, 8, 12 and so on. Needed because CPU reads a type correctly/fastest only at an address that is a multiple of its size.
                - **allocate(bytes, alignment)** consist of 5 steps:
                        1. Check `alignment` is a power of two, non-zero, and <= 16 (the buffer is `alignas(16)`); otherwise log an error and return `nullptr`.
                        2. Round the offset up to the next multiple of `alignment`.
                        3. Check the space left after that position can fit `bytes`; otherwise log and return `nullptr`, leaving the offset unchanged.
                        4. Move the offset to `aligned + bytes`.
                        5. Return a pointer to `m_data[aligned]`, the start of the reserved bytes.
                - **runArenaFrames()** simulates per-frame scratch memory for 100 frames: each frame does 10 × `allocate(32)`, logs the first pointer and `used()` every 25 frames, then calls `reset()`.
                - **Arena vs new/delete:** with `new`/`delete`, 100 frames × 10 temp allocations means 1000 heap searches and 1000 `delete` calls; with the arena each allocation is just an offset bump, and one `reset()` per frame frees all at once, so nothing can leak. However every pointer from the arena dangles after the next `reset()`, and `reset()` runs no destructors, so it suits short-lived plain data only.