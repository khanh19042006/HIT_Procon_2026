cmake_minimum_required(VERSION 3.10)
project(HexaUdon CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Collect core source files (excluding main.cpp)
file(GLOB_RECURSE CORE_SOURCES
    "src/io/*.cpp"
    "src/map/*.cpp"
    "src/solver/*.cpp"
)

# Executable chính
add_executable(HexaUdon src/main.cpp ${CORE_SOURCES})

# Executable kiểm thử (Unit test)
add_executable(test_all tests/test_all.cpp ${CORE_SOURCES})
