idf_component_register(SRCS "sdcard_ctrl.cpp"
                            "cmd_sdcard.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES console driver sdmmc spi_flash fatfs cxx)

