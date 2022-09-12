cmake_minimum_required(VERSION 3.4)
project(liblinear LANGUAGES CXX)

cmake_policy(SET CMP0057 NEW) # Enable IN_LIST operator
cmake_policy(SET CMP0064 NEW) # Support if (TEST) operator

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include(CMakePackageConfigHelpers)
include(CMakeDependentOption)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(GNUInstallDirs)
include(CTest)

set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 90)


# Compiler check
string(CONCAT is-msvc $<OR:
  $<C_COMPILER_ID:MSVC>,
  $<STREQUAL:${CMAKE_C_COMPILER_FRONTEND_VARIANT},MSVC>
>)

check_cxx_compiler_flag(/W4 LINEAR_LINT_W4)
check_cxx_compiler_flag(/wd4100 LINEAR_LINT_NO_UNUSED_PARAMETER_MSVC)
check_cxx_compiler_flag(/wd4127 LINEAR_LINT_NO_CONDITIONAL_CONSTANT_MSVC)
check_cxx_compiler_flag(/wd4201 LINEAR_LINT_NO_NONSTANDARD_MSVC)
check_cxx_compiler_flag(/wd4206 LINEAR_LINT_NO_NONSTANDARD_EMPTY_TU_MSVC)
check_cxx_compiler_flag(/wd4210 LINEAR_LINT_NO_NONSTANDARD_FILE_SCOPE_MSVC)
check_cxx_compiler_flag(/wd4232 LINEAR_LINT_NO_NONSTANDARD_NONSTATIC_DLIMPORT_MSVC)
check_cxx_compiler_flag(/wd4456 LINEAR_LINT_NO_HIDES_LOCAL)
check_cxx_compiler_flag(/wd4457 LINEAR_LINT_NO_HIDES_PARAM)
check_cxx_compiler_flag(/wd4459 LINEAR_LINT_NO_HIDES_GLOBAL)
check_cxx_compiler_flag(/wd4706 LINEAR_LINT_NO_CONDITIONAL_ASSIGNMENT_MSVC)
check_cxx_compiler_flag(/wd4996 LINEAR_LINT_NO_UNSAFE_MSVC)

check_cxx_compiler_flag(-Wall LINEAR_LINT_WALL) # DO NOT use this under MSVC


# TODO: Place these into its own function
check_cxx_compiler_flag(-Wno-unused-parameter LINEAR_LINT_NO_UNUSED_PARAMETER)
check_cxx_compiler_flag(-Wstrict-prototypes LINEAR_LINT_STRICT_PROTOTYPES)
check_cxx_compiler_flag(-Wextra LINEAR_LINT_EXTRA)

check_cxx_compiler_flag(/utf-8 LINEAR_LINT_UTF8_MSVC)

set(lint-no-unused-parameter $<$<BOOL:${LINEAR_LINT_NO_UNUSED_PARAMETER}>:-Wno-unused-parameter>)
set(lint-strict-prototypes $<$<BOOL:${LINEAR_LINT_STRICT_PROTOTYPES}>:-Wstrict-prototypes>)
set(lint-extra $<$<BOOL:${LINEAR_LINT_EXTRA}>:-Wextra>)
set(lint-w4 $<$<BOOL:${LINEAR_LINT_W4}>:/W4>)
set(lint-no-unused-parameter-msvc $<$<BOOL:${LINEAR_LINT_NO_UNUSED_PARAMETER_MSVC}>:/wd4100>)
set(lint-no-conditional-constant-msvc $<$<BOOL:${LINEAR_LINT_NO_CONDITIONAL_CONSTANT_MSVC}>:/wd4127>)
set(lint-no-nonstandard-msvc $<$<BOOL:${LINEAR_LINT_NO_NONSTANDARD_MSVC}>:/wd4201>)
set(lint-no-nonstandard-empty-tu-msvc $<$<BOOL:${LINEAR_LINT_NO_NONSTANDARD_EMPTY_TU_MSVC}>:/wd4206>)
set(lint-no-nonstandard-file-scope-msvc $<$<BOOL:${LINEAR_LINT_NO_NONSTANDARD_FILE_SCOPE_MSVC}>:/wd4210>)
set(lint-no-nonstandard-nonstatic-dlimport-msvc $<$<BOOL:${LINEAR_LINT_NO_NONSTANDARD_NONSTATIC_DLIMPORT_MSVC}>:/wd4232>)
set(lint-no-hides-local-msvc $<$<BOOL:${LINEAR_LINT_NO_HIDES_LOCAL}>:/wd4456>)
set(lint-no-hides-param-msvc $<$<BOOL:${LINEAR_LINT_NO_HIDES_PARAM}>:/wd4457>)
set(lint-no-hides-global-msvc $<$<BOOL:${LINEAR_LINT_NO_HIDES_GLOBAL}>:/wd4459>)
set(lint-no-conditional-assignment-msvc $<$<BOOL:${LINEAR_LINT_NO_CONDITIONAL_ASSIGNMENT_MSVC}>:/wd4706>)
set(lint-no-unsafe-msvc $<$<BOOL:${LINEAR_LINT_NO_UNSAFE_MSVC}>:/wd4996>)
# Unfortunately, this one is complicated because MSVC and clang-cl support -Wall
# but using it is like calling -Weverything
string(CONCAT lint-default $<
  $<AND:$<BOOL:${LINEAR_LINT_WALL}>,$<NOT:${is-msvc}>>:-Wall
>)
set(lint-utf8-msvc $<$<BOOL:${LINEAR_LINT_UTF8_MSVC}>:/utf-8>)

list(APPEND linear_cflags ${lint-strict-prototypes} ${lint-extra} ${lint-default} ${lint-w4})
list(APPEND linear_cflags ${lint-no-unused-parameter})
list(APPEND linear_cflags ${lint-no-unused-parameter-msvc})
list(APPEND linear_cflags ${lint-no-conditional-constant-msvc})
list(APPEND linear_cflags ${lint-no-nonstandard-msvc})
list(APPEND linear_cflags ${lint-no-nonstandard-empty-tu-msvc})
list(APPEND linear_cflags ${lint-no-nonstandard-file-scope-msvc})
list(APPEND linear_cflags ${lint-no-nonstandard-nonstatic-dlimport-msvc})
list(APPEND linear_cflags ${lint-no-hides-local-msvc})
list(APPEND linear_cflags ${lint-no-hides-param-msvc})
list(APPEND linear_cflags ${lint-no-hides-global-msvc})
list(APPEND linear_cflags ${lint-no-conditional-assignment-msvc})
list(APPEND linear_cflags ${lint-no-unsafe-msvc})
list(APPEND linear_cflags ${lint-utf8-msvc} )

check_cxx_compiler_flag(-Werror LINEAR_LINT_WERROR)
list(APPEND linear_cflags $<$<BOOL:${LINEAR_LINT_WERROR}>:-Werror>)
check_cxx_compiler_flag(-fno-strict-aliasing linear_F_STRICT_ALIASING)
list(APPEND linear_cflags $<$<BOOL:${linear_F_STRICT_ALIASING}>:-fno-strict-aliasing>)
check_cxx_compiler_flag(-Wcast-align LINEAR_LINT_CAST_ALIGN)
list(APPEND linear_cflags $<$<BOOL:${LINEAR_LINT_CAST_ALIGN}>:-Wcast-align>)
check_cxx_compiler_flag(-Wcast-qual LINEAR_LINT_CAST_QUAL)
list(APPEND linear_cflags $<$<BOOL:${LINEAR_LINT_QUAL}>:-Wcast-qual>)
check_cxx_compiler_flag(-Wdisabled-optimization LINEAR_LINT_WDISABLED_OPTIMIZATION)
list(APPEND linear_cflags $<$<BOOL:${LINEAR_LINT_WDISABLED_OPTIMIZATION}>:-Wdisabled-optimization>)


execute_process(COMMAND python tools/create_version_h.py -i include/linear/version.h.in -c configure.ac -o include/linear/version.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
execute_process(COMMAND python tools/create_memory_h.py -i include/linear/memory.h.in -o include/linear/memory.h
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(linear_sources
    src/addrinfo.cpp
    src/auth_context.cpp
    src/auth_context_impl.cpp
    src/condition_variable.cpp
    src/error.cpp
    src/event_loop.cpp
    src/event_loop_impl.cpp
    src/group.cpp
    src/handler_delegate.cpp
    src/log.cpp
    src/log_file.cpp
    src/log_function.cpp
    src/log_stderr.cpp
    src/message.cpp
    src/mutex.cpp
    src/server.cpp
    src/socket.cpp
    src/socket_impl.cpp
    src/tcp_client.cpp
    src/tcp_server.cpp
    src/tcp_server_impl.cpp
    src/tcp_socket.cpp
    src/tcp_socket_impl.cpp
    src/timer.cpp
    src/timer_impl.cpp
    src/ws_client.cpp
    src/ws_server.cpp
    src/ws_server_impl.cpp
    src/ws_socket.cpp
    src/ws_socket_impl.cpp
    src/ssl_client.cpp
    src/ssl_context.cpp
    src/ssl_server.cpp
    src/ssl_server_impl.cpp
    src/ssl_socket.cpp
    src/ssl_socket_impl.cpp
    src/wss_client.cpp
    src/wss_server.cpp
    src/wss_server_impl.cpp
    src/wss_socket.cpp
    src/wss_socket_impl.cpp
    src/x509_certificate.cpp
    include/linear/version.h
    include/linear/memory.h)

add_definitions(-DWITH_SSL)
find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
message("openssl " ${OPENSSL_VERSION})
message(${OPENSSL_SSL_LIBRARIES})
message(${OPENSSL_CRYPTO_LIBRARIES})
set(linear_libraries ${linear_libraries} ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})

add_subdirectory(deps/libtv)
include_directories(deps/libtv/deps/http-parser deps/msgpack/include)
set(linear_libraries ${linear_libraries} tv uv)

add_library(linear STATIC ${linear_sources})
target_compile_definitions(linear PRIVATE ${linear_defines})
target_compile_options(linear PRIVATE ${linear_cflags})
target_include_directories(linear
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
if(CMAKE_SYSTEM_NAME STREQUAL "OS390")
  target_include_directories(linear PUBLIC $<BUILD_INTERFACE:${ZOSLIB_DIR}/include>)
  set_target_properties(linear PROPERTIES LINKER_LANGUAGE CXX)
endif()
target_link_libraries(linear ${linear_libraries})


install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES LICENSE DESTINATION ${CMAKE_INSTALL_DOCDIR})
install(FILES ${PROJECT_BINARY_DIR}/liblinear.pc ${PROJECT_BINARY_DIR}/liblinear-static.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(TARGETS linear EXPORT liblinearConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT liblinearConfig DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/liblinear)

if(MSVC)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

message(STATUS "summary of build options:
    Install prefix:  ${CMAKE_INSTALL_PREFIX}
    Target system:   ${CMAKE_SYSTEM_NAME}
    Compiler:
      C compiler:    ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID})
      CFLAGS:        ${CMAKE_C_FLAGS_${_build_type}} ${CMAKE_C_FLAGS}
")
