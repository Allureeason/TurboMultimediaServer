aux_source_directory(. DIR_SRC)

add_library(base STATIC ${DIR_SRC})

add_executable(test_base ${DIR_SRC})
