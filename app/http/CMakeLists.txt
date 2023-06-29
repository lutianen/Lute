set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE srcs CONFIGURE_DEPENDS src/*.cc include/*.h)
add_executable(httpServer ${srcs})
target_include_directories(httpServer PUBLIC include)
target_link_libraries(httpServer PRIVATE Lute_Base
                                 PRIVATE Lute_Polaris 
                                 PRIVATE Lute_MySQL 
                                 PRIVATE Lute_Redis 
                                 PRIVATE mysqlclient )


