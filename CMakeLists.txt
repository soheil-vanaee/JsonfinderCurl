cmake_minimum_required(VERSION 3.10)
project(my-json-cli)

set(CMAKE_C_STANDARD 11)

include_directories(include)
add_executable(my-json-cli src/main.c)

find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JANSSON REQUIRED jansson)

target_link_libraries(my-json-cli CURL::libcurl ${JANSSON_LIBRARIES})

