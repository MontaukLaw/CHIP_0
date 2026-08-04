#include "user_comm.h"

void test_led(void) {

    static uint32_t last_time = 0;

    uint32_t current_time = HAL_GetTick();
    if (current_time - last_time < 1000)
        return;

    HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);

    last_time = current_time;
}

void sync_test(void) {
    
}