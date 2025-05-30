# GDrive DLL
add_library(${GDRIVE_DLL} SHARED
  "source/clientAuthentication.cpp"
  "source/callbackListener.cpp" 
  "source/file.cpp"
  "source/cache.cpp"
)

# Copy public include headers to target directory
file(COPY "${GDRIVECPP_SOURCE_DIR}/source/public_include/" DESTINATION "${CMAKE_BINARY_DIR}/GDriveCpp/public_include")

target_include_directories(${GDRIVE_DLL} 
    PRIVATE  
      "${GDRIVECPP_SOURCE_DIR}/source/lib/include"
    PUBLIC
      "${CMAKE_BINARY_DIR}/GDriveCpp/public_include"
)

add_compile_definitions(${GDRIVE_DLL}
  GDRIVECPP_VERSION_MAJOR=${PROJECT_MAJOR_VERSION}
  GDRIVECPP_VERSION_MINOR=${PROJECT_MINOR_VERSION}
  GDRIVECPP_BUILD_DLL=1
)

target_link_libraries(${GDRIVE_DLL} 
  cpr::cpr
  Drogon::Drogon
  nlohmann_json::nlohmann_json
  Iconv::Iconv
  ${LIBUTILS_OBJ}
)

set_target_properties(${GDRIVE_DLL}  PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/GDriveCpp/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/GDriveCpp/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/GDriveCpp/bin"
)
