#include "ads1115.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

// uint8_t ads_adr[ADS_NUM] = {0x48, 0x49, 0x4A, 0x4B};
// ads1115_t *ads1115[ADS_NUM] = {NULL, NULL, NULL, NULL};
#if (ADS1115_NUM == 1)
uint8_t ads_adr[ADS1115_NUM] = {ADS1115_ADR_A};
#elif (ADS1115_NUM == 2)
uint8_t ads_adr[ADS1115_NUM] = {ADS1115_ADR_A, ADS1115_ADR_B};
#elif (ADS1115_NUM == 3)
uint8_t ads_adr[ADS1115_NUM] = {ADS1115_ADR_A, ADS1115_ADR_B, ADS1115_ADR_C};
#elif (ADS1115_NUM == 4)
uint8_t ads_adr[ADS1115_NUM] = {ADS1115_ADR_A, ADS1115_ADR_B, ADS1115_ADR_C, ADS1115_ADR_D};
#endif

ads1115_t *ads1115[ADS1115_NUM] = {NULL};

esp_err_t ads1115_init(i2c_dev_lst_t *i2c_dev_lst)
{
    if (i2c_dev_lst == NULL)
    {
        ESP_LOGE("ADS1115", "I2C device list is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    for (int i = 0; i < ADS1115_NUM; i++)
    {
        i2c_dev_lst_t *i2c_dev = i2c_dev_list_find(i2c_dev_lst, ads_adr[i]);
        if (i2c_dev != NULL)
        {
            if (ads1115[i] != NULL)
            {
                free(ads1115[i]);
            }

            ads1115[i] = (ads1115_t *)malloc(sizeof(ads1115_t));
            ads1115[i]->i2c_dev_lst = i2c_dev;
            ads1115[i]->config =
                ads_cfg_bit_pga_4_096 |
                ads_cfg_bit_mode_single |
                ads_cfg_bit_data_rate_128 |
                ads_cfg_bit_comp_mode_normal |
                ads_cfg_bit_comp_pol_low |
                ads_cfg_bit_comp_latch_disabled |
                ads_cfg_bit_comp_queue_disable;
            ads1115[i]->gain = 4.096 / 32768;
            ads1115[i]->threshold_low = 0;
            ads1115[i]->threshold_high = 0;
            ads1115[i]->conversion[0] = 0;
            ads1115[i]->conversion[1] = 0;
            ads1115[i]->conversion[2] = 0;
            ads1115[i]->conversion[3] = 0;
            ads1115[i]->vlotage[0] = 0;
            ads1115[i]->vlotage[1] = 0;
            ads1115[i]->vlotage[2] = 0;
            ads1115[i]->vlotage[3] = 0;

            // Configure ADS1115 by writing to the config register
            esp_err_t ret;
            uint8_t data[3];
            data[0] = ads_regp_cfg;
            data[1] = ads1115[i]->config >> 8;
            data[2] = ads1115[i]->config & 0xFF;
            ret = i2c_master_transmit(ads1115[i]->i2c_dev_lst->i2c_dev, data, 3, 20);
            if (ret != ESP_OK)
            {
                ESP_LOGE("ADS1115", "Failed to write config to ADS1115 at address 0x%02X: %s", ads_adr[i], esp_err_to_name(ret));
                return ret;
            }
            ESP_LOGI("ADS1115", "ADS1115 at address 0x%02X initialized", ads_adr[i]);
        }
        else
        {
            ESP_LOGW("ADS1115", "ADS1115 at address 0x%02X not found", ads_adr[i]);
        }
    }
    return ESP_OK;
}

esp_err_t ads1115_oneshot_chx(ads1115_t *ads1115, ads1115_channel_t channel)
{
    if (ads1115 == NULL)
    {
        ESP_LOGE("ADS1115", "ADS1115 is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (channel & ads_chx_mask)
    {
        ESP_LOGE("ADS1115", "Invalid channel number");
        return ESP_ERR_INVALID_ARG;
    }

    // Configure and start a conversion ADS1115 by writing to the config register
    esp_err_t ret;
    uint8_t data[3];
    data[0] = ads_regp_cfg;
    ads1115->config &= ads_cfg_bit_mux_mask;
    ads1115->config |= channel;
    data[1] = (ads1115->config | ads_cfg_bit_os_start) >> 8;
    data[2] = ads1115->config & 0xFF;
    // printf("config to write: %04X\n", ads1115->config | ads_cfg_bit_os_start);
    ret = i2c_master_transmit(ads1115->i2c_dev_lst->i2c_dev, data, 3, 20);
    if (ret != ESP_OK)
    {
        ESP_LOGE("ADS1115", "Failed to write config to ADS1115 at address 0x%02X: %s", ads1115->i2c_dev_lst->address, esp_err_to_name(ret));
        return ret;
    }
    return ESP_OK;
}

esp_err_t ads1115_read(ads1115_t *ads1115)
{
    if (ads1115 == NULL)
    {
        ESP_LOGE("ADS1115", "ADS1115 is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    // Read the status register to check if the conversion is done
    esp_err_t ret;
    uint8_t data[3] = {ads_regp_cfg, 0, 0};// status register 
    uint16_t config;
    int count = 0;
    while (true)
    {
        ret = i2c_master_transmit_receive(ads1115->i2c_dev_lst->i2c_dev, data, 1, data + 1, 2, 20);
        if (ret != ESP_OK)
        {
            ESP_LOGE("ADS1115", "Failed to read status from ADS1115 at address 0x%02X: %s", ads1115->i2c_dev_lst->address, esp_err_to_name(ret));
            return ret;
        }
        config = (data[1] << 8) | data[2];
        if (config & ads_cfg_bit_os_start)
        {
            // printf("Conversion is not done after %d ms\n", count * 10);
            break;
        }
        vTaskDelay(1);
        count++;
    }

    // Read the conversion register
    data[0] = ads_regp_conv;// conversion register
    data[1] = 0;
    data[2] = 0;
    ret = i2c_master_transmit_receive(ads1115->i2c_dev_lst->i2c_dev, data, 1, data + 1, 2, 20);
    if (ret != ESP_OK)
    {
        ESP_LOGE("ADS1115", "Failed to read conversion from ADS1115 at address 0x%02X: %s", ads1115->i2c_dev_lst->address, esp_err_to_name(ret));
        return ret;
    }
    int id = (config >> 12) & 0x03;
    ads1115->conversion[id] = (data[1] << 8) | data[2];
    ads1115->vlotage[id] = ads1115->conversion[id] * ads1115->gain;
    return ESP_OK;
}
