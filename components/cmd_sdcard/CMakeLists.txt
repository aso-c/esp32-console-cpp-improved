idf_component_register(SRCS "fs_ctrl.cpp"
                            "sdcard_io.cpp"
                            "cmd_sdcard.cpp"
			    "cwd_emulate.cpp"
                    INCLUDE_DIRS .
                    PRIV_REQUIRES extrstream
                    REQUIRES console driver sdmmc spi_flash fatfs cxx utils
		    )

