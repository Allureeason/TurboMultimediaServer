aux_source_directory(. DIR_SRCS)
aux_source_directory(base DIR_SRCS)

set(LINK_LIBS
    base
    network
)

add_library(mmedia ${DIR_SRCS})
target_link_libraries(mmedia ${LINK_LIBS})