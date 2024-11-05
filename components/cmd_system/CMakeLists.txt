idf_component_register(SRCS "cmd_system.c" "cmd_system_common.cpp"
                    INCLUDE_DIRS . #"$ENV{IDF_PATH}/examples/cxx/experimental/experimental_cpp_component/include"
                    PRIV_REQUIRES extrstream
                    REQUIRES console spi_flash driver cxx)

target_sources(${COMPONENT_LIB} PRIVATE cmd_system_sleep.c)
