#include "dac8571.h"
#include "esp_log.h"

#if (DAC8571_NUM == 1)
uint8_t dac8571_adr[DAC8571_NUM] = {DAC8571_ADR_A};
#elif (DAC8571_NUM == 2)
uint8_t dac8571_adr[DAC8571_NUM] = {DAC8571_ADR_A, DAC8571_ADR_B};
#endif

dac8571_t *dac8571[DAC8571_NUM] = {NULL};

esp_err_t dac8571_init(i2c_dev_lst_t *i2c_dev_lst)
{
    if (i2c_dev_lst == NULL)
    {
        ESP_LOGE("DAC8571", "I2C device list is NULL");
        return ESP_ERR_INVALID_ARG;
    }
    for (int i = 0; i < DAC8571_NUM; i++)
    {
        i2c_dev_lst_t *i2c_dev = i2c_dev_list_find(i2c_dev_lst, dac8571_adr[i]);
        if (i2c_dev != NULL)
        {
            if (dac8571[i] != NULL)
            {
                free(dac8571[i]);
            }

            dac8571[i] = (dac8571_t *)malloc(sizeof(dac8571_t));
            dac8571[i]->i2c_dev_lst = i2c_dev;
            dac8571[i]->update = false;
            dac8571[i]->value = 0;

            // Configure DAC8571 by writing to the config register
            // power down DAC when initializing
            esp_err_t ret;
            uint8_t data[3] = {0, 0, 0};
            data[0] = dac8571_cfg_bit_power_down;
            data[1] = dac8571_pwmode_pwd_1k_ohm;
            ret = i2c_master_transmit(dac8571[i]->i2c_dev_lst->i2c_dev, data, 3, 20);
            if (ret != ESP_OK)
            {
                ESP_LOGE("DAC8571", "Failed to write to config register");
                return ret;
            }
            ESP_LOGI("DAC8571", "DAC8571 at address 0x%02X initialized", dac8571_adr[i]);
        }
    }
    return ESP_OK;
}

esp_err_t dac8571_set_value(dac8571_t *dac8571, uint16_t value)
{
    if (dac8571 == NULL)
    {
        ESP_LOGE("DAC8571", "DAC8571 is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;
    uint8_t data[3] = {0, 0, 0};
    data[0] = dac8571_cfg_bit_write_treg_load;
    data[1] = value >> 8;
    data[2] = value & 0xFF;
    ret = i2c_master_transmit(dac8571->i2c_dev_lst->i2c_dev, data, 3, 20);

    if (ret != ESP_OK)
    {
        ESP_LOGE("DAC8571", "Failed to write to DAC8571 at address 0x%02X: %s", dac8571->i2c_dev_lst->address, esp_err_to_name(ret));
        return ret;
    }

    dac8571->value = value;

    return ESP_OK;
}

// esp_err_t dac8571_read(dac8571_t *dac8571)
// {
//     if (dac8571 == NULL)
//     {
//         ESP_LOGE("DAC8571", "DAC8571 is NULL");
//         return ESP_ERR_INVALID_ARG;
//     }

//     // Read the status register to check if the conversion is done
//     esp_err_t ret;
//     uint8_t data[3] = {ads_regp_cfg, 0, 0}; // status register
//     uint16_t config;
//     int count = 0;
//     while (true)
//     {
//         ret = i2c_master_transmit_receive(dac8571->i2c_dev_lst->i2c_dev, data, 1, data + 1, 2, 20);
//         if (ret != ESP_OK)
//         {
//             ESP_LOGE("ADS1115", "Failed to read status from ADS1115 at address 0x%02X: %s", dac8571->i2c_dev_lst->address, esp_err_to_name(ret));
//             return ret;
//         }
//         config = (data[1] << 8) | data[2];
//         if (config & ads_cfg_bit_os_start)
//         {
//             // printf("Conversion is not done after %d ms\n", count * 10);
//             break;
//         }
//         vTaskDelay(1);
//         count++;
//     }

//     // Read the conversion register
//     data[0] = ads_regp_conv; // conversion register
//     data[1] = 0;
//     data[2] = 0;
//     ret = i2c_master_transmit_receive(dac8571->i2c_dev_lst->i2c_dev, data, 1, data + 1, 2, 20);
//     if (ret != ESP_OK)
//     {
//         ESP_LOGE("dac8571", "Failed to read conversion from dac8571 at address 0x%02X: %s", dac8571->i2c_dev_lst->address, esp_err_to_name(ret));
//         return ret;
//     }
//     int id = (config >> 12) & 0x03;
//     dac8571->value = (data[1] << 8) | data[2];
//     return ESP_OK;   
// }
