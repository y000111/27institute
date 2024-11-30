#include <stdio.h>
#include "io_controller.h"

gpio_num_t input_io_list[] = {
    MIOI_RLY_IND,
    MIOI_MCIND,
    MIOI_VCIND,
    MIOI_SCIND,
    MIOI_VMA2_IND,
    MIOI_VMA1_IND,
    MIOI_RBF181WOK,
    MIOI_HOHALARM,
};

gpio_num_t output_io_list[] = {
    MIOO_SCR_PWR,
    MIOO_RLY1,
    MIOO_RLY2,
    MIOO_RLY3,
    MIOO_RLY4,
    MIOO_RLY5,
    MIOO_RLY6,
    MIOO_RLY7,
    MIOO_RLY8,
    MIOO_RLY_PCTL,

    MIOO_MPVCTL,
    MIOO_VPVCTL,
    MIOO_SPVCTL,
    
    MIOO_VMAP2_CTL,
    MIOO_VMAP1_CTL,
    MIOO_VMAP3_CTL,
    MIOO_HEVCTL,
    MIOO_HEVRUN,
    MIOO_FWP,
};

void io_init(void)
{
    gpio_config_t io_conf;
    {
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = 0;
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;

        for (int i = 0; i < sizeof(input_io_list) / sizeof(gpio_num_t); i++)
        {
            io_conf.pin_bit_mask |= (1ULL << input_io_list[i]);
        }

        gpio_config(&io_conf);
    }

    {
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
        io_conf.pin_bit_mask = 0;
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;

        for (int i = 0; i < sizeof(output_io_list) / sizeof(gpio_num_t); i++)
        {
            io_conf.pin_bit_mask |= (1ULL << output_io_list[i]);
        }
        gpio_config(&io_conf);
    }
}

void io_set_level(io_list_t io, io_level_t level)
{
    gpio_set_level(io, level);
}

io_level_t io_get_level(io_list_t io)
{
    return gpio_get_level(io) ? HIGH : LOW;
}
