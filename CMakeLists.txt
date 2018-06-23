cmake_minimum_required(VERSION 3.0)

project(cathook-online)

set(CMAKE_CXX_STANDARD 17)

add_library(co-library STATIC "")
add_executable(co-test "")

target_include_directories(co-library PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/co>)

target_include_directories(co-library PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

set_target_properties(co-library PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
set_target_properties(co-test PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

target_compile_definitions(co-library PRIVATE
        _GLIBCXX_USE_CXX11_ABI=0)
target_compile_definitions(co-test PRIVATE
        _GLIBCXX_USE_CXX11_ABI=0)

target_link_libraries(co-test co-library)

add_subdirectory(include)
add_subdirectory(src)