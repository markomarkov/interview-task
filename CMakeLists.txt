cmake_minimum_required(VERSION 3.10)

set(BOOST_ROOT "C:/libs/boost_1_88_0")

set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Could not find Boost!")
endif()

set(CMAKE_CXX_STANDARD 17)

include_directories(include)




# ImGui as git submodule
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/imgui/imgui.h")
    message(STATUS "ImGui not found, initializing submodule...")
    execute_process(COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

# nlohmann/json as git submodule
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/external/json/single_include/nlohmann/json.hpp")
    message(STATUS "nlohmann/json not found, initializing submodule...")
    execute_process(COMMAND git submodule add https://github.com/nlohmann/json external/json
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(COMMAND git submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

include_directories(${CMAKE_SOURCE_DIR}/external/json/single_include)

set(IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/imgui_demo.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_win32.cpp
    ${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
)

file(GLOB SRC_CPP
    src/data/*.cpp
    src/core/*.cpp
    src/net/*.cpp
    src/gui/*.cpp
)

add_executable(cpp_task src/main.cpp ${SRC_CPP} ${IMGUI_SRC})

include_directories(${CMAKE_SOURCE_DIR}/external/imgui)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui/backends)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(cpp_task PRIVATE ws2_32 OpenGL::GL dwmapi)

set(OPENSSL_ROOT_DIR "C:/msys64/mingw64")
set(OPENSSL_INCLUDE_DIR "C:/msys64/mingw64/include")
set(OPENSSL_LIBRARIES "C:/msys64/mingw64/lib")

find_package(OpenSSL REQUIRED)
target_link_libraries(cpp_task PRIVATE OpenSSL::SSL OpenSSL::Crypto)
