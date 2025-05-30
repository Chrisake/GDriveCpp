#pragma once

#include <chrono>
#include <filesystem>
#include <future>
#include <memory>
#include <string>

#include "GDriveCpp/dllExport.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace GCloud::Authentication {
    class GDRIVE_API OAuthAgent {
      private:
        std::string _clientId;
        std::string _clientSecret;
        std::string _code;
        std::string _accessToken;
        std::string _refreshToken;
        std::chrono::system_clock::time_point _expirationTime;
        void authenticate();
        bool checkToken();
        void refreshAccessToken();
      public:
        OAuthAgent(std::string& clientId, std::string& clientSecret);
        ~OAuthAgent();
        std::string getAccessToken();
    };
}  // namespace GCloud::Authentication

#ifdef _MSC_VER
#pragma warning(pop)
#endif
