
# Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cc include/*.h)
add_library(Lute_Polaris STATIC ${srcs})
target_include_directories(Lute_Polaris PUBLIC include)
target_link_libraries(Lute_Polaris PUBLIC Lute_Base)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Build Lute_Polaris test.")
    add_subdirectory(test)
endif()

