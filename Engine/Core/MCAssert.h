#pragma once

#include <cassert>

#define MC_ASSERT(expr) assert(expr)    // Debug (NDEBUG not defined): Failed checks stop the program.
                                        // Release (NDEBUG defined): Failed checks do not stop the program.