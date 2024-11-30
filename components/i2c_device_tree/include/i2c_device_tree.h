#ifndef __I2C_DEVICE_TREE_H__
#define __I2C_DEVICE_TREE_H__

// #include "driver/i2c.h"  // I2C functions and definitions
#include "driver/i2c_master.h" // I2C functions and definitions
#include "driver/gpio.h"       // GPIO functions and definitions

#define I2C_SDA GPIO_NUM_48  // I2C SDA pin
#define I2C_SCL GPIO_NUM_47  // I2C SCL pin
#define I2C_SPEED 100 * 1000 // I2C speed

typedef struct i2c_dev_lst_t
{
    uint16_t address;
    i2c_master_dev_handle_t i2c_dev;
    struct i2c_dev_lst_t *next;
} i2c_dev_lst_t;

extern i2c_dev_lst_t i2c_dev_lst;

void i2c_device_tree_init(void);
void i2c_device_tree_deinit(void);
i2c_dev_lst_t *i2c_dev_list_find(i2c_dev_lst_t *i2c_dev_lst, uint16_t address);

#endif