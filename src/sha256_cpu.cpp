#include "../include/sha256_cpu.hpp"
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <thread>
#include <atomic>
#include <mutex>

namespace sha256 {

// SHA-256 helper functions
#define ROTRIGHT(a, b) (((a) >> (b)) | ((a) << (32 - (b))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))

void sha256_init(Context* ctx) {
    ctx->datalen = 0;
    ctx->bitlen = 0;
    ctx->state[0] = 0x6a09e667;
    ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372;
    ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f;
    ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab;
    ctx->state[7] = 0x5be0cd19;
}

void sha256_transform(Context* ctx, const uint8_t* data) {
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    for (i = 0, j = 0; i < 16; ++i, j += 4) {
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    }

    for (; i < 64; ++i) {
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];
    }

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i) {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + m[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

void sha256_update(Context* ctx, const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        ctx->data[ctx->datalen] = data[i];
        ctx->datalen++;
        if (ctx->datalen == 64) {
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

void sha256_final(Context* ctx, uint8_t* hash) {
    uint32_t i = ctx->datalen;

    // Pad whatever data is left in the buffer
    if (ctx->datalen < 56) {
        ctx->data[i++] = 0x80;
        while (i < 56) {
            ctx->data[i++] = 0x00;
        }
    } else {
        ctx->data[i++] = 0x80;
        while (i < 64) {
            ctx->data[i++] = 0x00;
        }
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append the total bit length
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data);

    // Copy the final state to the hash
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

Hash32 digest(const uint8_t* data, size_t len) {
    Context ctx;
    Hash32 hash;
    
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, hash.data_ptr());
    
    return hash;
}

Hash32 digest(const std::string& str) {
    return digest(reinterpret_cast<const uint8_t*>(str.data()), str.length());
}

std::string to_hex(const Hash32& hash) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (const auto& byte : hash) {
        ss << std::setw(2) << static_cast<unsigned>(byte);
    }
    
    return ss.str();
}

std::optional<Hash32> from_hex(const std::string& hex) {
    if (hex.length() != 64) {
        return std::nullopt;
    }
    
    Hash32 hash;
    for (size_t i = 0; i < 32; ++i) {
        try {
            hash[i] = static_cast<uint8_t>(std::stoi(hex.substr(i * 2, 2), nullptr, 16));
        } catch (...) {
            return std::nullopt;
        }
    }
    
    return hash;
}

bool verify_password(const std::string& password, const Hash32& target_hash) {
    Hash32 hash = digest(password);
    return hash == target_hash;
}

// Worker function for parallel password checking
void worker(
    const std::vector<std::string>& passwords,
    const Hash32& target_hash,
    size_t start, size_t end,
    std::atomic<bool>& found,
    std::string& result,
    std::mutex& result_mutex,
    std::atomic<uint64_t>& tested,
    std::atomic<bool>& stop
) {
    Hash32 hash;
    Context ctx;
    
    for (size_t i = start; i < end && !stop.load(); ++i) {
        // Hash the password
        const std::string& pwd = passwords[i];
        
        sha256_init(&ctx);
        sha256_update(&ctx, reinterpret_cast<const uint8_t*>(pwd.data()), pwd.length());
        sha256_final(&ctx, hash.data_ptr());
        
        tested.fetch_add(1, std::memory_order_relaxed);
        
        // Check if it matches
        if (hash == target_hash) {
            std::lock_guard<std::mutex> lock(result_mutex);
            if (!found.load()) {
                found.store(true);
                result = pwd;
                stop.store(true);
            }
            return;
        }
    }
}

CpuSearchResult brute_force(const CpuSearchConfig& cfg) {
    auto target_hash_opt = from_hex(cfg.target_hex);
    if (!target_hash_opt) {
        throw std::invalid_argument("Invalid target hash format");
    }
    const Hash32& target_hash = *target_hash_opt;
    
    CpuSearchResult result;
    result.found = false;
    result.tested = 0;
    
    const size_t num_threads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::mutex result_mutex;
    std::atomic<bool> found(false);
    std::atomic<bool> stop(false);
    std::atomic<uint64_t> tested(0);
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Process each password length
    for (size_t len = cfg.min_length; len <= cfg.max_length && !stop.load(); ++len) {
        uint64_t total = 1;
        for (size_t i = 0; i < len; ++i) {
            total *= cfg.alphabet.size();
        }
        
        const uint64_t batch_size = std::min<uint64_t>(cfg.batch_size, total);
        const size_t batch_count = (total + batch_size - 1) / batch_size;
        
        for (uint64_t batch = 0; batch < batch_count && !stop.load(); ++batch) {
            const uint64_t start = batch * batch_size;
            const uint64_t end = std::min(start + batch_size, total);
            const size_t count = end - start;
            
            // Generate passwords for this batch
            std::vector<std::string> passwords;
            passwords.reserve(count);
            for (uint64_t i = 0; i < count; ++i) {
                uint64_t idx = start + i;
                std::string pwd;
                pwd.reserve(len);
                
                for (size_t j = 0; j < len; ++j) {
                    pwd += cfg.alphabet[idx % cfg.alphabet.size()];
                    idx /= cfg.alphabet.size();
                }
                
                passwords.push_back(std::move(pwd));
            }
            
            // Split work among threads
            const size_t chunk_size = (count + num_threads - 1) / num_threads;
            std::string found_password;
            
            // Launch worker threads
            for (size_t t = 0; t < num_threads; ++t) {
                const size_t t_start = t * chunk_size;
                const size_t t_end = std::min(t_start + chunk_size, count);
                
                if (t_start < t_end) {
                    threads.emplace_back(
                        worker,
                        std::cref(passwords),
                        std::cref(target_hash),
                        t_start, t_end,
                        std::ref(found),
                        std::ref(found_password),
                        std::ref(result_mutex),
                        std::ref(tested),
                        std::ref(stop)
                    );
                }
            }
            
            // Wait for all threads to finish
            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
            threads.clear();
            
            // Check if we found a match
            if (found.load()) {
                result.found = true;
                result.password = found_password;
                break;
            }
        }
        
        if (found.load()) {
            break;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    
    result.tested = tested.load();
    result.seconds = elapsed.count();
    result.hashes_per_second = (result.seconds > 0) ? (result.tested / result.seconds) : 0;
    
    return result;
}

}  // namespace sha256