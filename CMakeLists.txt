cmake_minimum_required(VERSION 3.20)
project(cpp_asgi_server CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# fmt
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.1.1
)
FetchContent_MakeAvailable(fmt)

# nlohmann_json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)
FetchContent_MakeAvailable(json)

# llhttp - Using a specific release tarball that contains generated sources
FetchContent_Declare(
    llhttp
    URL https://github.com/nodejs/llhttp/archive/refs/tags/release/v9.1.3.tar.gz
)
FetchContent_MakeAvailable(llhttp)

# Ensure we can find llhttp.h
if(EXISTS "${llhttp_SOURCE_DIR}/include")
    include_directories(${llhttp_SOURCE_DIR}/include)
endif()

# Add llhttp sources directly to avoid linking issues on MinGW
file(GLOB_RECURSE LLHTTP_SOURCES "${llhttp_SOURCE_DIR}/src/*.c")

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(cppcorn ${SOURCES} ${LLHTTP_SOURCES})

target_include_directories(cppcorn PRIVATE src)
target_include_directories(cppcorn PRIVATE ${llhttp_SOURCE_DIR}/include)
target_link_libraries(cppcorn PRIVATE fmt::fmt nlohmann_json::nlohmann_json)

if(WIN32)
    target_link_libraries(cppcorn PRIVATE ws2_32 mswsock)
endif()

if(MSVC)
    target_compile_options(cppcorn PRIVATE /W4)
else()
    target_compile_options(cppcorn PRIVATE -Wall -Wextra -Wpedantic)
endif()
