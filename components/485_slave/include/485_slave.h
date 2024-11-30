#ifndef __485_SLAVE_H__
#define __485_SLAVE_H__

#include "stdint.h"
#include "esp_err.h"
#include "stdbool.h"

// typedef enum
// {
//     SLAVE_REG_TYPE_COILS = 0,
//     SLAVE_REG_TYPE_DISCRETE_INPUTS,
//     SLAVE_REG_TYPE_HOLDING_REGISTERS,
//     SLAVE_REG_TYPE_INPUT_REGISTERS
// } slave_reg_type_t;

typedef struct slave_reg_t
{
    uint16_t adr;
    // slave_reg_type_t type;
    uint16_t *data;
    uint16_t size;
    bool updated;
    struct slave_reg_t *next;
    void (*cb)(uint16_t *data);
} slave_reg_t;

typedef struct
{
    bool reg_updated;
    uint8_t address;
    uint32_t baudrate;
    slave_reg_t reg; // this is head of the linked list
} slave485_t;

extern slave485_t slave485;

esp_err_t slave485_init(slave485_t *slave485, uint8_t address, uint32_t baudrate);
esp_err_t slave485_deinit(slave485_t *slave485);
esp_err_t slave485_analyze(slave485_t *slave485, uint8_t *data, uint16_t size);
esp_err_t slave485_add_reg(slave485_t *slave485, slave_reg_t *reg);
esp_err_t slave485_remove_reg(slave485_t *slave485, uint16_t address);
esp_err_t slave485_read_regs(slave485_t *slave485, uint16_t address, uint16_t *data, uint16_t size);
esp_err_t slave485_write_regs(slave485_t *slave485, uint16_t address, uint16_t *data, uint16_t size);

void print_485_regs();

#endif