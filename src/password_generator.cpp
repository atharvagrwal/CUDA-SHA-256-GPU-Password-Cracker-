
#include "../include/password_generator.hpp"
#include <algorithm>
#include <stdexcept>
#include <random>

namespace password {

std::string index_to_password(uint64_t index, size_t length, const std::string& alphabet) {
    if (alphabet.empty()) {
        throw std::invalid_argument("Alphabet cannot be empty");
    }

    std::string password;
    password.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        password += alphabet[index % alphabet.size()];
        index /= alphabet.size();
    }
    
    return password;
}

std::vector<std::string> generate_random_passwords(size_t count, size_t min_len, size_t max_len,
                                                 const std::string& alphabet) {
    if (alphabet.empty()) {
        throw std::invalid_argument("Alphabet cannot be empty");
    }
    if (min_len > max_len) {
        throw std::invalid_argument("min_len cannot be greater than max_len");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> len_dist(min_len, max_len);
    std::uniform_int_distribution<size_t> char_dist(0, alphabet.size() - 1);
    
    std::vector<std::string> passwords;
    passwords.reserve(count);
    
    for (size_t i = 0; i < count; ++i) {
        size_t len = (min_len == max_len) ? min_len : len_dist(gen);
        std::string password;
        password.reserve(len);
        
        for (size_t j = 0; j < len; ++j) {
            password += alphabet[char_dist(gen)];
        }
        
        passwords.push_back(std::move(password));
    }
    
    return passwords;
}

} // namespace password