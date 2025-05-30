#include "cache.hpp"

#include <fstream>
#include <functional>

#include "data.hpp"
#include "directory.hpp"

namespace GCloud::Cache {
    std::filesystem::path getCacheFilePath(const std::string_view& clientId) {
        std::hash<std::string_view> hasher;
        std::string filename = std::format("{:x}.cache", hasher(clientId));
        return utils::directory::getLocalCacheDirectory() / "GDriveCpp" / "clients" / filename;
    }

    std::optional<ClientCache> getClientCache(const std::string_view& clientId, const std::string_view& clientSecret) {
        auto cacheFile = getCacheFilePath(clientId);
        if (!std::filesystem::exists(cacheFile)) {
            return std::nullopt;
        }
        std::ifstream file(cacheFile, std::ios::binary);
        if (!file.is_open()) {
            return std::nullopt;
        }
        file.seekg(0, std::ios::end);
        size_t fileSize = file.tellg();

        if (fileSize == 0) {
            return std::nullopt;
        }

        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> buffer(fileSize);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

        std::vector<uint8_t> psw{clientSecret.begin(), clientSecret.end()};
        std::vector<uint8_t> data = utils::data::decryptAES256(buffer, psw);
        std::string dataStr(data.begin(), data.end());
        return ClientCache::decode(dataStr);
    }

    void createClientCache(const std::string_view& clientId, const std::string_view& clientSecret,
                           const ClientCache& data) {
        auto cacheFile = getCacheFilePath(clientId);
        std::filesystem::create_directories(cacheFile.parent_path());
        std::ofstream file(cacheFile, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open cache file for writing: " + cacheFile.string());
        }
        std::string encodedData = data.encode();
        std::vector<uint8_t> encryptedData =
            utils::data::encryptAES256(std::vector<uint8_t>(encodedData.begin(), encodedData.end()),
                                       std::vector<uint8_t>(clientSecret.begin(), clientSecret.end()));
        file.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());
    }

    void clearClientCache(const std::string_view& clientId) {
        auto cacheFile = getCacheFilePath(clientId);
        if (std::filesystem::exists(cacheFile)) {
            std::filesystem::remove(cacheFile);
        }
    }
}  // namespace GCloud::Cache
