#include "main.h"
#include "user_comm.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

void test_led(void) {

    static uint32_t last_time = 0;

    uint32_t current_time = HAL_GetTick();
    if (current_time - last_time < 1000)
        return;

    HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);

    last_time = current_time;
}

void sync_test(void) {

    static uint8_t init = 0;
    if (!init) {
        init = 1;
        // 执行初始化操作
        HAL_GPIO_WritePin(OUT_S0_GPIO_Port, OUT_S0_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT_S1_GPIO_Port, OUT_S1_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT_S2_GPIO_Port, OUT_S2_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(OUT_S3_GPIO_Port, OUT_S3_Pin, GPIO_PIN_RESET);
    }

    HAL_GPIO_WritePin(SYNC_1_GPIO_Port, SYNC_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OUT_EN_1_GPIO_Port, OUT_EN_1_Pin, GPIO_PIN_RESET);
    delay_us(5);
    HAL_GPIO_WritePin(OUT_EN_1_GPIO_Port, OUT_EN_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SYNC_1_GPIO_Port, SYNC_1_Pin, GPIO_PIN_RESET);

    HAL_Delay(1);
}

void cdc_send_test(void) {
    static uint32_t last_time = 0U;
    static uint8_t message[] = "STM32H723 USB CDC test\r\n";
    uint32_t current_time = HAL_GetTick();

    if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED) {
        return;
    }

    if ((uint32_t)(current_time - last_time) < 1000U) {
        return;
    }

    if (CDC_Transmit_HS(message, (uint16_t)(sizeof(message) - 1U)) == USBD_OK) {
        last_time = current_time;
    }
}
