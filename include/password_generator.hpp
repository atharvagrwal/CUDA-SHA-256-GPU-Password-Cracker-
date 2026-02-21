#pragma once

#include <string>
#include <vector>
#include <random>

namespace password {

const std::string DEFAULT_ALPHABET = 
    "abcdefghijklmnopqrstuvwxyz"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "0123456789"
    "!@#$%^&*()_+-=[]{}|;:,.<>?";

// Generate a single password from an index
std::string index_to_password(uint64_t index, size_t length, const std::string& alphabet = DEFAULT_ALPHABET);

// Generate random passwords for benchmarking
std::vector<std::string> generate_random_passwords(size_t count, size_t min_len, size_t max_len, 
                                                  const std::string& alphabet = DEFAULT_ALPHABET);

} // namespace password