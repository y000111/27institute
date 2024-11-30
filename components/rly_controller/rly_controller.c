#include <stdio.h>
#include "rly_controller.h"
#include "stdbool.h"
#include "esp_log.h"

gpio_num_t __rly_list[RLY_NUM] = {
    MIOO_RLY1,
    MIOO_RLY2,
    MIOO_RLY3,
    MIOO_RLY4,
    MIOO_RLY5,
    MIOO_RLY6,
    MIOO_RLY7,
    MIOO_RLY8,
};

esp_err_t rly_set(uint8_t id, rly_status_t status)
{

    // param check
    if (id >= RLY_NUM)
    {
        ESP_LOGE("rly_ctl", "RLY id is invalid");
        return ESP_ERR_INVALID_ARG;
    }

    io_level_t level = LOW;
    if (status == RLY_ON)
    {
        level = HIGH;
    }
    else if (status == RLY_OFF)
    {
        level = LOW;
    }
    else
    {
        ESP_LOGE("rly_ctl", "RLY status is invalid");
        return ESP_ERR_INVALID_ARG;
    }

    io_set_level(__rly_list[id], level);
    return ESP_OK;
}

esp_err_t rly_get(uint8_t id)
{
    return ESP_FAIL;
}

esp_err_t relay_module_ope(bool status)
{
    io_set_level(MIOO_RLY_PCTL, HIGH);
    return ESP_OK;
}
