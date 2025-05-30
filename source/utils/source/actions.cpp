#include "actions.hpp"

#include <iostream>
#include <regex>
#include <format>

namespace utils::actions {
    void openBrowser(const std::string_view& url) {
#ifdef _WIN32
        // Windows
        std::string command = "start " + std::regex_replace(std::string(url), std::regex("&"), "^&");
#elif __APPLE__
        // macOS
        std::string command = std::format("open {}", url);
#elif __linux__
        // Linux
        std::string command = std::format("xdg-open {}", url);
#else
        // Unsupported platform
#error "Unsupported platform."
#endif

        // Execute the command
        int result = std::system(command.c_str());
        if (result != 0) {
            std::cerr << "Error: Failed to open browser. system() returned " << result << std::endl;
        }
    }
}  // namespace utils::actions