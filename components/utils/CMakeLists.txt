idf_component_register(SRCS "astring.cpp"
                    INCLUDE_DIRS .
		    #PRIV_REQUIRES extrstream
		    #REQUIRES console driver sdmmc spi_flash fatfs cxx
		    )

