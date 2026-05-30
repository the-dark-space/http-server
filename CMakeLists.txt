cmake_minimum_required(VERSION 3.10)

project(http_server)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(server

    src/main.cpp
    src/server.cpp
    src/http_parser.cpp
    src/file_handler.cpp
    src/thread_pool.cpp
    src/logger.cpp
    src/config_manager.cpp
    src/mime_type.cpp
    src/path_validator.cpp
    src/metrics_manager.cpp
    src/response_builder.cpp
    src/router.cpp
    src/request_handler.cpp
    src/static_file_handler.cpp
    src/access_logger.cpp
)