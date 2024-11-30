#ifndef __BSP_UART_H__
#define __BSP_UART_H__

#include "driver/uart.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"

typedef enum
{
    UART_A = UART_NUM_1,
    UART_B = UART_NUM_2,
    // UART_SCR = UART_NUM_0,
    UART_MASK = UART_NUM_MAX
} board_uart_port_t;

typedef enum
{
    UART_A_TX_PIN = GPIO_NUM_34,
    UART_A_RX_PIN = GPIO_NUM_33,
    UART_B_TX_PIN = GPIO_NUM_26,
    UART_B_RX_PIN = GPIO_NUM_21,
    // UART_SCR_TX_PIN = GPIO_NUM_1,
    // UART_SCR_RX_PIN = GPIO_NUM_2
} board_uart_pin_t;

typedef struct board_uart_tx
{
    uint8_t *data;
    void (*user)(uint8_t *data, void *args);
    void *user_args;
    uint16_t size;
    uint16_t tx_delay;       // uint is ms
    uint16_t tx_delay_after; // uint is ms
    struct board_uart_tx *next;
} board_uart_tx_t;

void uart_a_rx_task(void *arg);
void uart_a_tx_task(void *arg);
void uart_b_rx_task(void *arg);
void uart_b_tx_task(void *arg);

extern TaskHandle_t uart_a_rx_task_handle;
extern TaskHandle_t uart_a_tx_task_handle;
extern TaskHandle_t uart_b_rx_task_handle;
extern TaskHandle_t uart_b_tx_task_handle;

extern SemaphoreHandle_t uart_a_tx_semophore;
extern SemaphoreHandle_t uart_a_rx_semophore;
extern SemaphoreHandle_t uart_b_tx_semophore;
extern SemaphoreHandle_t uart_b_rx_semophore;
extern SemaphoreHandle_t uart_a_tx_list_mutex;
extern SemaphoreHandle_t uart_b_tx_list_mutex;

typedef void (*board_uart_rx_cb_t)(void *arg, uint8_t *data, int size);
// typedef void (*board_uart_rx_cb_t)(uint8_t *data, int size);

esp_err_t bsp_uart_a_init(int baud_rate);
esp_err_t bsp_uart_b_init(int baud_rate);
// esp_err_t bsp_uart_scr_init(int baud_rate);

esp_err_t bsp_uart_deinit(board_uart_port_t port);

esp_err_t bsp_uart_a_tx(uint8_t *data, uint16_t size, uint16_t tx_delay, uint16_t tx_delay_after);
esp_err_t bsp_uart_b_tx(uint8_t *data, uint16_t size, uint16_t tx_delay, uint16_t tx_delay_after, void (*user)(uint8_t *data, void *args), void *user_args);

esp_err_t bsp_uart_a_register_rx_cb(board_uart_rx_cb_t cb);
esp_err_t bsp_uart_b_register_rx_cb(board_uart_rx_cb_t cb);
void save_last_cmd(uint8_t *data, void *args);

#endif
