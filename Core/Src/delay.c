#include "delay.h"

#include "stm32h7xx.h"

void delay_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us) {
    uint32_t cycles_per_us;
    uint32_t max_us;

    if (us == 0U) {
        return;
    }

    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U) {
        delay_init();
    }

    cycles_per_us = SystemCoreClock / 1000000U;
    if (cycles_per_us == 0U) {
        cycles_per_us = 1U;
    }

    max_us = UINT32_MAX / cycles_per_us;
    while (us != 0U) {
        uint32_t chunk_us = (us > max_us) ? max_us : us;
        uint32_t wait_cycles = chunk_us * cycles_per_us;
        uint32_t start = DWT->CYCCNT;

        while ((uint32_t)(DWT->CYCCNT - start) < wait_cycles) {
            __NOP();
        }

        us -= chunk_us;
    }
}
