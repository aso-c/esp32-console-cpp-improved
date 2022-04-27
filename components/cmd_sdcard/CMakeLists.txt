idf_component_register(SRCS "cmd_sdcard.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES console driver sdmmc spi_flash fatfs cxx)

