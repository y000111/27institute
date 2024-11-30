#include <stdio.h>
#include "485_slave.h"
#include "string.h"
#include "esp_log.h"
#include "math.h"
#include "freertos/FreeRTOS.h"

// void task_slave_485_reg_updated_cd(void *args);

slave485_t slave485;
// TaskHandle_t __reg_change_cb_task_hd = NULL;

esp_err_t slave485_init(slave485_t *slave485, uint8_t address, uint32_t baudrate)
{
    if (slave485 == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    memset(slave485, 0, sizeof(slave485_t));
    slave485->address = address;
    slave485->baudrate = baudrate;

    // xTaskCreate(task_slave_485_reg_updated_cd,
    //             "task_slave_reg_cb",
    //             1024 * 4,
    //             NULL,
    //             1,
    //             &__reg_change_cb_task_hd);

    return ESP_OK;
}

esp_err_t slave485_deinit(slave485_t *slave485)
{
    return ESP_OK;
}

esp_err_t slave485_analyze(slave485_t *slave485, uint8_t *data, uint16_t size)
{
    return ESP_OK;
}

esp_err_t slave485_add_reg(slave485_t *slave485, slave_reg_t *reg)
{
    if (slave485 == NULL || reg == NULL)
    {
        ESP_LOGW("slave485", "Invalid argument");
        return ESP_ERR_INVALID_ARG;
    }

    slave_reg_t *tmp = &slave485->reg;
    while (tmp->next != NULL)
    {
        if (tmp->next->adr == reg->adr)
        {
            ESP_LOGE("slave485", "Register already exists: %d", reg->adr);
            return ESP_ERR_INVALID_ARG;
        }
        tmp = tmp->next;
    }
    tmp->next = malloc(sizeof(slave_reg_t));
    if (tmp->next == NULL)
    {
        ESP_LOGW("slave485", "Memory allocation failed");
        return ESP_ERR_NO_MEM;
    }

    memset(tmp->next, 0, sizeof(slave_reg_t));
    tmp->next->adr = reg->adr;
    tmp->next->data = malloc(reg->size * sizeof(uint16_t));
    if (tmp->next->data == NULL)
    {
        ESP_LOGW("slave485", "Memory allocation failed");
        free(tmp->next);
        tmp->next = NULL;
        return ESP_ERR_NO_MEM;
    }

    memset(tmp->next->data, 0x00, reg->size * sizeof(uint16_t));

    tmp->next->size = reg->size;
    tmp->next->updated = false;
    tmp->next->cb = reg->cb;

    return ESP_OK;
}

esp_err_t slave485_remove_reg(slave485_t *slave485, uint16_t address)
{
    if (slave485 == NULL)
    {
        ESP_LOGW("slave485", "Invalid argument");
        return ESP_ERR_INVALID_ARG;
    }

    slave_reg_t *tmp = &slave485->reg;
    while (tmp->next != NULL)
    {
        if (tmp->next->adr == address)
        {
            slave_reg_t *tmp2 = tmp->next->next;
            free(tmp->next->data);
            free(tmp->next);
            tmp->next = tmp2;
            return ESP_OK;
        }
        tmp = tmp->next;
    }

    return ESP_OK;
}

esp_err_t slave485_read_regs(slave485_t *slave485, uint16_t address, uint16_t *data, uint16_t size)
{
    if (slave485 == NULL)
    {
        ESP_LOGW("slave485", "Invalid argument");
        return ESP_ERR_INVALID_ARG;
    }

    slave_reg_t *tmp = &slave485->reg;
    bool found = false;
    while (tmp->next != NULL)
    {
        for (int i = 0; i < tmp->next->size; i++)
        {
            if (tmp->next->adr + i == address)
            {
                if (size > tmp->next->size)
                {
                    ESP_LOGW("slave485", "Invalid argument, size: %d", size);
                    return ESP_ERR_INVALID_ARG;
                }
                memcpy(data, &tmp->next->data[i], size * sizeof(uint16_t));
                found = true;
                return ESP_OK;
            }
        }
        // if (tmp->next->adr == address)
        // {
        //     if (size > tmp->next->size)
        //     {
        //         ESP_LOGW("slave485", "Invalid argument, size: %d", size);
        //         return ESP_ERR_INVALID_ARG;
        //     }
        //     memcpy(data, tmp->next->data, size * sizeof(uint16_t));
        //     found = true;
        //     return ESP_OK;
        // }
        tmp = tmp->next;
    }

    return found ? ESP_OK : ESP_ERR_NOT_FOUND;
}

esp_err_t slave485_write_regs(slave485_t *slave485, uint16_t address, uint16_t *data, uint16_t size)
{
    if (slave485 == NULL)
    {
        ESP_LOGW("slave485", "Invalid argument");
        return ESP_ERR_INVALID_ARG;
    }

    slave_reg_t *tmp = &slave485->reg;
    bool found = false;
    while (tmp->next != NULL)
    {
        for (int i = 0; i < tmp->next->size; i++)
        {
            if (tmp->next->adr + i == address)
            {
                if (size > tmp->next->size)
                {
                    ESP_LOGW("slave485", "Invalid argument, size: %d", size);
                    return ESP_ERR_INVALID_ARG;
                }
                memcpy(&tmp->next->data[i], data, size * sizeof(uint16_t));
                found = true;
                // tmp->next->updated = true;
                if (tmp->next->cb != NULL)
                {
                    tmp->next->cb(tmp->next->data);
                }
                return ESP_OK;
            }
        }
        tmp = tmp->next;
    }

    return found ? ESP_OK : ESP_ERR_NOT_FOUND;
}

void print_485_regs()
{
    slave_reg_t *tmp;
    tmp = slave485.reg.next;
    while (tmp != NULL)
    {
        printf("%04X:[ ", tmp->adr);
        for (int i = 0; i < tmp->size; i++)
        {
            printf("%04X ", tmp->data[i]);
        }
        printf("]\n");
        tmp = tmp->next;
    }
}

// void task_slave_485_reg_updated_cd(void *args)
// {
//     ESP_LOGI("slave485","Register updated sync callback task is running");
//     while (true)
//     {
//         slave_reg_t *reg = &slave485.reg;
//         while (reg->next != NULL)
//         {
//             reg = reg->next;
//             if (reg->cb != NULL)
//             {
//                 reg->cb(reg->data);
//             }
//         }
//         vTaskDelay(1);
//     }
// }
