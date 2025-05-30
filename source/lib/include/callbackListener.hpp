#pragma once

#include <string>
#include <cstdint>

namespace GCloud::Authentication {
std::string listenForCode(uint16_t port);
}
