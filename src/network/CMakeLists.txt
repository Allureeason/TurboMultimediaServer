aux_source_directory(. DIR_SRCS)
aux_source_directory(base DIR_SRCS)
aux_source_directory(net DIR_SRCS)

# 添加包含路径
include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

set(LINK_LIBS
    base
    pthread
)

add_library(network ${DIR_SRCS})
target_link_libraries(network ${LINK_LIBS})

add_subdirectory(test)