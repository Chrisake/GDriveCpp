#include "logging.hpp"

#include <filesystem>
#include <format>
#include <random>
#include <string>

#include "data.hpp"
#include "directory.hpp"

namespace utils::logging {
    LogConsole::LogConsole(const std::string_view& name, uint16_t port, MessageReceivedCallback msgCallback)
        : _name(std::string(name)), _port(port), _msgReceivedCallback(msgCallback) {
        ZeroMemory(&_si, sizeof(_si));
        _si.cb = sizeof(_si);
        ZeroMemory(&_pi, sizeof(_pi));
        if (_port == 0) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(49152, 65535);
            _port = static_cast<uint16_t>(distrib(gen));
        }
    }

    LogConsole::~LogConsole() { destroy(); }

    bool LogConsole::isRunning() const { return _isRunning; }

#ifdef _WIN32
    void LogConsole::create() {
        if (_isRunning) return;

        if (_port == 0) {
            printf("Port is not set. Cannot create log console.\n");
            return;
        }

        std::filesystem::path logConsolePath;
        std::filesystem::path directory = utils::data::getEnvironmentVariable("LOG_CONSOLE_DIR");
        if (directory.empty()) {
            directory = utils::directory::getExecutableDirectory();
        }
        logConsolePath = directory / "LogConsole.exe";
        if (!std::filesystem::exists(logConsolePath)) {
            printf("LogConsole.exe not found in %s\n", logConsolePath.string().c_str());
            return;
        }

        auto res = CreateProcess(
            NULL,  // No module name (use command line)
            const_cast<char*>(std::format("{} {}", logConsolePath.generic_string(), _port).c_str()),  // Command line
            NULL,   // Process handle not inheritable
            NULL,   // Thread handle not inheritable
            FALSE,  // Set handle inheritance to FALSE
            CREATE_NEW_CONSOLE,  // No creation flags
            NULL,   // Use parent's environment block
            NULL,   // Use parent's starting directory
            &_si,   // Pointer to STARTUPINFO structure
            &_pi);  // Pointer to PROCESS_INFORMATION structure
        if (!res) {
            printf("CreateProcess failed (%d).\n", GetLastError());
            return;
        }

        _isRunning = true;
    }

    void LogConsole::connect() {
    
    }

    void LogConsole::destroy() {
        if (!_isRunning) return;
        TerminateProcess(_pi.hProcess, 0);
        CloseHandle(_pi.hProcess);
        CloseHandle(_pi.hThread);
        _isRunning = false;
    }
#endif


}  // namespace utils::logging
