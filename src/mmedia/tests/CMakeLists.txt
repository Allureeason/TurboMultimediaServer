aux_source_directory(. DIR_SRC)

add_executable(RtmpServer RtmpServer.cpp)
target_link_libraries(RtmpServer base network mmedia)