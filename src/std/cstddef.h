#pragma once

// Define size_t, ptrdiff_t, and nullptr_t for freestanding environments

typedef unsigned long size_t;  // Adjust based on your architecture (e.g., use `unsigned int` for 32-bit systems)
typedef long ptrdiff_t;        // Signed type for pointer differences
#define nullptr 0              // Replace with the appropriate definition if needed
