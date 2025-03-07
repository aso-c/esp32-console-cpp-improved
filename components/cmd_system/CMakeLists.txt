idf_component_register(SRCS "cmd_system.c" "cmd_system_common.cpp"
                    INCLUDE_DIRS .
                    PRIV_REQUIRES extrstream
                    REQUIRES argtable console spi_flash driver esp_driver_gpio cxx)

if(CONFIG_SOC_DEEP_SLEEP_SUPPORTED OR CONFIG_SOC_LIGHT_SLEEP_SUPPORTED)
    target_sources(${COMPONENT_LIB} PRIVATE cmd_system_sleep.cpp)
endif()
