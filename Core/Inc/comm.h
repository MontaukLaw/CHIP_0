#ifndef _COMM_H
#define _COMM_H

#include "stm32h7xx_hal.h"

HAL_StatusTypeDef comm_init(void);
void comm_process(void);

#endif // _COMM_H
