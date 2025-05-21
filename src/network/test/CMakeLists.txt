aux_source_directory(. DIR_SRC)

add_executable(TestEventLoop TestEventLoop.cpp)
target_link_libraries(TestEventLoop base network)