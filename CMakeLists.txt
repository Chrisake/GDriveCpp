cmake_minimum_required (VERSION 3.8)

project ("GDriveCpp")

set(PROJECT_MAJOR_VERSION 0)
set(PROJECT_MINOR_VERSION 1)

option(BUILD_TESTS "Build unit tests for library (Default: OFF)" OFF)
option(BUILD_DEMO_APP "Build demonstration application (Default: OFF)" ON)

set(GDRIVECPP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(GDRIVECPP_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/GDriveCpp/")

include("${GDRIVECPP_SOURCE_DIR}/cmake/compilerSetup.cmake")

find_package(cpr REQUIRED)
find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Iconv REQUIRED)
find_package(spdlog REQUIRED)

set(GDRIVE_DLL "GDriveCpp")
set(TESTAPP_EXE "GDriveCppTestApp")
set(LIBUTILS_OBJ "GDriveCppUtils")

add_subdirectory("source/utils")
add_subdirectory("source/lib")

if(BUILD_DEMO_APP)
  message(STATUS "Building demonstration application: ${TESTAPP_EXE}")
  add_subdirectory("testApp")
endif()
