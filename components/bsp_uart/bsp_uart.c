#include "bsp_uart.h"
#include "freertos/semphr.h"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "string.h"
#include "bsp_main.h" // err

// #include "485_master.h"
// void uart_a_rx_task(void *arg);
// void uart_a_tx_task(void *arg);
// void uart_b_rx_task(void *arg);
// void uart_b_tx_task(void *arg);

TaskHandle_t uart_a_rx_task_handle = NULL;
TaskHandle_t uart_a_tx_task_handle = NULL;
TaskHandle_t uart_b_rx_task_handle = NULL;
TaskHandle_t uart_b_tx_task_handle = NULL;
// TaskHandle_t uart_scr_rx_task_handle = NULL;

board_uart_rx_cb_t __uart_a_rx_cb = NULL;
board_uart_rx_cb_t __uart_b_rx_cb = NULL;

board_uart_tx_t *uart_a_tx_head = NULL;
board_uart_tx_t *uart_b_tx_head = NULL;

SemaphoreHandle_t uart_a_tx_semophore;
// SemaphoreHandle_t uart_a_rx_semophore;
SemaphoreHandle_t uart_a_tx_list_mutex;

SemaphoreHandle_t uart_b_tx_semophore;
// SemaphoreHandle_t uart_b_rx_semophore;
SemaphoreHandle_t uart_b_tx_list_mutex;
esp_err_t bsp_uart_a_init(int baud_rate)
{
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Configure UART
    ESP_ERROR_CHECK(uart_param_config(UART_A, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_A, UART_A_TX_PIN, UART_A_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_A, 1024, 0, 0, NULL, 0));

    uart_a_tx_head = malloc(sizeof(board_uart_tx_t));
    memset(uart_a_tx_head, 0, sizeof(board_uart_tx_t));

    xTaskCreate(uart_a_rx_task, "uart_a_rx_task", 1024 * 4, NULL, 0, &uart_a_rx_task_handle);
    xTaskCreate(uart_a_tx_task, "uart_a_tx_task", 1024 * 4, NULL, 0, &uart_a_tx_task_handle);

    return ESP_OK;
}

esp_err_t bsp_uart_b_init(int baud_rate)
{
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Configure UART
    ESP_ERROR_CHECK(uart_param_config(UART_B, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_B, UART_B_TX_PIN, UART_B_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_B, 1024, 0, 0, NULL, 0));

    uart_b_tx_head = malloc(sizeof(board_uart_tx_t));
    memset(uart_b_tx_head, 0, sizeof(board_uart_tx_t));

    // xTaskCreate(uart_b_rx_task, "uart_b_rx_task", 1024 * 4, NULL, 1, &uart_b_rx_task_handle);
    // xTaskCreate(uart_b_tx_task, "uart_b_tx_task", 1024 * 4, NULL, 1, &uart_b_tx_task_handle);

    return ESP_OK;
}

esp_err_t bsp_uart_deinit(board_uart_port_t port)
{
    ESP_ERROR_CHECK(uart_driver_delete(port));
    return ESP_OK;
}
void uart_a_rx_task(void *arg)
{
    uint8_t data[1024];
    while (1)
    {
        // if (xSemaphoreTake(uart_a_tx_semophore, portMAX_DELAY) == pdTRUE)

        uint32_t len = 0;
        uart_get_buffered_data_len(UART_A, (size_t *)&len);
        if (len > 0)
        {
            uart_read_bytes(UART_A, (void *)data, len, 50);
            // printf("Received %ld bytes\n", len);
            ESP_LOGI("UART_A", "Received %ld bytes", len);
            // ESP_LOG_BUFFER_HEXDUMP("UART_A", data, len, ESP_LOG_INFO);
            if (__uart_a_rx_cb != NULL)
            {
                // __uart_a_rx_cb(arg, data, len);
            }
        }
        // xSemaphoreGive(uart_a_rx_semophore);
        vTaskDelay(1);
    }
}

void uart_a_tx_task(void *arg)
{
    while (true)
    {
        if (xSemaphoreTake(uart_a_tx_semophore, 1000 / portTICK_PERIOD_MS) == pdTRUE)
        {
            if (uart_a_tx_head->next != NULL)
            {
                // make sure to take mutex of list, before removing node from list
                if (xSemaphoreTake(uart_a_tx_list_mutex, portMAX_DELAY) == pdTRUE)
                {
                    board_uart_tx_t *current = uart_a_tx_head->next;
                    ESP_LOGI("UART_A", "Send %d bytes", current->size);
                    // printf("Send %d bytes\n", current->size);
                    for (int i = 0; i < current->size; i++)
                        printf("%02X ", current->data[i]);
                    printf("\n");
                    // current->user(current->data, current->user_args);
                    uart_write_bytes(UART_A, (const char *)current->data, current->size);
                    // uart_wait_tx_done(UART_B, 10 / portTICK_PERIOD_MS);
                    uart_a_tx_head->next = current->next;
                    free(current->data);
                    free(current);
                    xSemaphoreGive(uart_a_tx_list_mutex);
                }
            }
        }
        else
        {
            // if not give semaphore in one second, give it back
            xSemaphoreGive(uart_a_tx_semophore);
        }
        vTaskDelay(1);
    }
}

void uart_b_rx_task(void *arg) // receive task
{
    uint8_t data[1024];
    while (true)
    {
        // printf("Waiting for data...\n");
        uint32_t len = 0;
        uart_get_buffered_data_len(UART_B, (size_t *)&len);
        // printf("received data len [%ld]", len);
        if (len > 0)
        {
            // printf("Received %ld bytes\n", len);
            ESP_LOGI("UART_B", "Received %ld bytes", len);
            uart_read_bytes(UART_B, (void *)data, len, 10);
            // ESP_LOG_BUFFER_HEXDUMP("UART_B", data, len, ESP_LOG_INFO);
            if (__uart_b_rx_cb != NULL)
            {
                // __uart_b_rx_cb(arg, data, len);
                __uart_b_rx_cb(data, len);
            }
            // vTaskDelay(3000 / portTICK_PERIOD_MS);
            // xSemaphoreGive(uart_b_tx_semophore);
        }
        vTaskDelay(1);
    }
    free(data);
}
void uart_b_tx_task(void *arg) // send task
{
    while (true)
    {
        if (xSemaphoreTake(uart_b_tx_semophore, 1000 / portTICK_PERIOD_MS) == pdTRUE)
        {
            // board_uart_tx_t *read_cmd = uart_b_tx_head;
            if (uart_b_tx_head->next != NULL)
            {
                // make sure to take mutex of list, before removing node from list
                if (xSemaphoreTake(uart_b_tx_list_mutex, portMAX_DELAY) == pdTRUE)
                {
                    board_uart_tx_t *current = uart_b_tx_head->next;
                    ESP_LOGI("UART_B", "Send %d bytes", current->size);
                    // printf("Send %d bytes\n", current->size);
                    for (int i = 0; i < current->size; i++)
                        printf("%02X ", current->data[i]);
                    printf("\n");
                    current->user(current->data, current->user_args);
                    uart_write_bytes(UART_B, (const char *)current->data, current->size);
                    // uart_wait_tx_done(UART_B, 10 / portTICK_PERIOD_MS);
                    uart_b_tx_head->next = current->next;
                    free(current->data);
                    free(current);
                    xSemaphoreGive(uart_b_tx_list_mutex);
                }
            }
        }
        else
        {
            // if not give semaphore in one second, give it back
            xSemaphoreGive(uart_b_tx_semophore);
        }
        vTaskDelay(1);
    }
}

esp_err_t bsp_uart_a_register_rx_cb(board_uart_rx_cb_t cb)
{
    if (__uart_a_rx_cb != NULL)
    {
        ESP_LOGW("UART_A", "UART_A already has a callback registered");
        return ESP_FAIL;
    }

    if (cb == NULL)
    {
        ESP_LOGE("UART_A", "Callback is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    __uart_a_rx_cb = cb;
    return ESP_OK;
}

esp_err_t bsp_uart_b_register_rx_cb(board_uart_rx_cb_t cb)
{
    if (__uart_b_rx_cb != NULL)
    {
        ESP_LOGW("UART_B", "UART_B already has a callback registered");
        return ESP_FAIL;
    }

    if (cb == NULL)
    {
        ESP_LOGE("UART_B", "Callback is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    __uart_b_rx_cb = cb;
    return ESP_OK;
}
esp_err_t bsp_uart_a_tx(uint8_t *data, uint16_t size, uint16_t tx_delay, uint16_t tx_delay_after)
{

    if (uart_a_tx_head == NULL)
    {
        ESP_LOGE("UART_A", "UART_A is not initialized");
        return ESP_FAIL;
    }

    board_uart_tx_t *current = uart_a_tx_head;
    while (current->next != NULL)
    {
        current = current->next;
    }

    current->next = malloc(sizeof(board_uart_tx_t));
    current = current->next;
    current->data = malloc(size);
    memcpy(current->data, data, size);
    current->size = size;
    current->tx_delay = tx_delay;
    current->tx_delay_after = tx_delay_after;
    current->next = NULL;

    return ESP_OK;
}
// inlist
esp_err_t bsp_uart_b_tx(uint8_t *data, uint16_t size, uint16_t tx_delay, uint16_t tx_delay_after, void (*user)(uint8_t *data, void *args), void *user_args)
{

    if (uart_b_tx_head == NULL)
    {
        ESP_LOGE("UART_B", "UART_B is not initialized");
        return ESP_FAIL;
    }

    if (xSemaphoreTake(uart_b_tx_list_mutex, portMAX_DELAY) == pdTRUE)
    {

        board_uart_tx_t *current = uart_b_tx_head;
        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = malloc(sizeof(board_uart_tx_t));
        current = current->next;
        current->data = malloc(size);
        memcpy(current->data, data, size);
        current->size = size;
        current->tx_delay = tx_delay;
        current->tx_delay_after = tx_delay_after;
        current->user = user;
        current->user_args = user_args;
        current->next = NULL;

        // give mutex back
        xSemaphoreGive(uart_b_tx_list_mutex);
    }

    return ESP_OK;
}

void save_last_cmd(uint8_t *data, void *args) // err
{
    esp32.master485.last_cmd.dev_addr = data[0];
    esp32.master485.last_cmd.cmd_type = data[1];
    esp32.master485.last_cmd.reg_addr = ((uint16_t)data[2] << 8) | data[3];
    esp32.master485.last_cmd.reg_count = ((uint16_t)data[4] << 8) | data[5];
    esp32.master485.last_cmd.index = (int)args;
}
