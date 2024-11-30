// Description: IO controller header file.
#ifndef __IO_CONTROLLER_H__
#define __IO_CONTROLLER_H__

#include "driver/gpio.h" // GPIO functions and definitions

typedef enum
{
    MIOO_SCR_PWR = GPIO_NUM_0,
    // Relays
    MIOO_RLY1 = GPIO_NUM_3,
    MIOO_RLY2 = GPIO_NUM_8,
    MIOO_RLY3 = GPIO_NUM_4,
    MIOO_RLY4 = GPIO_NUM_10,
    MIOO_RLY5 = GPIO_NUM_5,
    MIOO_RLY6 = GPIO_NUM_11,
    MIOO_RLY7 = GPIO_NUM_6,
    MIOO_RLY8 = GPIO_NUM_12,
    MIOO_RLY_PCTL = GPIO_NUM_7, // power control
    MIOI_RLY_IND = GPIO_NUM_9,  // 

    MIOO_MPVCTL = GPIO_NUM_13,// power voltage control
    MIOI_MCIND = GPIO_NUM_14,

    MIOO_VPVCTL = GPIO_NUM_15,
    MIOI_VCIND = GPIO_NUM_16,

    MIOO_SPVCTL = GPIO_NUM_18,
    MIOI_SCIND = GPIO_NUM_17,

    MIOI_VMA2_IND = GPIO_NUM_35,
    MIOO_VMAP2_CTL = GPIO_NUM_36,

    MIOI_VMA1_IND = GPIO_NUM_38,
    MIOO_VMAP1_CTL = GPIO_NUM_37,

    MIOO_VMAP3_CTL = GPIO_NUM_40,

    MIOI_RBF181WOK = GPIO_NUM_39,
    MIOO_HEVCTL = GPIO_NUM_41,
    MIOO_HEVRUN = GPIO_NUM_42,
    MIOI_HOHALARM = GPIO_NUM_43,
    MIOO_FWP = GPIO_NUM_46,
} io_list_t;

typedef enum
{
    HIGH = 1,
    LOW = 0
} io_level_t;

void io_init(void);
void io_set_level(io_list_t io, io_level_t level);
io_level_t io_get_level(io_list_t io);

#endif
