#pragma once


#include <cuda_runtime.h>
#include "../include/sha256_gpu.cuh"

namespace sha256 {
namespace gpu {

// SHA-256 constants in constant memory for faster access
__constant__ static uint32_t d_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// Context for SHA-256 hashing
struct Context {
    uint8_t data[64];     // Current block of data being processed
    uint32_t datalen;     // Length of data in the current block
    uint64_t bitlen;      // Total length of the input in bits
    uint32_t state[8];    // Intermediate hash state
};

// Rotate right operation
__device__ uint32_t rotr32(uint32_t x, uint8_t n) {
    return (x >> n) | (x << (32 - n));
}

// SHA-256 logical functions
__device__ uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

__device__ uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

__device__ uint32_t sigma0(uint32_t x) {
    return rotr32(x, 2) ^ rotr32(x, 13) ^ rotr32(x, 22);
}

__device__ uint32_t sigma1(uint32_t x) {
    return rotr32(x, 6) ^ rotr32(x, 11) ^ rotr32(x, 25);
}

__device__ uint32_t gamma0(uint32_t x) {
    return rotr32(x, 7) ^ rotr32(x, 18) ^ (x >> 3);
}

__device__ uint32_t gamma1(uint32_t x) {
    return rotr32(x, 17) ^ rotr32(x, 19) ^ (x >> 10);
}

// Main SHA-256 transform function
__device__ void sha256_transform(Context* ctx, const uint8_t data[]) {
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];
    
    // Prepare message schedule
    for (i = 0, j = 0; i < 16; ++i, j += 4) {
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | 
               (data[j + 2] << 8) | (data[j + 3]);
    }
    
    // Extend the first 16 words into the remaining 48 words
    for (; i < 64; ++i) {
        m[i] = gamma1(m[i - 2]) + m[i - 7] + gamma0(m[i - 15]) + m[i - 16];
    }
    
    // Initialize working variables
    a = ctx->state[0]; b = ctx->state[1]; 
    c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; 
    g = ctx->state[6]; h = ctx->state[7];
    
    // Main compression function
    for (i = 0; i < 64; ++i) {
        t1 = h + sigma1(e) + ch(e, f, g) + d_K[i] + m[i];
        t2 = sigma0(a) + maj(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }
    
    // Update the hash state
    ctx->state[0] += a; ctx->state[1] += b; 
    ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; 
    ctx->state[6] += g; ctx->state[7] += h;
}

// Initialize SHA-256 context
__device__ void sha256_init(Context* ctx) {
    ctx->datalen = 0; 
    ctx->bitlen = 0;
    // Initial hash values (first 32 bits of the fractional parts of the square roots of the first 8 primes)
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

// Process input data
__device__ void sha256_update(Context* ctx, const uint8_t data[], size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        if (++(ctx->datalen) == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

// Finalize the hash
__device__ void sha256_final(Context* ctx, uint8_t hash[]) {
    uint32_t i = ctx->datalen;
    
    // Pad the data
    ctx->data[i++] = 0x80;  // Append a '1' bit
    
    // Pad with zeros until we have 8 bytes left for the length
    if (ctx->datalen < 56) {
        while (i < 56) ctx->data[i++] = 0x00;
    } else {
        while (i < 64) ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }
    
    // Append the total bit length
    ctx->bitlen += ctx->datalen * 8;
    for (i = 0; i < 8; ++i) {
        ctx->data[63 - i] = (ctx->bitlen >> (i * 8)) & 0xff;
    }
    
    // Final transform
    sha256_transform(ctx, ctx->data);
    
    // Copy the final hash to the output
    for (i = 0; i < 4; ++i) {
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
}

// Convert a number to a password using the given character set
__device__ void number_to_password(uint64_t num, int length, const char* charset, int charset_size, char* password) {
    for (int i = length - 1; i >= 0; --i) {
        password[i] = charset[num % charset_size];
        num /= charset_size;
    }
    password[length] = '\0';  // Null-terminate the string
}

// CUDA kernel that runs on the GPU
__global__ void sha256_kernel(const SearchConfig cfg, SearchResult* result) {
    uint64_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    uint64_t total_threads = blockDim.x * gridDim.x;
    uint64_t password_num = cfg.start_index + idx;
    
    // Each thread processes multiple passwords to cover the entire range
    for (uint64_t p = password_num; p < cfg.end_index; p += total_threads) {
        // Early exit if another thread found the password
        if (result->found) return;
        
        // Convert number to password
        char password[9] = {0};  // Max 8 chars + null terminator
        number_to_password(p, cfg.current_length, cfg.alphabet, cfg.alphabet_size, password);
        
        // Hash the password
        Context ctx;
        uint8_t hash[32];
        
        sha256_init(&ctx);
        sha256_update(&ctx, reinterpret_cast<const uint8_t*>(password), cfg.current_length);
        sha256_final(&ctx, hash);
        
        // Check if hash matches
        bool match = true;
        #pragma unroll
        for (int i = 0; i < 32; ++i) {
            if (hash[i] != cfg.target_hash[i]) {
                match = false;
                break;
            }
        }
        
        // If match found, store the result
        if (match) {
            // Use atomic operation to ensure only one thread updates the result
            bool expected = false;
            if (atomicCAS((unsigned long long int*)&result->found, 0, 1) == 0) {
                memcpy(result->password, password, 9);
                result->tested = p - cfg.start_index + 1;
                return;
            }
        }
    }
}

}  // namespace gpu
}  // namespace sha256