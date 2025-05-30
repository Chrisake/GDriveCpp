add_library(${LIBUTILS_OBJ} OBJECT
    "source/directory.cpp"
    "source/data.cpp"
    "source/actions.cpp"
    "source/logging.cpp"
)

target_include_directories(${LIBUTILS_OBJ}
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${LIBUTILS_OBJ}
    openssl::openssl
    spdlog::spdlog
)
