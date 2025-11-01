cmake_minimum_required(VERSION 3.14.0)
project(MyApi LANGUAGES C)

add_compile_options(-Werror -Wall) 

set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE MyApi
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(MyApi ${MyApi})

target_include_directories(MyApi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/services
    ${CMAKE_CURRENT_SOURCE_DIR}/include/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/config
)

find_package(SQLite3 REQUIRED)
target_link_libraries(MyApi PRIVATE SQLite::SQLite3)