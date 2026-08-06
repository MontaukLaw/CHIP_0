#include "main.h"
#include "stm32h7xx_hal_gpio.h"
#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"
#include "user_comm.h"
#include <string.h>

uint8_t uart1_rx_data[2] = {0};

uint8_t test_data[1] = {'S'};

#define UART1_TEST_RX_BUFFER_SIZE (128U * 32U * 2U)
#define UART1_TEST_CRC_SIZE 4U
#define UART1_TEST_UART_FRAME_SIZE (UART1_TEST_RX_BUFFER_SIZE + UART1_TEST_CRC_SIZE)
#define UART1_TEST_RX_TIMEOUT_MS 100U
#define UART1_TEST_CRC_LED_TIME_MS 100U
#define UART1_TEST_CDC_HEADER_SIZE 3U
#define UART1_TEST_CDC_TAIL_SIZE 2U
#define UART1_TEST_CDC_FRAME_SIZE (UART1_TEST_CDC_HEADER_SIZE + UART1_TEST_RX_BUFFER_SIZE + UART1_TEST_CRC_SIZE + UART1_TEST_CDC_TAIL_SIZE)
#define UART1_TEST_CDC_SOURCE_ID 0x01U
#define UART1_TEST_CDC_HEADER_MAGIC_0 0xAAU
#define UART1_TEST_CDC_HEADER_MAGIC_1 0x55U
#define UART1_TEST_CDC_TAIL_MAGIC_0 0x55U
#define UART1_TEST_CDC_TAIL_MAGIC_1 0xAAU

static uint8_t uart1_test_rx_buffer[UART1_TEST_UART_FRAME_SIZE];
static uint8_t uart1_test_cdc_frame[UART1_TEST_CDC_FRAME_SIZE];
static uint8_t uart1_test_cdc_pending;
static uint8_t uart1_test_cdc_in_flight;
static uint8_t uart1_test_crc_led_active;
static uint32_t uart1_test_crc_led_start_tick;

volatile uint16_t sync_1_counter = 0U;
volatile uint8_t get_sync_1 = 0U;

static uint32_t uart1_test_crc32(const uint8_t *data, uint16_t length)
{
    uint32_t crc = 0xFFFFFFFFU;
    uint16_t i;

    for (i = 0U; i < length; ++i)
    {
        uint8_t bit;

        crc ^= data[i];
        for (bit = 0U; bit < 8U; ++bit)
        {
            if ((crc & 1U) != 0U)
            {
                crc = (crc >> 1U) ^ 0xEDB88320U;
            }
            else
            {
                crc >>= 1U;
            }
        }
    }

    return crc ^ 0xFFFFFFFFU;
}

static uint32_t uart1_test_received_crc32(void)
{
    const uint8_t *crc_data = &uart1_test_rx_buffer[UART1_TEST_RX_BUFFER_SIZE];

    /* The sender appends the CRC32 least-significant byte first. */
    return ((uint32_t)crc_data[0]) | ((uint32_t)crc_data[1] << 8U) | ((uint32_t)crc_data[2] << 16U) | ((uint32_t)crc_data[3] << 24U);
}

static void uart1_test_crc_led_flash(void)
{
    /* Board LEDs are active low. */
    HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
    uart1_test_crc_led_start_tick = HAL_GetTick();
    uart1_test_crc_led_active = 1U;
}

static void uart1_test_crc_led_process(void)
{
    if ((uart1_test_crc_led_active != 0U) && ((HAL_GetTick() - uart1_test_crc_led_start_tick) >= UART1_TEST_CRC_LED_TIME_MS))
    {
        HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
        uart1_test_crc_led_active = 0U;
    }
}

static void uart1_test_cdc_reset(void)
{
    uart1_test_cdc_pending = 0U;
    uart1_test_cdc_in_flight = 0U;
}

static uint32_t uart1_test_cdc_prepare_frame(void)
{
    uint32_t crc = uart1_test_crc32(uart1_test_rx_buffer, UART1_TEST_RX_BUFFER_SIZE);
    uint16_t crc_offset = UART1_TEST_CDC_HEADER_SIZE + UART1_TEST_RX_BUFFER_SIZE;

    uart1_test_cdc_frame[0] = UART1_TEST_CDC_HEADER_MAGIC_0;
    uart1_test_cdc_frame[1] = UART1_TEST_CDC_HEADER_MAGIC_1;
    uart1_test_cdc_frame[2] = UART1_TEST_CDC_SOURCE_ID;

    memcpy(&uart1_test_cdc_frame[UART1_TEST_CDC_HEADER_SIZE], uart1_test_rx_buffer, UART1_TEST_RX_BUFFER_SIZE);

    uart1_test_cdc_frame[crc_offset] = (uint8_t)crc;
    uart1_test_cdc_frame[crc_offset + 1U] = (uint8_t)(crc >> 8U);
    uart1_test_cdc_frame[crc_offset + 2U] = (uint8_t)(crc >> 16U);
    uart1_test_cdc_frame[crc_offset + 3U] = (uint8_t)(crc >> 24U);

    uart1_test_cdc_frame[UART1_TEST_CDC_FRAME_SIZE - 2U] = UART1_TEST_CDC_TAIL_MAGIC_0;
    uart1_test_cdc_frame[UART1_TEST_CDC_FRAME_SIZE - 1U] = UART1_TEST_CDC_TAIL_MAGIC_1;

    uart1_test_cdc_in_flight = 0U;
    uart1_test_cdc_pending = 1U;

    return crc;
}

static void uart1_test_cdc_process(void)
{
    USBD_CDC_HandleTypeDef *cdc;

    if (uart1_test_cdc_pending == 0U)
    {
        return;
    }

    if ((hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED) || (hUsbDeviceHS.pClassData == NULL))
    {
        uart1_test_cdc_reset();
        return;
    }

    cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceHS.pClassData;

    /* One call submits the complete 8201-byte frame. */
    if (uart1_test_cdc_in_flight == 0U)
    {
        if (CDC_Transmit_HS(uart1_test_cdc_frame, (uint16_t)UART1_TEST_CDC_FRAME_SIZE) == USBD_OK)
        {
            uart1_test_cdc_in_flight = 1U;
        }

        return;
    }

    /* TxState becomes zero only after the complete frame has left the endpoint. */
    if (cdc->TxState == 0U)
    {
        uart1_test_cdc_reset();
    }
}

static void uart1_rx_test_without_crc(void)
{
    uint16_t rx_length = 0U;
    HAL_StatusTypeDef rtn;
    volatile uint32_t uart_rx_time_ms;
    uint32_t start = HAL_GetTick();

    rtn = HAL_UARTEx_ReceiveToIdle(&huart1, uart1_test_rx_buffer, sizeof(uart1_test_rx_buffer), &rx_length, UART1_TEST_RX_TIMEOUT_MS);

    uart_rx_time_ms = HAL_GetTick() - start;

    if (((rtn == HAL_OK) || (rtn == HAL_TIMEOUT)) && (rx_length == UART1_TEST_UART_FRAME_SIZE))
    {
        (void)uart1_test_cdc_prepare_frame();
    }
    else
    {
        uart1_test_crc_led_flash();
    }
}

static void uart1_rx_test(void)
{
    uint16_t rx_length = 0U;
    HAL_StatusTypeDef rtn;
    uint32_t calculated_crc;
    uint32_t received_crc;
    volatile uint32_t uart_rx_time_ms;
    uint32_t start = HAL_GetTick();

    rtn = HAL_UARTEx_ReceiveToIdle(&huart1, uart1_test_rx_buffer, sizeof(uart1_test_rx_buffer), &rx_length, UART1_TEST_RX_TIMEOUT_MS);

    uart_rx_time_ms = HAL_GetTick() - start;

    if (((rtn == HAL_OK) || (rtn == HAL_TIMEOUT)) && (rx_length == UART1_TEST_UART_FRAME_SIZE))
    {
        calculated_crc = uart1_test_cdc_prepare_frame();

        received_crc = uart1_test_received_crc32();

        if (calculated_crc != received_crc)
        {
            uart1_test_crc_led_flash();
        }
        else
        {
            HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
        }
    }
    else
    {
        /* A short, missing or otherwise failed frame cannot contain a valid CRC. */
        uart1_test_crc_led_flash();
    }
}

void uart1_tx_test(void)
{
    uart1_test_crc_led_process();
    uart1_test_cdc_process();

    /* Do not overwrite the UART buffer while USB is still using it. */
    if (uart1_test_cdc_pending != 0U)
    {
        return;
    }

    if ((hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED) || (hUsbDeviceHS.pClassData == NULL))
    {
        return;
    }

    sync_1_counter = 0;
    get_sync_1 = 0;

    (void)HAL_UART_Transmit(&huart1, test_data, 1, 0xFFFF);

#if 1
    uint8_t adc_idx = 0;
    uint8_t wave_idx = 0;

    // 一共4096次循环, 每次循环发送一个同步信号, 打开一个波形通道, 等待5us, 关闭波形通道, 关闭同步信号.
    for (adc_idx = 0; adc_idx < ADC_CH_PER_CHIP; adc_idx++)
    {
        for (wave_idx = 0; wave_idx < WAVE_CH_NMB; wave_idx++)
        {
            while (get_sync_1 == 0)
                ;
            HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);
            wave_ch_on(wave_idx);
            delay_us(5); // 等待5us
            all_wave_ch_disable();
            get_sync_1 = 0;
        }
    }
#endif

    uart1_rx_test_without_crc();
    // uart1_rx_test();

    // HAL_Delay(100);
    
}

void uart1_tx_test_without_cdc(void)
{
    (void)HAL_UART_Transmit(&huart1, test_data, 1, 0xFFFF);
    uint16_t rx_length = 0U;
    HAL_StatusTypeDef rtn;
    uint32_t calculated_crc;
    uint32_t received_crc;
    volatile uint32_t uart_rx_time_ms;
    // uint32_t start = HAL_GetTick();
    rtn = HAL_UARTEx_ReceiveToIdle(&huart1, uart1_test_rx_buffer, sizeof(uart1_test_rx_buffer), &rx_length, UART1_TEST_RX_TIMEOUT_MS);
    // uart_rx_time_ms = HAL_GetTick() - start;

    // if (((rtn == HAL_OK) || (rtn == HAL_TIMEOUT)) && (rx_length == UART1_TEST_UART_FRAME_SIZE))
    // {
    //     calculated_crc = uart1_test_crc32(uart1_test_rx_buffer, UART1_TEST_RX_BUFFER_SIZE);

    //     received_crc = uart1_test_received_crc32();

    //     if (calculated_crc != received_crc)
    //     {
    //         uart1_test_crc_led_flash();
    //     }
    //     else
    //     {
    //         HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
    //     }
    // }
    // else
    // {
    //     uart1_test_crc_led_flash();
    // }

    // HAL_Delay(200);
}

void main_task_sync_test(void)
{

    uint8_t adc_idx = 0;
    uint8_t wave_idx = 0;

    for (adc_idx = 0; adc_idx < ADC_CH_NMB; adc_idx++)
    {

        for (wave_idx = 0; wave_idx < WAVE_CH_NMB; wave_idx++)
        {

            // step 1: 发送同步信号, 到不同的ADC芯片.
            send_sync_signal(adc_idx, GPIO_PIN_SET);

            // step 2: 打开波形通道, 输出波形
            wave_ch_on(wave_idx);

            // step 3: 等待5us
            delay_us(5); // 等待5us

            // step 4: 关闭波形通道
            all_wave_ch_disable();

            // step 5: 关闭同步信号
            send_sync_signal(adc_idx, GPIO_PIN_RESET);

            delay_us(5); // 等待5us
        }
    }

    HAL_GPIO_TogglePin(TEST_GPIO_Port, TEST_Pin);
}

void main_task(void)
{

    uint8_t adc_idx = 0;
    uint8_t wave_idx = 0;

    for (adc_idx = 0; adc_idx < ADC_CH_NMB; adc_idx++)
    {

        for (wave_idx = 0; wave_idx < WAVE_CH_NMB; wave_idx++)
        {

            // step 1: 发送同步信号, 到不同的ADC芯片.
            send_sync_signal(adc_idx, GPIO_PIN_SET);

            // step 2: 打开波形通道, 输出波形
            wave_ch_on(wave_idx);

            // step 3: 等待5us
            delay_us(5); // 等待5us

            // step 4: 关闭波形通道
            all_wave_ch_disable();

            // step 5: 关闭同步信号
            send_sync_signal(adc_idx, GPIO_PIN_RESET);

            if (comm_uart1_prepare_rx() != HAL_OK)
            {
                MAIN_TASK_LOG("UART1 RX start failed: adc=%u, wave=%u\r\n", (unsigned int)adc_idx, (unsigned int)wave_idx);
                continue;
            }

            // step 6: 等待uart1接收到数据
            if (comm_uart1_wait_2bytes(uart1_rx_data, UART1_RX_TIMEOUT_MS) == HAL_OK)
            {
                uint16_t adc_result_max = 0;
                adc_result_max = ((uint16_t)uart1_rx_data[0] << 8) | ((uint16_t)uart1_rx_data[1]);

                MAIN_TASK_LOG("UART1 RX data: %d \r\n", adc_result_max);
            }
        }

        HAL_Delay(10);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == SYNC1_Pin)
    {
        sync_1_counter++;
        get_sync_1 = 1;
    }
}
