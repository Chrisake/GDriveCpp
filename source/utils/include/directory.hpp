#pragma once

#include <filesystem>

namespace utils {
    namespace directory {
        std::filesystem::path getLocalAppDataDirectory();
        std::filesystem::path getRoamingAppDataDirectory();
        std::filesystem::path getLocalCacheDirectory();
        std::filesystem::path getTempDirectory();
        std::filesystem::path getExecutableDirectory();
    }  // namespace directory
}  // namespace utils