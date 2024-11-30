#include "i2c_device_tree.h"
#include "stdbool.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "stdio.h"

void i2c_device_detect(void);
void i2c_dev_list_add(i2c_dev_lst_t *i2c_dev_lst, i2c_master_dev_handle_t i2c_dev, uint16_t address);
void i2c_dev_list_clear(i2c_dev_lst_t *i2c_dev_lst);
void i2c_dev_list_print(i2c_dev_lst_t *i2c_dev_lst);

i2c_master_bus_handle_t i2c_master_bus = NULL;
i2c_dev_lst_t i2c_dev_lst;

void i2c_device_tree_init(void)
{
    if (i2c_master_bus != NULL)
    {
        ESP_LOGW("I2C", "I2C already initialized");
        return;
    }
    {
        i2c_master_bus_config_t i2c_master_bus_cfg = {
            .sda_io_num = I2C_SDA,
            .scl_io_num = I2C_SCL,
            .i2c_port = I2C_NUM_0,
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .glitch_ignore_cnt = 7,
            .intr_priority = 0,
            .trans_queue_depth = 0,
            .flags.enable_internal_pullup = true,
        };

        esp_err_t ret = i2c_new_master_bus(&i2c_master_bus_cfg, &i2c_master_bus);
        if (ret != ESP_OK)
        {
            ESP_LOGE("I2C", "Failed to initialize I2C master bus: %s", esp_err_to_name(ret));
            return;
        }

        // detect I2C devices and add them to the device list
        i2c_device_detect();
    }
}

void i2c_device_tree_deinit(void)
{
    if (i2c_master_bus == NULL)
    {
        ESP_LOGW("I2C", "I2C already deleted");
        return;
    }
    if (i2c_dev_lst.next != NULL)
    {
        i2c_dev_lst_t *current = i2c_dev_lst.next;
        while (current != NULL)
        {
            i2c_master_bus_rm_device(current->i2c_dev);
            i2c_dev_lst.next = current->next;
            free(current);
            current = i2c_dev_lst.next;
        }
    }

    i2c_del_master_bus(i2c_master_bus);
    i2c_master_bus = NULL;
}

void i2c_device_detect(void)
{
    //   uint8_t address;
    // printf("Scanning I2C bus...\n");
    for (int i = 1; i < 127; i++)
    {
        if (i2c_master_probe(i2c_master_bus, i, 20) == ESP_OK)
        {
            // this is a brocast address, skip it
            if (i == 0x00)
                continue;

            // printf("Device found at address: 0x%02X\n", i);

            i2c_device_config_t i2c_device_cfg = {
                .dev_addr_length = I2C_ADDR_BIT_LEN_7,
                .device_address = i,
                .scl_speed_hz = I2C_SPEED,
                .scl_wait_us = 10,
                .flags.disable_ack_check = false};
            i2c_master_dev_handle_t i2c_dev;
            if (i2c_master_bus_add_device(i2c_master_bus, &i2c_device_cfg, &i2c_dev) == ESP_OK)
            {
                i2c_dev_list_add(&i2c_dev_lst, i2c_dev, i);
            }
        }
    }
    // printf("I2C bus scan complete\n");

    // i2c_dev_list_print(&i2c_dev_lst);
}

void i2c_dev_list_init(i2c_dev_lst_t *i2c_dev_lst)
{
    i2c_dev_lst->i2c_dev = NULL;
    i2c_dev_lst->next = NULL;
}

void i2c_dev_list_add(i2c_dev_lst_t *i2c_dev_lst, i2c_master_dev_handle_t i2c_dev, uint16_t address)
{
    i2c_dev_lst_t *new_i2c_dev_lst = (i2c_dev_lst_t *)malloc(sizeof(i2c_dev_lst_t));
    new_i2c_dev_lst->i2c_dev = i2c_dev;
    new_i2c_dev_lst->address = address;
    // insert new device at the beginning of the list
    new_i2c_dev_lst->next = i2c_dev_lst->next;
    i2c_dev_lst->next = new_i2c_dev_lst;
}

void i2c_dev_list_clear(i2c_dev_lst_t *i2c_dev_lst)
{
    i2c_dev_lst_t *current = i2c_dev_lst->next;
    while (current != NULL)
    {
        i2c_dev_lst_t *next = current->next;
        free(current);
        current = next;
    }
    i2c_dev_lst->next = NULL;
}

void i2c_dev_list_print(i2c_dev_lst_t *i2c_dev_lst)
{
    i2c_dev_lst_t *current = i2c_dev_lst->next;
    while (current != NULL)
    {
        printf("Device at address 0x%02X\n", current->address);
        current = current->next;
    }
}

i2c_dev_lst_t *i2c_dev_list_find(i2c_dev_lst_t *i2c_dev_lst, uint16_t address)
{
    i2c_dev_lst_t *current = i2c_dev_lst->next;
    while (current != NULL)
    {
        if (current->address == address)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}
