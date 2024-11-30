#ifndef __RLY_CONTROLLER__
#define __RLY_CONTROLLER__

#include "io_controller.h"
#include "esp_err.h"

#define RLY_NUM 8

typedef enum
{
    RLY_ON = 0xA5,
    RLY_OFF = 0x50
} rly_status_t;

esp_err_t rly_set(uint8_t id, rly_status_t status);
esp_err_t rly_get(uint8_t id);
esp_err_t relay_module_ope(bool status);

#endif
