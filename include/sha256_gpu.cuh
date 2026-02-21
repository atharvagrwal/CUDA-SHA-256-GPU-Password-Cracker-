
#pragma once

#include <cstdint>
#include <string>

namespace sha256 {
namespace gpu {

// Forward declarations of structures that will be used in both CPU and GPU code
struct SearchConfig {
    uint8_t target_hash[32];    // The target hash to find (32 bytes for SHA-256)
    const char* alphabet;       // Character set to use for password generation
    uint32_t alphabet_size;     // Size of the alphabet
    uint32_t min_length;        // Minimum password length to try
    uint32_t max_length;        // Maximum password length to try
    uint32_t threads_per_block; // CUDA threads per block
    uint32_t blocks;            // Number of CUDA blocks
    uint64_t start_index;       // Starting index in password space
    uint64_t end_index;         // Ending index in password space
    uint32_t current_length;    // Current password length being processed
};

struct SearchResult {
    bool found;                 // Whether the password was found
    char password[9];           // Buffer for the found password (max 8 chars + null)
    uint64_t tested;            // Number of passwords tested
    float milliseconds;         // Time taken for the search
};

// Function declaration - implementation will be in the .cu file
SearchResult brute_force(const SearchConfig& cfg);

}  // namespace gpu
}  // namespace sha256