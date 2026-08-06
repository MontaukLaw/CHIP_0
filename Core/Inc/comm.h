#ifndef _COMM_H
#define _COMM_H

#include "stm32h7xx_hal.h"

HAL_StatusTypeDef comm_init(void);
void comm_process(void);
HAL_StatusTypeDef comm_uart1_prepare_rx(void);
HAL_StatusTypeDef comm_uart1_wait_2bytes(uint8_t data[2],
                                         uint32_t timeout_ms);

#endif // _COMM_H
