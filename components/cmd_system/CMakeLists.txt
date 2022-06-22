idf_component_register(SRCS "cmd_system.cpp"
                    INCLUDE_DIRS . "../extrstream" "$ENV{IDF_PATH}/examples/cxx/experimental/experimental_cpp_component/include"
                    REQUIRES console spi_flash cxx)
