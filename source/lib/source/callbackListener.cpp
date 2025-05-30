#include "callbackListener.hpp"

#include <drogon/drogon.h>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>

std::string GCloud::Authentication::listenForCode(uint16_t port) {
  using namespace drogon;
  std::string _code;
  bool _receivedCode = false;
  app().registerHandler(
      "/?code={code}",
      [&_code, &_receivedCode](
          const HttpRequestPtr &req,
          std::function<void(const HttpResponsePtr &)> &&callback,
          const std::string &code) {
        (void)req;
        _code = code;
        _receivedCode = true;
        Json::Value json;
        json["result"] = "ok";
        json["message"] = std::string("hello");
        auto resp = HttpResponse::newHttpJsonResponse(json);
        callback(resp);
        app().getLoop()->quit();
      },
      {Get});
  app().addListener("0.0.0.0", port).setThreadNum(1).run();
  return _code;
}