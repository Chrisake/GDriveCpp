#pragma once

#include <string>
#include <string_view>
#include <functional>

#pragma warning(push)
#pragma warning(disable : 4996)
#include <spdlog/spdlog.h>
#pragma warning(pop)

#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif  // _WIN32



namespace utils::logging {
    class LogConsole {
      public:
        using MessageReceivedCallback = std::function<void(std::string)>;
        LogConsole(const std::string_view &name, uint16_t port = 0, MessageReceivedCallback msgCallback = nullptr);
        ~LogConsole();

        void logMessage(const std::string &message, spdlog::level::level_enum level);

        void create();
        void destroy();
        bool isRunning() const;

      private:
        void connect();
        std::string _name;
        uint16_t _port;
        MessageReceivedCallback _msgReceivedCallback;
        bool _isRunning = false;
        std::thread _listener;
#ifdef _WIN32
        STARTUPINFO _si;
        PROCESS_INFORMATION _pi;
#endif  // _WIN32
    };

}  // namespace utils::logging
