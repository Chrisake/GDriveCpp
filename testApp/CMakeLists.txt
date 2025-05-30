# TestApp
add_executable(${TESTAPP_EXE}
  "${GDRIVECPP_SOURCE_DIR}/testApp/main.cpp"
)

target_include_directories(${TESTAPP_EXE}
  PRIVATE
    "${GDRIVECPP_SOURCE_DIR}/source/public_include/"
    "${CMAKE_CURRENT_SOURCE_DIR}/.secrets"
)

add_compile_definitions(${TESTAPP_EXE}
  GDRIVECPP_VERSION_MAJOR=${PROJECT_MAJOR_VERSION}
  GDRIVECPP_VERSION_MINOR=${PROJECT_MINOR_VERSION}
)

target_link_libraries(
  ${TESTAPP_EXE}
  ${GDRIVE_DLL}
  ${LIBUTILS_OBJ}
  spdlog::spdlog
)

set_target_properties(${TESTAPP_EXE} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/GDriveCpp/demo/"
)

add_custom_command(
    TARGET ${TESTAPP_EXE}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${GDRIVE_DLL}>     
        $<TARGET_FILE_DIR:${TESTAPP_EXE}>
    COMMENT "Copying ${GDRIVE_DLL}.dll to ${TESTAPP_EXE}'s runtime directory"
)
