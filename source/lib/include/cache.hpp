#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>

namespace GCloud::Cache {
    struct ClientCache {
        std::string accessToken;
        std::chrono::system_clock::time_point accessTokenExpiration;
        std::string refreshToken;

        std::string encode() const {
            nlohmann::json jsonData;
            jsonData["refresh_token"] = refreshToken;
            if (!accessToken.empty()) {
                jsonData["access_token"] = accessToken;
                jsonData["expires_at"] =
                    std::chrono::system_clock::to_time_t(accessTokenExpiration);
            }
            return jsonData.dump();  // Simplified for demonstration; implement proper encoding as needed
        }

        static ClientCache decode(const std::string_view &data) {
            nlohmann::json jsonData;
            try {
                jsonData = nlohmann::json::parse(data);
            } catch (const nlohmann::json::parse_error &e) {
                throw std::runtime_error("Failed to decode ClientCache: " + std::string(e.what()));
            }
            ClientCache cache;
            if (jsonData.contains("refresh_token")) {
                cache.refreshToken = jsonData["refresh_token"].get<std::string>();
            } else {
                throw std::runtime_error("Invalid ClientCache data: Missing refresh token");
            }

            if (jsonData.contains("access_token")) {
                if (jsonData.contains("expires_in")) {
                    cache.accessTokenExpiration =
                        std::chrono::system_clock::now() +
                        std::chrono::seconds(std::stoi(jsonData["expires_in"].get<std::string>()));
                } else if (jsonData.contains("expires_at")) {
                    cache.accessTokenExpiration =
                        std::chrono::system_clock::from_time_t(jsonData["expires_at"].get<int64_t>());
                } else {
                    throw std::runtime_error("Invalid ClientCache data: Missing access token expiration");
                }
                cache.accessToken = jsonData["access_token"].get<std::string>();
            }
            return cache;
        }
    };

    std::filesystem::path getCacheFilePath(const std::string_view &clientId);
    std::optional<ClientCache> getClientCache(const std::string_view &clientId, const std::string_view &clientSecret);
    void createClientCache(const std::string_view &clientId, const std::string_view &clientSecret,
                           const ClientCache &data);
    void clearClientCache(const std::string_view &clientId);


}  // namespace GCloud::Cache