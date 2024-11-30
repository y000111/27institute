#ifndef __ADS1115_H__
#define __ADS1115_H__

#include "i2c_device_tree.h" // I2C device tree functions and definitions
#include "i2c_dev_lst.h"     // I2C device list and device address

#define bit(x) (1 << (x))

typedef enum
{
    ads_cfg_bit_os_start = bit(15),
    ads_cfg_bit_os_busy = 0,
    ads_cfg_bit_os_not_busy = bit(15),
    ads_cfg_bit_os_mask = ~bit(15),

    ads_cfg_bit_mux_0_1 = 0,
    ads_cfg_bit_mux_0_3 = bit(12),
    ads_cfg_bit_mux_1_3 = bit(13),
    ads_cfg_bit_mux_2_3 = bit(13) | bit(12),
    ads_cfg_bit_mux_0_gnd = bit(14),
    ads_cfg_bit_mux_1_gnd = bit(14) | bit(12),
    ads_cfg_bit_mux_2_gnd = bit(14) | bit(13),
    ads_cfg_bit_mux_3_gnd = bit(14) | bit(13) | bit(12),
    ads_cfg_bit_mux_mask = ~(bit(14) | bit(13) | bit(12)),

    ads_cfg_bit_pga_6_144 = 0,
    ads_cfg_bit_pga_4_096 = bit(9),
    ads_cfg_bit_pga_2_048 = bit(10),
    ads_cfg_bit_pga_1_024 = bit(10) | bit(9),
    ads_cfg_bit_pga_0_512 = bit(11),
    ads_cfg_bit_pga_0_256 = bit(11) | bit(9),
    ads_cfg_bit_pga_mask = ~(bit(11) | bit(10) | bit(9)),

    ads_cfg_bit_mode_continuous = 0,
    ads_cfg_bit_mode_single = bit(8),
    ads_cfg_bit_mode_mask = ~bit(8),

    ads_cfg_bit_data_rate_8 = 0,
    ads_cfg_bit_data_rate_16 = bit(5),
    ads_cfg_bit_data_rate_32 = bit(6),
    ads_cfg_bit_data_rate_64 = bit(6) | bit(5),
    ads_cfg_bit_data_rate_128 = bit(7),
    ads_cfg_bit_data_rate_250 = bit(7) | bit(5),
    ads_cfg_bit_data_rate_475 = bit(7) | bit(6),
    ads_cfg_bit_data_rate_860 = bit(7) | bit(6) | bit(5),
    ads_cfg_bit_data_rate_mask = ~(bit(7) | bit(6) | bit(5)),

    ads_cfg_bit_comp_mode_normal = 0,
    ads_cfg_bit_comp_mode_window = bit(4),
    ads_cfg_bit_comp_mode_mask = ~bit(4),

    ads_cfg_bit_comp_pol_low = 0,
    ads_cfg_bit_comp_pol_high = bit(3),
    ads_cfg_bit_comp_pol_mask = ~bit(3),

    ads_cfg_bit_comp_latch_disabled = 0,
    ads_cfg_bit_comp_latch_enabled = bit(2),
    ads_cfg_bit_comp_latch_mask = ~bit(2),

    ads_cfg_bit_comp_queue_1 = 0,
    ads_cfg_bit_comp_queue_2 = bit(0),
    ads_cfg_bit_comp_queue_4 = bit(1),
    ads_cfg_bit_comp_queue_disable = bit(1) | bit(0),
    ads_cfg_bit_comp_queue_mask = ~(bit(1) | bit(0)),
} ads1115_cfg_bit_t;

typedef enum
{
    ads_ch0_ch1_diff = ads_cfg_bit_mux_0_1,
    ads_ch0_ch3_diff = ads_cfg_bit_mux_0_3,
    ads_ch1_ch3_diff = ads_cfg_bit_mux_1_3,
    ads_ch2_ch3_diff = ads_cfg_bit_mux_2_3,
    ads_ch0_gnd = ads_cfg_bit_mux_0_gnd,
    ads_ch1_gnd = ads_cfg_bit_mux_1_gnd,
    ads_ch2_gnd = ads_cfg_bit_mux_2_gnd,
    ads_ch3_gnd = ads_cfg_bit_mux_3_gnd,
    ads_chx_mask = ads_cfg_bit_mux_mask,
} ads1115_channel_t;

typedef enum
{
    ads_regp_conv = 0x00,
    ads_regp_cfg = 0x01,
    ads_regp_lo_thresh = 0x02,
    ads_regp_hi_thresh = 0x03,
} ads1115_regp_t;

typedef struct ads1115_t
{
    i2c_dev_lst_t *i2c_dev_lst;
    uint16_t config;        // config
    uint16_t conversion[4]; // conversion
    float vlotage[4];       // voltage
    float gain;             // gain, for voltage calculation factor
    uint16_t threshold_low;
    uint16_t threshold_high;
} ads1115_t;

extern uint8_t ads_adr[ADS1115_NUM];
extern ads1115_t *ads1115[ADS1115_NUM];

esp_err_t ads1115_init(i2c_dev_lst_t *i2c_dev_lst);
esp_err_t ads1115_oneshot_chx(ads1115_t *ads1115, ads1115_channel_t channel);
esp_err_t ads1115_read(ads1115_t *ads1115);

#endif