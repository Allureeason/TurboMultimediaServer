aux_source_directory(. DIR_SRC)

add_executable(TestEventLoop TestEventLoop.cpp)
target_link_libraries(TestEventLoop base network)

add_executable(TestInetAddress TestInetAddress.cpp)
target_link_libraries(TestInetAddress base network)

add_executable(TestSocketOpt TestSocketOpt.cpp)
target_link_libraries(TestSocketOpt base network)