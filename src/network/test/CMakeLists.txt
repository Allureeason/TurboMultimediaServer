aux_source_directory(. DIR_SRC)

add_executable(TestEventLoop TestEventLoop.cpp)
target_link_libraries(TestEventLoop base network)

add_executable(TestInetAddress TestInetAddress.cpp)
target_link_libraries(TestInetAddress base network)

add_executable(TestSocketOpt TestSocketOpt.cpp)
target_link_libraries(TestSocketOpt base network)

add_executable(TestAcceptor TestAcceptor.cpp)
target_link_libraries(TestAcceptor base network)

add_executable(TestTcpConnection TestTcpConnection.cpp)
target_link_libraries(TestTcpConnection base network)

add_executable(TestTcpClient TestTcpClient.cpp)
target_link_libraries(TestTcpClient base network)

add_executable(TestTcpServer TestTcpServer.cpp)
target_link_libraries(TestTcpServer base network)

add_executable(TestDnsService TestDnsService.cpp)
target_link_libraries(TestDnsService base network)

add_executable(TestUdpClient TestUdpClient.cpp)
target_link_libraries(TestUdpClient base network)

add_executable(TestUdpServer TestUdpServer.cpp)
target_link_libraries(TestUdpServer base network)