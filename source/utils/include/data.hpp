#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace utils::data {
    std::string getEnvironmentVariable(const std::string_view &varname);

    std::vector<uint8_t> encryptAES256(const std::vector<uint8_t> &plaintext, const std::vector<uint8_t> &password);
    std::vector<uint8_t> decryptAES256(const std::vector<uint8_t> &ciphertext, const std::vector<uint8_t> &password);
}  // namespace utils::data
