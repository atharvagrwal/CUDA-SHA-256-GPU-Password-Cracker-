
#include "../include/password_generator.hpp"
#include "../include/sha256_cpu.hpp"
#include "../include/sha256_gpu.cuh"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <optional>

struct Options {
    std::string mode = "gpu";
    std::string target_hex;
    size_t min_len = 3;
    size_t max_len = 8;
    std::string alphabet = password::DEFAULT_ALPHABET;
    size_t threads_per_block = 256;
    size_t blocks = 1024;
    bool benchmark = false;
    bool benchmark_detailed = false;  // Add this line
    bool verify = false;
};

std::string generate_random_password(size_t length, const std::string& alphabet) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, alphabet.size() - 1);
    
    std::string password;
    password.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        password += alphabet[dis(gen)];
    }
    return password;
}

std::string get_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "Options:\n"
              << "  --mode <cpu|gpu>      Select computation mode (default: gpu)\n"
              << "  --target <hash>       Target SHA-256 hash in hex (required for non-benchmark mode)\n"
              << "  --min <length>        Minimum password length (default: 3)\n"
              << "  --max <length>        Maximum password length (default: 8)\n"
              << "  --alphabet <chars>    Character set to use (default: a-zA-Z0-9 + symbols)\n"
              << "  --threads <count>     Threads per block (GPU only, default: 256)\n"
              << "  --blocks <count>      Number of blocks (GPU only, default: 1024)\n"
              << "  --benchmark           Run performance benchmark\n"
              << "  --benchmark-detailed  Run detailed benchmark with more metrics\n"
              << "  --verify              Verify hash after finding password\n"
              << "  --help                Show this help message\n";
}

Options parse_options(int argc, char** argv) {
    Options opt;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--mode" && i + 1 < argc) {
            opt.mode = argv[++i];
        } else if (arg == "--target" && i + 1 < argc) {
            opt.target_hex = argv[++i];
        } else if (arg == "--min" && i + 1 < argc) {
            opt.min_len = std::stoul(argv[++i]);
        } else if (arg == "--max" && i + 1 < argc) {
            opt.max_len = std::stoul(argv[++i]);
        } else if (arg == "--alphabet" && i + 1 < argc) {
            opt.alphabet = argv[++i];
        } else if (arg == "--threads" && i + 1 < argc) {
            opt.threads_per_block = std::stoul(argv[++i]);
        } else if (arg == "--blocks" && i + 1 < argc) {
            opt.blocks = std::stoul(argv[++i]);
        } else if (arg == "--benchmark") {
            opt.benchmark = true;

        } else if (arg == "--benchmark-detailed") {
            opt.benchmark = true;
            opt.benchmark_detailed = true;
        
        } else if (arg == "--verify") {
            opt.verify = true;
        } else if (arg == "--help") {
            print_usage(argv[0]);
            std::exit(0);
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            print_usage(argv[0]);
            std::exit(1);
        }
    }
    
    // Validate options
    if (opt.target_hex.empty() && !opt.benchmark) {
        std::cerr << "Error: --target is required when not in benchmark mode\n";
        print_usage(argv[0]);
        std::exit(1);
    }
    
    if (opt.min_len < 1 || opt.min_len > 8 || opt.max_len < 1 || opt.max_len > 8) {
        std::cerr << "Error: Password length must be between 1 and 8 characters\n";
        std::exit(1);
    }
    
    if (opt.min_len > opt.max_len) {
        std::cerr << "Error: min length cannot be greater than max length\n";
        std::exit(1);
    }
    
    if (opt.alphabet.empty()) {
        std::cerr << "Error: Alphabet cannot be empty\n";
        std::exit(1);
    }
    
    return opt;
}

void run_cpu(const Options& opt) {
    std::cout << "[" << get_timestamp() << "] Starting CPU password cracking...\n";
    std::cout << "Target hash: " << opt.target_hex << "\n";
    std::cout << "Password length: " << opt.min_len;
    if (opt.max_len > opt.min_len) {
        std::cout << " to " << opt.max_len;
    }
    std::cout << " characters\n";
    std::cout << "Alphabet size: " << opt.alphabet.size() << " characters\n";
    
    sha256::CpuSearchConfig cfg;
    cfg.target_hex = opt.target_hex;
    cfg.alphabet = opt.alphabet;
    cfg.min_length = static_cast<uint32_t>(opt.min_len);
    cfg.max_length = static_cast<uint32_t>(opt.max_len);
    cfg.batch_size = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    auto result = sha256::brute_force(cfg);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    
    std::cout << "\n--- Results ---\n";
    if (result.found) {
        std::cout << "Password found: " << result.password << "\n";
        
        if (opt.verify) {
            std::cout << "Verifying hash... ";
            auto hash = sha256::digest(result.password);
            std::string hash_hex = sha256::to_hex(hash);
            
            if (hash_hex == opt.target_hex) {
                std::cout << "VERIFIED\n";
            } else {
                std::cout << "VERIFICATION FAILED!\n";
                std::cout << "Expected: " << opt.target_hex << "\n";
                std::cout << "Got:      " << hash_hex << "\n";
            }
        }
    } else {
        std::cout << "Password not found\n";
    }
    
    std::cout << "Tested " << result.tested << " passwords in " 
              << std::fixed << std::setprecision(2) << result.seconds << " seconds ("
              << (result.hashes_per_second / 1e6) << " M hashes/s)\n";
}

void run_gpu(const Options& opt) {
    std::cout << "[" << get_timestamp() << "] Starting GPU password cracking...\n";
    std::cout << "Target hash: " << opt.target_hex << "\n";
    std::cout << "Password length: " << opt.min_len;
    if (opt.max_len > opt.min_len) {
        std::cout << " to " << opt.max_len;
    }
    std::cout << " characters\n";
    std::cout << "Alphabet size: " << opt.alphabet.size() << " characters\n";
    std::cout << "GPU configuration: " << opt.blocks << " blocks x " 
              << opt.threads_per_block << " threads/block = " 
              << (opt.blocks * opt.threads_per_block) << " total threads\n";
    
    // Get target hash as bytes
    auto target_hash_opt = sha256::from_hex(opt.target_hex);
    if (!target_hash_opt) {
        std::cerr << "Error: Invalid target hash format\n";
        return;
    }
    const auto& target_hash = *target_hash_opt;
    
    // Prepare GPU configuration
    sha256::gpu::SearchConfig cfg;
    std::memcpy(cfg.target_hash, target_hash.data_ptr(), 32);
    cfg.alphabet = const_cast<char*>(opt.alphabet.c_str());
    cfg.alphabet_size = static_cast<uint32_t>(opt.alphabet.size());
    cfg.min_length = static_cast<uint32_t>(opt.min_len);
    cfg.max_length = static_cast<uint32_t>(opt.max_len);
    cfg.threads_per_block = static_cast<uint32_t>(opt.threads_per_block);
    cfg.blocks = static_cast<uint32_t>(opt.blocks);
    
    // Process each password length
    for (size_t len = opt.min_len; len <= opt.max_len; ++len) {
        std::cout << "\nTrying length " << len << "...\n";
        
        // Calculate total passwords for this length
        uint64_t total = 1;
        for (size_t i = 0; i < len; ++i) {
            total *= opt.alphabet.size();
        }
        
        std::cout << "Total passwords to test: " << total << "\n";
        
        // Split work into batches to avoid integer overflow
        const uint64_t max_batch_size = 1ULL << 30;  // 1 billion passwords per batch
        uint64_t batch_size = std::min(total, max_batch_size);
        uint64_t num_batches = (total + batch_size - 1) / batch_size;
        
        std::cout << "Processing in " << num_batches << " batches of " << batch_size << " passwords\n";
        
        bool found = false;
        uint64_t total_tested = 0;
        double total_time = 0.0;
        
        for (uint64_t batch = 0; batch < num_batches && !found; ++batch) {
            uint64_t start = batch * batch_size;
            uint64_t end = std::min(start + batch_size, total);
            uint64_t count = end - start;
            
            std::cout << "Batch " << (batch + 1) << "/" << num_batches 
                      << " (" << start << " - " << (end - 1) << ")... " << std::flush;
            
            // Update config for this batch
            cfg.start_index = start;
            cfg.end_index = end;
            cfg.current_length = static_cast<uint32_t>(len);
            
            // Run GPU kernel
            auto start_time = std::chrono::high_resolution_clock::now();
            auto result = sha256::gpu::brute_force(cfg);
            auto end_time = std::chrono::high_resolution_clock::now();
            
            // Update statistics
            double batch_time = std::chrono::duration<double>(end_time - start_time).count();
            total_time += batch_time;
            total_tested += count;
            
            std::cout << std::fixed << std::setprecision(2) 
                      << (count / batch_time / 1e6) << " M hashes/s";
            
            if (result.found) {
                std::cout << "\n\n--- PASSWORD FOUND ---\n";
                std::cout << "Password: " << result.password << "\n";
                
                if (opt.verify) {
                    std::cout << "Verifying hash... ";
                    auto hash = sha256::digest(result.password);
                    std::string hash_hex = sha256::to_hex(hash);
                    
                    if (hash_hex == opt.target_hex) {
                        std::cout << "VERIFIED\n";
                    } else {
                        std::cout << "VERIFICATION FAILED!\n";
                        std::cout << "Expected: " << opt.target_hex << "\n";
                        std::cout << "Got:      " << hash_hex << "\n";
                    }
                }
                
                found = true;
                break;
            }
            
            std::cout << "\n";
        }
        
        std::cout << "Tested " << total_tested << " passwords in " 
                  << std::fixed << std::setprecision(2) << total_time << " seconds ("
                  << (total_tested / total_time / 1e6) << " M hashes/s)\n";
        
        if (found) {
            return;
        }
    }
    
    std::cout << "\nPassword not found in the specified range\n";
}

void run_benchmark(const Options& opt) {
    std::cout << "[" << get_timestamp() << "] Starting benchmark...\n";
    
    // Generate a random password
    std::string password;
    if (opt.min_len == opt.max_len) {
        password = password::generate_random_passwords(1, opt.min_len, opt.min_len, opt.alphabet)[0];
    } else {
        password = password::generate_random_passwords(1, opt.min_len, opt.max_len, opt.alphabet)[0];
    }
    
    // Get its hash
    auto hash = sha256::digest(password);
    std::string hash_hex = sha256::to_hex(hash);
    
    // Calculate total combinations
    uint64_t total_combinations = 1;
    for (size_t i = 0; i < password.length(); ++i) {
        total_combinations *= opt.alphabet.size();
    }
    
    // Detailed output header
    if (opt.benchmark_detailed) {
        std::cout << "\n=== Detailed Benchmark Configuration ===\n";
        std::cout << "Generated password: " << password << "\n";
        std::cout << "Target hash: " << hash_hex << "\n";
        std::cout << "Password length: " << password.length() << " characters\n";
        std::cout << "Alphabet size: " << opt.alphabet.size() << " characters\n";
        std::cout << "Total combinations: " << total_combinations << "\n";
        std::cout << std::string(50, '=') << "\n";
    } else {
        std::cout << "Running benchmark with password length: " << password.length() << "\n";
    }

    // Run CPU benchmark
    if (opt.mode == "cpu" || opt.mode == "both") {
        if (opt.benchmark_detailed) {
            std::cout << "\n--- CPU Benchmark ---\n";
        }
        
        sha256::CpuSearchConfig cfg;
        cfg.target_hex = hash_hex;
        cfg.alphabet = opt.alphabet;
        cfg.min_length = static_cast<uint32_t>(password.length());
        cfg.max_length = static_cast<uint32_t>(password.length());
        cfg.batch_size = 1000000;
        
        auto start = std::chrono::high_resolution_clock::now();
        auto result = sha256::brute_force(cfg);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
        if (opt.benchmark_detailed) {
            std::cout << "Password found: " << (result.found ? "YES" : "NO") << "\n";
            if (result.found) {
                std::cout << "Password: " << result.password << "\n";
            }
            std::cout << "Tested " << result.tested << " passwords in " 
                     << std::fixed << std::setprecision(6) << result.seconds << " seconds\n";
            std::cout << "Hash rate: " << (result.hashes_per_second / 1e6) << " M hashes/s\n";
            std::cout << "Estimated time to test all combinations: " 
                     << (total_combinations / result.hashes_per_second) << " seconds\n";
            std::cout << std::string(50, '-') << "\n";
        } else {
            std::cout << "CPU: " << (result.hashes_per_second / 1e6) << " M hashes/s\n";
        }
    }
    
// Run GPU benchmark
    if (opt.mode == "gpu" || opt.mode == "both") {
        if (opt.benchmark_detailed) {
            std::cout << "\n--- GPU Benchmark ---\n";
        }
        
        // Get target hash as bytes
        auto target_hash_opt = sha256::from_hex(hash_hex);
        if (!target_hash_opt) {
            std::cerr << "Error: Failed to parse hash\n";
            return;
        }
        const auto& target_hash = *target_hash_opt;
        
        // Prepare GPU configuration
        sha256::gpu::SearchConfig cfg;
        std::memcpy(cfg.target_hash, target_hash.data_ptr(), 32);
        cfg.alphabet = const_cast<char*>(opt.alphabet.c_str());
        cfg.alphabet_size = static_cast<uint32_t>(opt.alphabet.size());
        cfg.min_length = static_cast<uint32_t>(password.length());
        cfg.max_length = static_cast<uint32_t>(password.length());
        cfg.threads_per_block = static_cast<uint32_t>(opt.threads_per_block);
        cfg.blocks = static_cast<uint32_t>(opt.blocks);
        cfg.current_length = static_cast<uint32_t>(password.length());
        
        // Calculate total passwords
        uint64_t total = 1;
        for (size_t i = 0; i < password.length(); ++i) {
            total *= opt.alphabet.size();
        }
        
        // Split into batches
        const uint64_t max_batch_size = 1ULL << 30;  // 1 billion passwords per batch
        uint64_t batch_size = std::min(total, max_batch_size);
        uint64_t num_batches = (total + batch_size - 1) / batch_size;
        
        std::cout << "Total passwords: " << total << "\n";
        std::cout << "Batch size: " << batch_size << " passwords\n";
        std::cout << "Number of batches: " << num_batches << "\n";
        std::cout << "GPU configuration: " << opt.blocks << " blocks x " 
                  << opt.threads_per_block << " threads/block = " 
                  << (opt.blocks * opt.threads_per_block) << " total threads\n";
        
        // Warm-up run
        std::cout << "Warming up... " << std::flush;
        cfg.start_index = 0;
        cfg.end_index = std::min(batch_size, total);
        (void)sha256::gpu::brute_force(cfg);  // Ignore warmup result
        std::cout << "done\n";

         //  Benchmark output:
        if (opt.benchmark_detailed) {
            std::cout << "Total passwords: " << total << "\n";
            std::cout << "Batch size: " << batch_size << " passwords\n";
            std::cout << "Number of batches: " << num_batches << "\n";
            std::cout << "GPU configuration: " << opt.blocks << " blocks x " 
                     << opt.threads_per_block << " threads/block = " 
                     << (opt.blocks * opt.threads_per_block) << " total threads\n";
            std::cout << "Warming up... " << std::flush;
        }
        
        // Benchmark run
        std::cout << "Running benchmark... " << std::flush;
        auto start = std::chrono::high_resolution_clock::now();
        auto result = sha256::gpu::brute_force(cfg);
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        
 double hashes_per_second = batch_size / elapsed.count();
        
        if (opt.benchmark_detailed) {
            std::cout << "\n=== GPU Benchmark Results ===\n";
            std::cout << "Password found: " << (result.found ? "YES" : "NO") << "\n";
            if (result.found) {
                std::cout << "Password: " << result.password << "\n";
            }
            std::cout << "Tested " << batch_size << " passwords in " 
                     << std::fixed << std::setprecision(6) << elapsed.count() << " seconds\n";
            std::cout << "Hash rate: " << (hashes_per_second / 1e6) << " M hashes/s (";
            
            // Human-readable format
            const char* units[] = {"H/s", "kH/s", "MH/s", "GH/s", "TH/s"};
            int unit = 0;
            double hr = hashes_per_second;
            while (hr > 1000 && unit < 4) {
                hr /= 1000;
                unit++;
            }
            std::cout << std::fixed << std::setprecision(2) << hr << " " << units[unit] << ")\n";
            
            std::cout << "Estimated time to test all combinations: " 
                     << std::fixed << std::setprecision(2) 
                     << (total / hashes_per_second) << " seconds\n";
            std::cout << std::string(50, '=') << "\n";
        } else {
            std::cout << "GPU: " << (hashes_per_second / 1e6) << " M hashes/s\n";
        }
    }
}

int main(int argc, char** argv) {
    try {
        // Parse command line options
        Options opt = parse_options(argc, argv);
        
        // Run in the selected mode
        if (opt.benchmark) {
            run_benchmark(opt);
        } else if (opt.mode == "cpu") {
            run_cpu(opt);
        } else if (opt.mode == "gpu") {
            run_gpu(opt);
        } else {
            std::cerr << "Error: Invalid mode '" << opt.mode << "'. Use 'cpu' or 'gpu'.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}