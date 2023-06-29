cmake_minimum_required(VERSION 3.12)
project(Lute LANGUAGES CXX)

# Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wold-style-cast")

# submodules
add_subdirectory(Base)
add_subdirectory(polaris)

