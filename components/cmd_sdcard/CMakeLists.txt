idf_component_register(SRCS "fs_ctrl.cpp"
			    "cmd_sdcard.cpp"
                    INCLUDE_DIRS .
                    PRIV_REQUIRES extrstream fake_cwd sd-ctrl
                    REQUIRES console driver sdmmc spi_flash fatfs cxx utils
		    )

