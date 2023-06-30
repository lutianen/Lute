set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cc include/*.h)
add_executable(CDNServer ${srcs})
target_include_directories(CDNServer PUBLIC include)
target_link_libraries(CDNServer PRIVATE Lute_Base
                                 PRIVATE Lute_Polaris )


