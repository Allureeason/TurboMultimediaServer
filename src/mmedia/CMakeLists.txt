aux_source_directory(. DIR_SRCS)
aux_source_directory(base DIR_SRCS)
aux_source_directory(rtmp DIR_SRCS)

# 添加包含路径
include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

set(LINK_LIBS
    base
    network
    crypto
)

add_library(mmedia ${DIR_SRCS})
target_link_libraries(mmedia ${LINK_LIBS})

add_subdirectory(tests)