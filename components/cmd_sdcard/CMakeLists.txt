idf_component_register(SRCS "sdcard_ctrl.cpp"
                            "sdcard_io.cpp"
                            "cmd_sdcard.cpp"
                    INCLUDE_DIRS .
                    PRIV_REQUIRES extrstream
                    REQUIRES console driver sdmmc spi_flash fatfs cxx utils
		    )

