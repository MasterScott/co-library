cmake_minimum_required(VERSION 3.0)

project(cathook-online)

set(CMAKE_CXX_STANDARD 17)

add_library(co-library STATIC "")
add_executable(co-test "")

target_include_directories(co-library PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/co>)

target_include_directories(co-library PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

target_link_libraries(co-test co-library)

add_subdirectory(include)
add_subdirectory(src)