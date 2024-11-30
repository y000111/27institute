#ifndef __DAC8571_H__
#define __DAC8571_H__

#include "i2c_device_tree.h" // I2C device tree functions and definitions
#include "i2c_dev_lst.h"     // I2C device list and device address

#define bit(x) (1 << (x))

typedef enum
{
    dac8571_cfg_bit_write_treg = 0,
    dac8571_cfg_bit_pwdn_with_ps = bit(0),
    dac8571_cfg_bit_write_treg_load = bit(4),
    dac8571_cfg_bit_power_down = bit(0) | bit(4),
    dac8571_cfg_bit_update_with_treg = bit(5),
    dac8571_cfg_bit_bro_load_all_dacs = bit(4) | bit(5),
    dac8571_cfg_bit_bro_load_all_dacs_with_data = bit(4) | bit(5) | bit(2),
    dac8571_cfg_bit_bro_pwdn_all_dacs_with_ps = bit(0) | bit(4) | bit(5) | bit(2),
} dac8571_cfg_bit_t;

typedef enum
{
    dac8571_pwmode_low_power = 0,
    dac8571_pwmode_fast_settling = bit(5),
    dac8571_pwmode_pwd_1k_ohm = bit(6),
    dac8571_pwmode_pwd_100k_ohm = bit(7),
    dac8571_pwmode_pwd_hiz = bit(6) | bit(7),
} dac8571_pwmode_t;

typedef struct
{
    i2c_dev_lst_t *i2c_dev_lst;
    bool update;
    uint16_t value;
} dac8571_t;

extern dac8571_t *dac8571[DAC8571_NUM];
extern uint8_t dac8571_adr[DAC8571_NUM];

esp_err_t dac8571_init(i2c_dev_lst_t *i2c_dev_lst);
esp_err_t dac8571_set_value(dac8571_t *dac8571, uint16_t value);
esp_err_t dac8571_set_voltage(dac8571_t *dac8571, float voltage);
esp_err_t dac8571_set_power_mode(dac8571_t *dac8571, dac8571_pwmode_t pwmode);
esp_err_t dac8571_read(dac8571_t *dac8571);

#endif
