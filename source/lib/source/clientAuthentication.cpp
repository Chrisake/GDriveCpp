#include <cpr/cpr.h>

#include <cstdlib>
#include <format>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>

#include "GDriveCpp/gDrive.h"
#include "actions.hpp"
#include "cache.hpp"
#include "callbackListener.hpp"

namespace GCloud::Authentication {
    OAuthAgent::OAuthAgent(std::string& clientId, std::string& clientSecret)
        : _clientId(clientId), _clientSecret(clientSecret) {}

    OAuthAgent::~OAuthAgent() {}

    void OAuthAgent::authenticate() {
        // Check if we have a cached auth code
        std::optional<GCloud::Cache::ClientCache> clientCache;
        try {
            clientCache = GCloud::Cache::getClientCache(_clientId, _clientSecret);
            if (clientCache.has_value()) {
                _refreshToken = clientCache->refreshToken;
                _accessToken = clientCache->accessToken;
                _expirationTime = clientCache->accessTokenExpiration;
                if (!checkToken()) refreshAccessToken();
                return;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error retrieving client cache: " << e.what() << std::endl;
        }


        std::string authURI = std::format(
            "https://accounts.google.com/o/oauth2/auth?"
            "client_id={}&redirect_uri={}&response_type=code&scope={}",
            _clientId, "http://localhost:8080", "https://www.googleapis.com/auth/drive");
        utils::actions::openBrowser(authURI);
        _code = GCloud::Authentication::listenForCode(8080);
        auto response = cpr::Post(cpr::Url{"https://oauth2.googleapis.com/token"},
                                  cpr::Parameters{{"code", _code},
                                                  {"client_id", _clientId},
                                                  {"client_secret", _clientSecret},
                                                  {"redirect_uri", "http://localhost:8080"},
                                                  {"grant_type", "authorization_code"}},
                                  cpr::VerifySsl(0));
        if (response.status_code != 200) {
            throw std::runtime_error(std::format("Failed to fetch access token: {} - {}\n{}", response.status_code,
                                                 response.reason, response.text));
            return;
        }
        auto jsonResponse = nlohmann::json::parse(response.text);
        if (!jsonResponse.contains("access_token") || !jsonResponse.contains("refresh_token")) {
            throw std::runtime_error("Failed to fetch access token: Invalid JSON Response");
            return;
        }
        _accessToken = jsonResponse["access_token"];
        int expiresIn = jsonResponse.value("expires_in", 3600);  // Default to 1 hour if not provided
        _expirationTime = std::chrono::system_clock::now() + std::chrono::seconds(expiresIn);
        _refreshToken = jsonResponse["refresh_token"];

        GCloud::Cache::createClientCache(
            _clientId, _clientSecret,
            GCloud::Cache::ClientCache{
                .accessToken = _accessToken, .accessTokenExpiration = _expirationTime, .refreshToken = _refreshToken});
    }

    void OAuthAgent::refreshAccessToken() {
        if (_refreshToken.empty()) {
            throw std::runtime_error("No refresh token available. Please authenticate first.");
        }
        auto response = cpr::Post(cpr::Url{"https://oauth2.googleapis.com/token"},
                                  cpr::Parameters{{"client_id", _clientId},
                                                  {"client_secret", _clientSecret},
                                                  {"refresh_token", _refreshToken},
                                                  {"grant_type", "refresh_token"}},
                                  cpr::VerifySsl(0));
        if (response.status_code != 200) {
            throw std::runtime_error(std::format("Failed to refresh access token: {} - {}\n{}", response.status_code,
                                                 response.reason, response.text));
        }
        auto jsonResponse = nlohmann::json::parse(response.text);
        if (!jsonResponse.contains("access_token")) {
            throw std::runtime_error("Failed to refresh access token: Invalid JSON Response");
        }
        _accessToken = jsonResponse["access_token"];
        int expiresIn = jsonResponse.value("expires_in", 3600);  // Default to 1 hour if not provided
        _expirationTime = std::chrono::system_clock::now() + std::chrono::seconds(expiresIn);

        GCloud::Cache::createClientCache(
            _clientId, _clientSecret,
            GCloud::Cache::ClientCache{
                .accessToken = _accessToken, .accessTokenExpiration = _expirationTime, .refreshToken = _refreshToken});
    }

    bool OAuthAgent::checkToken() {
        if (_refreshToken.empty() || _accessToken.empty() || std::chrono::system_clock::now() >= _expirationTime) {
            return false;
        }
        return true;
    }

    std::string OAuthAgent::getAccessToken() {
        if (_refreshToken.empty()) {
            authenticate();
        }
        if (!checkToken()) {
            refreshAccessToken();
        }
        return _accessToken;
    }
}  // namespace GCloud::Authentication
