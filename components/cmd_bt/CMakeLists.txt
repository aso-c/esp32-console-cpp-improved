idf_component_register(SRCS "cmd_bt.cpp"
                    INCLUDE_DIRS .
                    REQUIRES console nvs_flash
                    PRIV_REQUIRES bt)
