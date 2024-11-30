idf_component_register(SRCS "bsp_uart.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES driver 485_master main)
