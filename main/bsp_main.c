#include "bsp_main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "math.h"
#include "string.h"

esp32_t esp32;

uint16_t jpump_reg[7] = {JPUMP_OPERATE_ADR, JPUMP_STATUS_ADR, JPUMP_VOLTAGE_ADR, JPUMP_CURRENT_ADR,
                         JPUMP_R_SPEED_ADR, JPUMP_TEMPERATURE1_ADR, JPUMP_TEMPERATURE2_ADR};
uint16_t jpump_reg_value[7]; // jpump_operate_value, jpump_status_value, jpump_voltage_value, jpump_current_value,
                             // jpump_r_speed_value, jpump_temperature1_value, jpump_temperature2_value};
// mpump 1000-1003 r_speed, voltage, current, status *** 1004 temperature

uint16_t mpump_reg[3] = {MPUMP_OPERATE_ADR, MPUMP_DATA_STATUS_ADR, MPUMP_TEMPERATURE_ADR};
uint16_t mpump_reg_value[7]; // mpump_operate_value, mpump_status_value, mpump_voltage_value, mpump_current_value,
                             // mpump_r_speed_value, mpump_temperature_value};
// modbus register changed callback
void relay_control_cb(uint16_t *args);
void slave485_register_init();
esp_err_t esp32_init(esp32_t *esp32)
{
    if (esp32 == NULL)
        return ESP_ERR_INVALID_ARG;
    esp32->addr = 0x50;
    return ESP_OK;
}

void master485_send(uint8_t *data, int len, void *user)
{
    // printf("\nmaster send: %d bytes ", len);
    // for (int i = 0; i < len; i++)
    //     printf("%02X ", data[i]);
    // printf("\n");
    bsp_uart_b_tx(data, len, 50, 50, save_last_cmd, user);
}
void temp(ads1115_channel_t ch)
{
    for (int i = 0; i < ADS1115_NUM; i++)
    {
        if (ads1115[i] != NULL)
        {
            ads1115_oneshot_chx(ads1115[i], ch);
        }
    }
    for (int i = 0; i < ADS1115_NUM; i++)
    {
        if (ads1115[i] != NULL)
        {
            ads1115_read(ads1115[i]);
        }
    }
}

void uart_a_rx_cb(uint8_t *data, uint16_t size)
{
    printf("UART A received: ");
    for (int i = 0; i < size; i++)
    {
        printf("%c", data[i]);
    }
    printf("\n");
}

void uart_b_rx_cb(uint8_t *data, uint16_t size)
{
    printf("UART B received: ");
    for (int i = 0; i < size; i++)
    {
        printf("%02X ", data[i]);
    }
}
// jpump 1A03-voltage 1A04-current 1A0A-r_speed 1A27-temperature1 1A28-temperature2
// 6000-operate(0001-run 0005-close 0007-reset)
// 6001-status(0001-run 0005-standby 0006-fault)
// jpump_reg_value[operate, status, voltage, current, r_speed, temperature1, temperature2]
void jpump_handler(master485_t *master485)
{
    // 01 06 00 02 00 02 A9 CB   set 485 control
    master485_write_reg(master485, JPUMP_ADR, 0x0002, 0x0002, 7);
    // 01 06 00 18 00 00 09 CD   cmd status
    master485_write_reg(master485, JPUMP_ADR, 0x0018, 0x0000, 7);
    // master485_err_t master485_err;
    uint16_t screen_jpump_reg[7];
    for (int i = 0; i < 7; i++)
        screen_jpump_reg[i] = 0x0060 + i;
    for (int i = 0; i < 7; i++)
    {
        master485_read_regs(master485, JPUMP_ADR, jpump_reg[i], 1, i);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        master485_get_value_frome_mem(master485, JPUMP_ADR, true, i, &jpump_reg_value[i]);
        // printf("jpump_mem[%d]:[%04X]\n", i, jpump_reg_value[i]);
        if (i != 0)
            master485_write_reg(master485, SCREEN_ADR, screen_jpump_reg[i], jpump_reg_value[i], 0x0060 + i);
    }
}
// mpump 1000-1003[r_speed, voltage, current, status] 1004 [temperature]
// 2000-operate(0008-run 0003-close 0001-high speed 0002-low speed)
// mpump_reg_value[operate-0x10, voltage-0x12, current-0x13, r_speed-0x14, temperature-0x15]
void mpump_handler(master485_t *master485)
{
    uint16_t screen_mpump_reg[6];
    for (int i = 0; i < 6; i++)
        screen_mpump_reg[i] = 0x0010 + i;
    // master485_read_regs(master485, MPUMP_ADR, mpump_reg[0], 1, 0);
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    vTaskDelay(500 / portTICK_PERIOD_MS);
    master485_read_regs(master485, MPUMP_ADR, mpump_reg[1], 4, 1);
    master485_read_regs(master485, MPUMP_ADR, mpump_reg[2], 1, 5);
    for (int i = 0; i < 6; i++)
    {
        master485_get_value_frome_mem(master485, MPUMP_ADR, true, i, &mpump_reg_value[i]);
        if (i == 1 || i == 2 || i == 3)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            master485_write_reg(master485, SCREEN_ADR, screen_mpump_reg[i + 1], mpump_reg_value[i], 0x0010 + i + 1);
        }
        if (i == 4)
            for (int j = 0; j < 8; j++)
            {
                if (((mpump_reg_value[i] >> j) & 1) == 1)
                    master485_write_coil(master485, SCREEN_ADR, 0x10 + j, 0xFF00, j);
                else
                    master485_write_coil(master485, SCREEN_ADR, 0x10 + j, 0x0000, j);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        if (i == 5)
        {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            master485_write_reg(master485, SCREEN_ADR, screen_mpump_reg[i], mpump_reg_value[i], 0x0010 + i);
        }
    }
}
// operate jpump&mpump
void screen_handler(master485_t *master485)
{
    // master485_err_t master485_err;
    uint16_t jpump_operate, mpump_operate, jpump_status, mpump_status;
    master485_read_regs(master485, SCREEN_ADR, 0x0060, 2, 0x0060); // read jpump operate & status(useless)
    vTaskDelay(100 / portTICK_PERIOD_MS);
    master485_read_regs(master485, SCREEN_ADR, 0x0010, 1, 0x0010); // read mpump operate
    vTaskDelay(100 / portTICK_PERIOD_MS);
    master485_get_value_frome_mem(master485, SCREEN_ADR, true, 0x0060, &jpump_operate);
    master485_get_value_frome_mem(master485, JPUMP_ADR, true, 1, &jpump_status);
    printf("jpump_operate:[%04x]", jpump_operate);
    printf("jpump_status:[%04x]\n", jpump_status);
    if (jpump_operate == 0x0002 && jpump_status == 0x0005)
        master485_write_reg(master485, JPUMP_ADR, JPUMP_OPERATE_ADR, 0x0002, 0);
    else if (jpump_operate == 0x0005 && jpump_status == 0x0002)
        master485_write_reg(master485, JPUMP_ADR, JPUMP_OPERATE_ADR, 0x0005, 0);

    master485_get_value_frome_mem(master485, SCREEN_ADR, true, 0x0010, &mpump_operate);
    master485_get_value_frome_mem(master485, MPUMP_ADR, true, 4, &mpump_status);
    printf("mpump_operate:[%04x]", mpump_operate);
    printf("mpump_status:[%02x]\n", mpump_status);
    mpump_status = (mpump_status >> 4) & 0x01;
    // 03-stop 08-run 02-high 01-low
    if (mpump_status == 0 && mpump_operate == 0x0008) // add vacuum check
        master485_write_reg(master485, MPUMP_ADR, MPUMP_OPERATE_ADR, 0x0008, 0);
    else if (mpump_status == 1)
    {
        if (mpump_operate == 0x0003)
            master485_write_reg(master485, MPUMP_ADR, MPUMP_OPERATE_ADR, 0x0003, 0);
        else if (mpump_operate == 0x0002)
            master485_write_reg(master485, MPUMP_ADR, MPUMP_OPERATE_ADR, 0x0002, 0);
        else if (mpump_operate == 0x0001)
            master485_write_reg(master485, MPUMP_ADR, MPUMP_OPERATE_ADR, 0x0001, 0);
    }
}
// this function is called when the app starts
// and is used to initialize the peripherals, will be called only once
void app_main(void)
{
    io_init();
    i2c_device_tree_init();
    ads1115_init(&i2c_dev_lst);
    dac8571_init(&i2c_dev_lst);
    // master485
    master485_err_t master485_err;
    master485_err = master485_init(&esp32.master485, master485_send);
    // printf("master485_init err: %d\n", master485_err);
    master485_err = master485_add_dev(&esp32.master485, SCREEN_ADR, 0x68, 2, 400); // screen
    master485_err = master485_add_dev(&esp32.master485, MPUMP_ADR, 7, 0, 2000);    // mpump
    //------------------------------------------------------------------------------------
    // jpump 1A03-voltage 1A04-current 1A0A-r_speed 1A27-temperature1 1A28-temperature2
    // 6000-operate(0001-run 0005-standby 0007-reset)
    // 6001-status(0001-run 0005-standby 0006-fault)
    // [operate, status, voltage, current, r_speed, temperature1, temperature2]
    //------------------------------------------------------------------------------------
    master485_err = master485_add_dev(&esp32.master485, JPUMP_ADR, 8, 0, 400); // jpump
    // master485_err = master485_add_dev(&esp32.master485, VACUUM_ADR, 64, 64, 400); // vacuum
    printf("master485_add_dev err: %d\n", master485_err);
    // err = master485_read_regs(&esp32.master485, 0x01, 0x0001, 0x000A);
    // printf("master485_read_regs err: %d\n", err);
    io_set_level(MIOO_VMAP3_CTL, HIGH);
    io_set_level(MIOO_HEVRUN, HIGH);
    io_set_level(MIOO_VPVCTL, HIGH);

    // bsp_uart_a_init(9600);
    // bsp_uart_a_register_rx_cb(master485_receive);
    uart_a_tx_semophore = xSemaphoreCreateBinary();
    uart_a_tx_list_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(uart_a_tx_semophore);
    xSemaphoreGive(uart_a_tx_list_mutex);
    uart_b_tx_semophore = xSemaphoreCreateBinary();
    uart_b_tx_list_mutex = xSemaphoreCreateBinary();
    xSemaphoreGive(uart_b_tx_semophore);
    xSemaphoreGive(uart_b_tx_list_mutex);
    bsp_uart_b_init(9600);
    bsp_uart_a_init(9600);
    xTaskCreate(uart_b_rx_task, "uart_b_rx_task", 1024 * 4, &esp32.master485, 1, &uart_b_rx_task_handle);
    xTaskCreate(uart_b_tx_task, "uart_b_tx_task", 1024 * 4, NULL, 1, &uart_b_tx_task_handle);
    bsp_uart_b_register_rx_cb((void *)master485_receive);
    // bsp_uart_b_register_rx_cb(uart_b_rx_cb);

    // open relay power
    relay_module_ope(true);

    dac8571_set_value(dac8571[0], 0xFFFF);

    // char a = 'P';
    slave485_init(&slave485, 0x50, 9600);

    // init slave regs
    slave485_register_init();

    // uint16_t rly_sta = 0x0080;
    // slave485_write_regs(&slave485, 0x0010, &rly_sta, 1);

    // print_485_regs();
    // int buf[10];
    // for (int i = 0; i < 10; i++)
    // {

    //     // sprintf(buf, "Hello, this is UART A %d times sending\n", i);
    //     // bsp_uart_a_tx((uint8_t *)buf, strlen(buf), 500, 0);
    //     // sprintf(buf, "Hello, this is UART B %d times sending\n", i);
    //     buf[i] = i;
    //     bsp_uart_b_tx((uint8_t *)buf, sizeof(buf), 500, 0);
    // }
    while (1)
    {
        // master485_err = master485_write_coil(&esp32.master485, SCREEN_ADR, 0x0010, 0xFF00, 1);
        // vTaskDelay(50 / portTICK_PERIOD_MS);
        // master485_err = master485_write_coil(&esp32.master485, SCREEN_ADR, 0x0011, 0xFF00, 1);
        // vTaskDelay(50 / portTICK_PERIOD_MS);
        // master485_err = master485_write_coil(&esp32.master485, SCREEN_ADR, 0x0018, 0xFF00, 1);
        // vTaskDelay(50 / portTICK_PERIOD_MS);
        // master485_err = master485_read_coils(&esp32.master485, SCREEN_ADR, 0x0010, 9, 0);
        // vTaskDelay(5000 / portTICK_PERIOD_MS);
        // uint16_t reg_value;
        // uint8_t coil_value;
        // master485_err = master485_get_value_frome_mem(&esp32.master485, 0x05, false, 0, &coil_value);
        // printf("master485_get_value:[%02XH]\n", coil_value);

        temp(ads_ch0_gnd);
        temp(ads_ch1_gnd);
        temp(ads_ch2_gnd);
        temp(ads_ch3_gnd);

        // print dac8571 value
        // for (int i = 0; i < DAC8571_NUM; i++)
        // {
        //     if (dac8571[i] != NULL)
        //     {
        //         printf("DAC8571 %d value: %04X\n", i, dac8571[i]->value);
        //     }
        // }
        // for (int i = 0; i < ADS1115_NUM; i++)
        // {
        //     if (ads1115[i] != NULL)
        //     {
        //         printf("ADS %d voltage V1: %.3f, V2: %.3f, V3: %.3f, V4: %.3f\n", i, ads1115[i]->vlotage[0], ads1115[i]->vlotage[1], ads1115[i]->vlotage[2], ads1115[i]->vlotage[3]);
        //     }
        // }
        float vac = ads1115[0]->vlotage[3]; // ads115[0] vacuum
        vac = vac * (14.7 / 4.7 * 2);
        // printf("VAC: %.3f\n", vac);
        vac = pow(10, ((vac * 1.667) - 9.333));
        if (vac > pow(8, 4) || vac < 0)
            vac = pow(10, 5);
        char vbuf[32];
        memset(vbuf, 0, 32);
        sprintf(vbuf, "%1.1E", vac);
        uint16_t value[2];
        char *cpv = (char *)(&value);
        // cpv[0] = '1';
        // cpv[1] = '0';
        // cpv[2] = '+';
        // cpv[3] = '5';

        cpv[0] = vbuf[0];
        cpv[1] = vbuf[2];
        cpv[2] = vbuf[4];
        cpv[3] = vbuf[6];

        uint16_t vacuum_value[2];
        vacuum_value[0] = (uint16_t)cpv[0] << 8 | ((uint16_t)cpv[1] & 0xFF);
        vacuum_value[1] = (uint16_t)cpv[2] << 8 | ((uint16_t)cpv[3] & 0xFF);
        master485_write_reg(&esp32.master485, SCREEN_ADR, SCREEN_VACUUM_FIRST_ADR, vacuum_value[0], 0);
        master485_write_reg(&esp32.master485, SCREEN_ADR, SCREEN_VACUUM_FIRST_ADR + 1, vacuum_value[1], 1);
        jpump_handler(&esp32.master485);
        mpump_handler(&esp32.master485);
        screen_handler(&esp32.master485);
        // printf("%c%c%c%c\n", cpv[0], cpv[1], cpv[2], cpv[3]);
        // printf("VAC: %1.1E Pa\n", vac);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void slave485_register_init()
{
    // Relay register
    slave_reg_t reg = {
        .adr = 0x0010,
        .size = 1,
        .cb = relay_control_cb};

    slave485_add_reg(&slave485, &reg);

    // ports outputs control
    reg.adr = 0x0011;
    reg.size = 1;
    reg.cb = NULL;
    slave485_add_reg(&slave485, &reg);

    // ports connect status
    reg.adr = 0x00A0;
    reg.size = 1;
    reg.cb = NULL;
    slave485_add_reg(&slave485, &reg);

    // main vaacumeter valve
    reg.adr = 0x00A1;
    reg.size = 2;
    reg.cb = NULL;
    slave485_add_reg(&slave485, &reg);
}

void relay_control_cb(uint16_t *args)
{
    uint16_t rlys = *args;
    for (int i = 0; i < fmin(RLY_NUM, 8); i++)
    {
        if (rlys & (1 << i))
        {
            rly_set(i, RLY_ON);
        }
        else
        {
            rly_set(i, RLY_OFF);
        }
    }
}
