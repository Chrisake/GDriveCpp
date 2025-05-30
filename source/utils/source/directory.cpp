#include "directory.hpp"
#include "data.hpp"

#include <filesystem>
#include <string>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else

#endif
namespace utils {
    namespace directory {
        std::filesystem::path getLocalAppDataDirectory() {
#ifdef _WIN32
            return std::filesystem::path(utils::data::getEnvironmentVariable("LOCALAPPDATA"));
#elif __linux__ || __APPLE__
            return std::filesystem::path(utils::data::getEnvironmentVariable("HOME")) / ".local" / "share";
#else
#error "Unsupported platform"
#endif
        }

        std::filesystem::path getRoamingAppDataDirectory() {
#ifdef _WIN32
            return std::filesystem::path(utils::data::getEnvironmentVariable("APPDATA"));
#elif __linux__ || __APPLE__
            return std::filesystem::path(utils::data::getEnvironmentVariable("HOME")) / ".config";
#else
#error "Unsupported platform"
#endif
        }

        std::filesystem::path getLocalCacheDirectory() {
#ifdef _WIN32
            return (std::filesystem::path(utils::data::getEnvironmentVariable("LOCALAPPDATA")) / "Cache");
#elif __linux__ || __APPLE__
            return std::filesystem::path(utils::data::getEnvironmentVariable("HOME")) / ".cache";
#else
#error "Unsupported platform"
#endif
        }

        std::filesystem::path getTempDirectory() {
#ifdef _WIN32
            return std::filesystem::path(utils::data::getEnvironmentVariable("TEMP"));
#elif __linux__ || __APPLE__
            return std::filesystem::path(utils::data::getEnvironmentVariable("TMPDIR"));
#else
#error "Unsupported platform"
#endif
        }

        std::filesystem::path getExecutableDirectory() {
#ifdef _WIN32
            char buffer[MAX_PATH];
            GetModuleFileNameA(nullptr, buffer, MAX_PATH);
            std::filesystem::path exePath(buffer);
            return exePath.parent_path();
#elif __linux__ || __APPLE__
            char buffer[PATH_MAX];
            ssize_t count = readlink("/proc/self/exe", buffer, PATH_MAX);
            if (count != -1) {
                std::filesystem::path exePath(buffer, count);
                return exePath.parent_path();
            } else {
                return std::filesystem::current_path(); // Fallback to current path
            }
#else
#error "Unsupported platform"
#endif
        }
    }  // namespace directory
}  // namespace utils