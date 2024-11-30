#ifndef __BSP_MAIN_H__
#define __BSP_MAIN_H__

#include "bsp_uart.h"
#include "io_controller.h"
#include "i2c_device_tree.h"
#include "i2c_dev_com.h"
#include "ads1115.h"
#include "dac8571.h"
#include "485_slave.h"
#include "rly_controller.h"
#include "485_master.h"

#define DEV_ADR 0x50;
typedef struct
{
    uint8_t addr;
    master485_t master485;
} esp32_t;
extern esp32_t esp32;
esp_err_t esp32_init(esp32_t *esp32);

typedef enum
{
    JPUMP_ADR = 0x01,
    SCREEN_ADR = 0x05,
    RLY_ADR = 0x09,
    VACUUM_ADR = 0x07,
    MPUMP_ADR = 0x10
} dev_adr_t;
// jpump 1A03-voltage 1A04-current 1A0A-r_speed 1A27-temperature1 1A28-temperature2
typedef enum
{
    JPUMP_OPERATE_ADR = 0x6000,
    JPUMP_STATUS_ADR = 0x6001,
    JPUMP_VOLTAGE_ADR = 0x1A03,
    JPUMP_CURRENT_ADR = 0x1A04,
    JPUMP_R_SPEED_ADR = 0x1A0A,
    JPUMP_TEMPERATURE1_ADR = 0x1A27,
    JPUMP_TEMPERATURE2_ADR = 0x1A28
} jpump_reg_t;

typedef enum
{
    MPUMP_OPERATE_ADR = 0x2000,
    MPUMP_DATA_STATUS_ADR = 0x1000, // 1000 0004 r_speed, voltage, current, status
    MPUMP_TEMPERATURE_ADR = 0x1004
} MPUMP_reg_t;

typedef enum
{
    SCREEN_MPUMP_FIRST_ADR = 0x0010,
    SCREEN_VACUUM_FIRST_ADR = 0x0020,
    SCREEN_JMPUMP_FIRST_ADE = 0x0060
} screen_reg_t;
#endif