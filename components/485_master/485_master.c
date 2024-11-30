#include "485_master.h"
#include "stdio.h"
#include "esp_crc.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "bsp_uart.h"

modbus_cmd_t *read_cmd_list = NULL;
modbus_cmd_t *write_cmd_list = NULL;
master485_err_t master485_check(master485_t *master485);
uint16_t crc16(uint8_t const *data, int32_t data_len, uint16_t poly)
{
    uint16_t crc_value = 0xFFFF;
    // reverse poly
    uint16_t poly_rev = 0;
    for (int i = 0; i < 16; i++)
    {
        if (poly & (1 << i))
            poly_rev |= (1 << (15 - i));
    }
    poly = poly_rev;
    while (data_len--)
    {
        crc_value ^= *data++;
        for (int i = 0; i < 8; i++)
            if (crc_value & 0x0001)
                crc_value = (crc_value >> 1) ^ poly;
            else
                crc_value >>= 1;
    }
    return crc_value;
}

master485_err_t master485_check(master485_t *master485)
{
    if (master485 == NULL)
        return MASTER485_NOT_EXIST;
    else if (master485->dev == NULL)
        return MASTER485_NO_DEVICE;
    else if (master485->send == NULL)
        return MASTER485_NO_SEND_HANDLER;
    return MASTER485_OK;
}
master485_err_t master485_init(master485_t *master485, void (*send)(uint8_t *data, int len, void *user))
{
    master485->send = send;
    master485->receive = master485_receive;
    master485->dev = malloc(sizeof(rs485_dev_t));
    master485->dev->next = NULL;
    master485->wait_ack = false;
    return MASTER485_OK;
}

master485_err_t master485_add_dev(master485_t *master485, uint8_t dev_addr, int reg_count, int coil_count, int timeout)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK && err != MASTER485_NO_DEVICE)
        return err;
    rs485_dev_t *dev = master485->dev;
    // check if dev_addr exist
    while (dev->next != NULL)
    {
        if (dev->dev_addr == dev_addr)
            return MASTER485_DEV_EXIST;
        dev = dev->next;
    }
    dev = malloc(sizeof(rs485_dev_t));
    dev->dev_addr = dev_addr;
    dev->reg_count = reg_count;
    dev->coil_count = coil_count;
    dev->timeout = timeout;
    dev->reg_value = malloc(sizeof(uint16_t) * reg_count);
    memset(dev->reg_value, 0, sizeof(uint16_t) * reg_count);

    dev->coil_value = malloc(coil_count / 8 + 1);
    memset(dev->reg_value, 0, coil_count / 8 + 1);
    // head add dev
    dev->next = master485->dev->next;
    master485->dev->next = dev;
    return MASTER485_OK;
}
master485_err_t master485_remove_dev(master485_t *master485, uint8_t dev_addr)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    // find dev and remove it
    rs485_dev_t *dev = master485->dev;

    while (dev->next != NULL)
    {
        // rs485_dev_t *dev_next = dev->next;
        if (dev->next->dev_addr == dev_addr)
        {
            rs485_dev_t *temp = dev->next;
            dev->next = temp->next;
            free(temp->reg_value);
            free(temp);
            return MASTER485_OK;
        }
        dev = dev->next;
    }
    return MASTER485_DEV_NOT_EXIST;
}
/**
 * @brief 485 read format:dev_addr 03 reg_addr reg_count crc
 * @param index 将读取到的数据记录到内存中的索引
 */
master485_err_t master485_read_regs(master485_t *master485, uint8_t dev_addr, uint16_t reg_addr, uint16_t reg_count, int index)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    rs485_dev_t *dev = master485->dev->next;
    while (dev != NULL)
    {
        if (dev->dev_addr == dev_addr)
            break;
        dev = dev->next;
    }
    if (dev == NULL)
    {
        return MASTER485_DEV_NOT_EXIST;
    }
    if (dev->dev_addr == dev_addr)
    {
        // if (reg_addr + reg_count > dev->reg_count)
        //     return MASTER485_READ_OUT_OF_RANGE;
        uint8_t frame[8];
        frame[0] = dev_addr;
        frame[1] = 0x03;
        frame[2] = reg_addr >> 8;
        frame[3] = reg_addr & 0xFF;
        frame[4] = reg_count >> 8;
        frame[5] = reg_count & 0xFF;
        uint16_t crc = crc16(frame, 6, 0x8005);
        frame[6] = (uint8_t)(crc & 0xFF);
        frame[7] = (uint8_t)(crc >> 8);
        // save last cmd
        // master485->last_cmd.dev_addr = dev_addr;
        // master485->last_cmd.cmd_type = 0x03;
        // master485->last_cmd.reg_addr = reg_addr;
        // master485->last_cmd.reg_count = reg_count;
        // master485->last_cmd.index = index;
        master485->send(frame, 8, (void *)index);
        // wait ack
        // int timeout = dev->timeout;
        // master485->wait_ack = true; // seuccess
        // printf("send wait_ack:%d\n", master485->wait_ack);
        // while (timeout > 0)
        // {
        //     // printf("\ntimeout:%d\n", timeout);

        //     if (master485->wait_ack == false)
        //         break;
        //     timeout -= 10;
        //     vTaskDelay(10 / portTICK_PERIOD_MS);
        // }
        // send cmd
        // master485->wait_ack = false;
    }
    return MASTER485_OK;
}
master485_err_t master485_read_coils(master485_t *master485, uint8_t dev_addr, uint16_t coil_addr, uint16_t coil_count, int index)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    rs485_dev_t *dev = master485->dev->next;
    while (dev != NULL)
    {
        if (dev->dev_addr == dev_addr)
            break;
        dev = dev->next;
    }
    if (dev == NULL)
    {
        return MASTER485_DEV_NOT_EXIST;
    }
    if (dev->dev_addr == dev_addr)
    {
        // if (reg_addr + reg_count > dev->reg_count)
        //     return MASTER485_READ_OUT_OF_RANGE;
        uint8_t frame[8];
        frame[0] = dev_addr;
        frame[1] = 0x01;
        frame[2] = coil_addr >> 8;
        frame[3] = coil_addr & 0xFF;
        frame[4] = coil_count >> 8;
        frame[5] = coil_count & 0xFF;
        uint16_t crc = crc16(frame, 6, 0x8005);
        frame[6] = (uint8_t)(crc & 0xFF);
        frame[7] = (uint8_t)(crc >> 8);
        // save last cmd
        master485->last_cmd.dev_addr = dev_addr;
        master485->last_cmd.cmd_type = 0x01;
        master485->last_cmd.coil_addr = coil_addr;
        master485->last_cmd.coil_count = coil_count;
        master485->last_cmd.index = index;
        master485->send(frame, 8, (void *)index);
        // master485->send(frame, 8);
        // wait ack
        // int timeout = dev->timeout;
        // master485->wait_ack = true; // seuccess
        // printf("send wait_ack:%d\n", master485->wait_ack);
        // while (timeout > 0)
        // {
        //     if (master485->wait_ack == false)
        //         break;
        //     timeout -= 10;
        //     vTaskDelay(10 / portTICK_PERIOD_MS);
        // }
        // send cmd
    }
    // master485->wait_ack = false;
    return MASTER485_OK;
}
master485_err_t master485_write_reg(master485_t *master485, uint8_t dev_addr, uint16_t reg_addr, uint16_t value, int index)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    rs485_dev_t *dev = master485->dev->next;
    while (dev != NULL)
    {
        if (dev->dev_addr == dev_addr)
            break;
        dev = dev->next;
    }
    if (dev == NULL)
    {
        return MASTER485_DEV_NOT_EXIST;
    }
    if (dev->dev_addr == dev_addr)
    {
        uint8_t frame[8];
        frame[0] = dev_addr;
        frame[1] = 0x06;
        frame[2] = reg_addr >> 8;
        frame[3] = reg_addr & 0xFF;
        frame[4] = value >> 8;
        frame[5] = value & 0xFF;
        uint16_t crc = crc16(frame, 6, 0x8005);
        frame[6] = (uint8_t)(crc & 0xFF);
        frame[7] = (uint8_t)(crc >> 8);
        // save last cmd
        master485->last_cmd.dev_addr = dev_addr;
        master485->last_cmd.cmd_type = 0x06;
        master485->last_cmd.reg_addr = reg_addr;
        master485->last_cmd.reg_value = value;
        master485->last_cmd.reg_count = 1;
        master485->last_cmd.index = index;
        // send cmd
        master485->send(frame, 8, (void *)index);
        // master485->send(frame, 8);
        // master485->wait_ack = true;
        // // wait ack
        // printf("send wait_ack:%d\n", master485->wait_ack);
        // int timeout = dev->timeout;
        // while (timeout > 0)
        // {
        //     // printf("\ntimeout:%d\n", timeout);

        //     if (master485->wait_ack == false)
        //         break;
        //     timeout -= 10;
        //     vTaskDelay(10 / portTICK_PERIOD_MS);
        // }
    }
    // master485->wait_ack = false;
    return MASTER485_OK;
}
master485_err_t master485_write_coil(master485_t *master485, uint8_t dev_addr, uint16_t coil_addr, uint16_t value, int index)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    rs485_dev_t *dev = master485->dev->next;
    while (dev != NULL)
    {
        if (dev->dev_addr == dev_addr)
            break;
        dev = dev->next;
    }
    if (dev == NULL)
    {
        return MASTER485_DEV_NOT_EXIST;
    }
    if (dev->dev_addr == dev_addr)
    {
        uint8_t frame[8];
        frame[0] = dev_addr;
        frame[1] = 0x05;
        frame[2] = coil_addr >> 8;
        frame[3] = coil_addr & 0xFF;
        frame[4] = value >> 8;
        frame[5] = value & 0xFF;
        uint16_t crc = crc16(frame, 6, 0x8005);
        frame[6] = (uint8_t)(crc & 0xFF);
        frame[7] = (uint8_t)(crc >> 8);
        // save last cmd
        master485->last_cmd.dev_addr = dev_addr;
        master485->last_cmd.cmd_type = 0x05;
        master485->last_cmd.coil_addr = coil_addr;
        master485->last_cmd.coil_value = value;
        master485->last_cmd.reg_count = 1;
        master485->last_cmd.index = index;
        // send cmd
        master485->send(frame, 8, (void *)index);
        // master485->send(frame, 8);
        //     master485->wait_ack = true;
        //     // wait ack
        //     printf("send wait_ack:%d\n", master485->wait_ack);
        //     int timeout = dev->timeout;
        //     while (timeout > 0)
        //     {
        //         if (master485->wait_ack == false)
        //             break;
        //         timeout -= 10;
        //         vTaskDelay(10 / portTICK_PERIOD_MS);
        //     }
    }
    // master485->wait_ack = false;
    return MASTER485_OK;
}
master485_err_t master485_receive(master485_t *master485, uint8_t *frame_in, int len)
{
    printf("master receive:");
    for (int i = 0; i < len; i++)
    {
        printf("%02X ", frame_in[i]);
    }
    printf("\n");
    // printf("\nrecive wait_ack1:%d\n", master485->wait_ack);
    // master485->wait_ack = false;
    // printf("recive wait_ack2:%d\n", master485->wait_ack);

    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    if (len < 5)
        return MASTER485_receive_ERROR_LEN;
    if (frame_in[0] != master485->last_cmd.dev_addr || frame_in[1] != master485->last_cmd.cmd_type)
    {
        printf("master485->last_cmd.cmd_type:[%02x]\n", master485->last_cmd.cmd_type);
        printf("master485->last_cmd.dev_addr:[%02x]\n", master485->last_cmd.dev_addr);
    }

    // return MASTER485_receive_ERROR_NOT_MATCH;
    // Find device
    rs485_dev_t *dev = master485->dev->next;
    while (dev != NULL)
    {
        if (dev->dev_addr == master485->last_cmd.dev_addr)
            break;
        dev = dev->next;
    }
    if (dev == NULL)
    {
        return MASTER485_DEV_NOT_EXIST;
    }
    // save receive value to mem
    switch (master485->last_cmd.cmd_type) // frame_in[1]
    {
    case 0x03:
        if (len != 5 + master485->last_cmd.reg_count * 2)
            // return MASTER485_receive_ERROR_LEN;
            printf("len:[%d]    master485->last_cmd[%02x %02x %02x %02x]\n", len, master485->last_cmd.dev_addr, master485->last_cmd.cmd_type, master485->last_cmd.reg_addr, master485->last_cmd.reg_count);
        int mem_reg_adr = master485->last_cmd.index;
        printf("mem_reg_adr:[%d]\n", mem_reg_adr);
        for (int i = 0; i < master485->last_cmd.reg_count; i++)
            dev->reg_value[mem_reg_adr + i] = ((uint16_t)frame_in[3 + i * 2] << 8) | (frame_in[4 + i * 2]);

        for (int i = 0; i < dev->reg_count; i++)
        {
            printf("reg_value[%d]:%d\n", i, dev->reg_value[i]);
        }
        break;
    case 0x06:
        if (len != 8)
            // printf("len:[%d]\n", len);
            return MASTER485_receive_ERROR_LEN;
        dev->reg_value[master485->last_cmd.index] = ((uint16_t)frame_in[4] << 8) | (frame_in[5]);
        break;
    case 0x01:
        int byte_count;
        if (master485->last_cmd.coil_count % 8 == 0)
            byte_count = master485->last_cmd.coil_count / 8 + 5;
        else
            byte_count = master485->last_cmd.coil_count / 8 + 6;
        if (len != byte_count)
            return MASTER485_receive_ERROR_LEN;
        int mem_coil_adr = master485->last_cmd.index;
        for (int i = 0; i < master485->last_cmd.reg_count; i++)
            dev->coil_value[mem_coil_adr + i] = frame_in[3 + i];
        break;
    case 0x05:
        if (len != 8)
            return MASTER485_receive_ERROR_LEN;
        break;
    }
    // master485->wait_ack = false;
    // vTaskDelay(3000 / portTICK_PERIOD_MS);
    xSemaphoreGive(uart_b_tx_semophore);
    return MASTER485_OK;
}
// value frome mem to esp32
master485_err_t master485_get_value_frome_mem(master485_t *master485, uint8_t dev_addr, bool read_reg, int index, uint16_t *value)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    // Find device
    rs485_dev_t *dev = master485->dev->next;
    while (dev != NULL)
    {
        if (dev->dev_addr == dev_addr)
            break;
        dev = dev->next;
    }
    if (dev == NULL)
    {
        return MASTER485_DEV_NOT_EXIST;
    }
    // if (reg_addr > dev->reg_count + dev->reg_offset || reg_addr < dev->reg_offset)
    //     return MASTER485_READ_OUT_OF_RANGE;
    // get value
    *value = read_reg ? dev->reg_value[index] : dev->coil_value[index];
    // *value = dev->reg_value[index];
    return MASTER485_OK;
}

master485_err_t master485_read_cmd_list_init(master485_t *master485)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    if (read_cmd_list == NULL)
    {
        read_cmd_list = malloc(sizeof(modbus_cmd_t));
        memset(read_cmd_list, 0, sizeof(modbus_cmd_t));
        return MASTER485_OK;
    }
    else
        return master485_cmd_list_clear(master485); // clear read cmd list
}
master485_err_t master485_write_cmd_list_init(master485_t *master485)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    if (write_cmd_list == NULL)
    {
        write_cmd_list = malloc(sizeof(modbus_cmd_t));
        memset(write_cmd_list, 0, sizeof(modbus_cmd_t));
        return MASTER485_OK;
    }
    else
        return MASTER485_WRITE_CMD_LIST_NOT_EMPTY;
}
master485_err_t master485_cmd_list_clear(master485_t *master485)
{
    master485_err_t err = master485_check(master485);
    if (err != MASTER485_OK)
        return err;
    while (read_cmd_list != NULL)
    {
        modbus_cmd_t *tmp = read_cmd_list;
        read_cmd_list = read_cmd_list->next;
        free(tmp);
    }
    return MASTER485_OK;
}
