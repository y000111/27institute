#ifndef __485_MASTER_H__
#define __485_MASTER_H__
#include "stdbool.h"
#include "stdint.h"

typedef enum
{
    MASTER485_OK,
    MASTER485_DEV_EXIST,
    MASTER485_DEV_NOT_EXIST,
    MASTER485_NO_DEVICE,
    MASTER485_NO_SEND_HANDLER,
    MASTER485_NOT_EXIST,
    MASTER485_receive_ERROR_LEN,
    MASTER485_receive_ERROR_NOT_MATCH,
    MASTER485_READ_OUT_OF_RANGE,
    MASTER485_WRITE_CMD_LIST_NOT_EMPTY,
    MASTER485_READ_CMD_LIST_EMPTY
} master485_err_t;
typedef struct __rs485_dev_t
{
    uint8_t dev_addr;
    // uint32_t baudrate;
    uint16_t reg_count;
    uint16_t coil_count;
    uint16_t reg_offset;
    uint16_t *reg_value;
    uint8_t *coil_value;
    int timeout; // unit 1ms
    struct __rs485_dev_t *next;
} rs485_dev_t;

typedef struct __modbus_cmd_t
{
    uint8_t dev_addr;
    uint8_t cmd_type;
    uint16_t reg_addr;
    uint16_t reg_count;
    uint16_t reg_value;

    uint16_t coil_addr;
    uint16_t coil_count;
    uint16_t coil_value;
    int index;
    struct __modbus_cmd_t *next;
    int max_retry;
    void (*receive_cb)(void);
    uint16_t spacetime;
} modbus_cmd_t;

typedef struct __master485_t
{
    rs485_dev_t *dev;
    modbus_cmd_t last_cmd;
    bool wait_ack;
    void (*send)(uint8_t *data, int len, void *user);
    master485_err_t (*receive)(struct __master485_t *master485, uint8_t *data, int len);
} master485_t;
// init->add_dev->list_init->list_add->cmd_execute->read/writer_reg(send_cmd)->receive
master485_err_t master485_init(master485_t *master485, void (*send)(uint8_t *data, int len, void *user));
master485_err_t master485_add_dev(master485_t *master485, uint8_t dev_addr, int reg_count, int coil_count, int timeout);
master485_err_t master485_remove_dev(master485_t *master485, uint8_t dev_addr);
master485_err_t master485_read_regs(master485_t *master485, uint8_t dev_addr, uint16_t reg_addr, uint16_t reg_count, int index);
master485_err_t master485_write_reg(master485_t *master485, uint8_t dev_addr, uint16_t reg_addr, uint16_t value, int index);
master485_err_t master485_receive(master485_t *master485, uint8_t *frame_in, int len);                                              // value update to mem
master485_err_t master485_get_value_frome_mem(master485_t *master485, uint8_t dev_addr, bool read_reg, int index, uint16_t *value); // mem to esp32

master485_err_t master485_read_coils(master485_t *master485, uint8_t dev_addr, uint16_t coil_addr, uint16_t coil_count, int index);
master485_err_t master485_write_coil(master485_t *master485, uint8_t dev_addr, uint16_t coil_addr, uint16_t value, int index);

master485_err_t master485_read_cmd_list_init(master485_t *master485);
master485_err_t master485_write_cmd_list_init(master485_t *master485);
master485_err_t master485_cmd_list_clear(master485_t *master485);
master485_err_t master485_cmd_list_add(master485_t *master485, uint8_t cmd_type, uint8_t dev_addr, uint16_t reg_addr, uint16_t reg_count, uint16_t reg_value, int spacetime, void (*receive_cb)(void));
master485_err_t master485_cmd_execute(master485_t *master485);

uint16_t crc16(uint8_t const *data, int32_t data_len, uint16_t poly);
void master485_send(uint8_t *data, int len, void *user);
// void master485_send(uint8_t *data, int len, void *user);

#endif