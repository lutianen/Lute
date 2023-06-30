add_executable(EchoServer EchoServer_unit.cc)
target_link_libraries(EchoServer PRIVATE Lute_Base Lute_Polaris)

add_executable(EchoClient EchoClient_unit.cc)
target_link_libraries(EchoClient PRIVATE Lute_Base Lute_Polaris)

add_executable(buffer buffer_unit.cc)
target_link_libraries(buffer PRIVATE Lute_Base Lute_Polaris)
