idf_component_register(SRCS "dac8571.c" "ads1115.c" "i2c_dev_com.c"
                    INCLUDE_DIRS "include"
                    REQUIRES i2c_device_tree driver)
