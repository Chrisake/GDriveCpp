#include "data.hpp"

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <format>
#include <stdexcept>

namespace utils::data {
    std::string getEnvironmentVariable(const std::string_view& varname) {
        char* buf = nullptr;
        size_t sz = 0;
        if (_dupenv_s(&buf, &sz, varname.data()) != 0 || buf == nullptr) {
            // throw std::runtime_error(std::format("Failed to get environment variable: {}", varname));
            return std::string();
        }
        std::string result(buf, sz);
        free(buf);
        return result;
    }

    static std::vector<uint8_t> generateSalt(int saltSize) {
        std::vector<unsigned char> salt(saltSize);
        if (RAND_bytes(salt.data(), saltSize) != 1) {
            throw std::runtime_error("Error generating salt");
        }
        return salt;
    }

    static void generateKeyAndIV(const std::vector<uint8_t>& password, const std::vector<uint8_t>& salt,
                                 std::vector<uint8_t>& key, std::vector<uint8_t>& iv) {
        key.resize(32);
        iv.resize(16);
        int ret = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha256(), salt.data(), password.data(),
                                 static_cast<int32_t>(password.size()), 1, key.data(), iv.data());
        if (ret != 32) {
            throw std::runtime_error("Error generating key and IV");
        }
    }

    std::vector<uint8_t> encryptAES256(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& password) {
        std::vector<uint8_t> salt = generateSalt(16);
        std::vector<uint8_t> key;
        std::vector<uint8_t> iv;
        generateKeyAndIV(password, salt, key, iv);

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Error creating cipher context");

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error initializing encryption");
        }

        std::vector<uint8_t> ciphertext(salt.size() + plaintext.size() + EVP_MAX_BLOCK_LENGTH);
        int32_t ciphertext_len;
        // Copy the salt to the beginning of the ciphertext
        std::copy(salt.begin(), salt.end(), ciphertext.begin());
        if (EVP_EncryptUpdate(ctx, ciphertext.data() + salt.size(), &ciphertext_len, plaintext.data(),
                              static_cast<int32_t>(plaintext.size())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error during encryption update");
        }
        int32_t final_len;
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + salt.size() + ciphertext_len, &final_len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error during final encryption");
        }

        ciphertext.resize(salt.size() + ciphertext_len + final_len);
        EVP_CIPHER_CTX_free(ctx);
        return ciphertext;
    }

    std::vector<uint8_t> decryptAES256(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& password) {
        std::vector<uint8_t> salt = std::vector<uint8_t>(16);
        std::vector<uint8_t> key;
        std::vector<uint8_t> iv;
        std::copy_n(ciphertext.begin(), salt.size(), salt.begin());
        generateKeyAndIV(password, salt, key, iv);

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("Error creating cipher context");

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error initializing decryption");
        }

        std::vector<unsigned char> plaintext(ciphertext.size() - salt.size());
        int32_t plaintext_len;
        if (EVP_DecryptUpdate(ctx, plaintext.data(), &plaintext_len, ciphertext.data() + salt.size(),
                              static_cast<int32_t>(ciphertext.size() - salt.size())) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error during decryption update");
        }
        int32_t final_len;
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintext_len, &final_len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Error during final decryption");
        }

        plaintext.resize(plaintext_len + final_len);
        EVP_CIPHER_CTX_free(ctx);
        return plaintext;
    }
}  // namespace utils::data