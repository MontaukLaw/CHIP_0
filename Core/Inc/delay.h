#ifndef DELAY_H
#define DELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void delay_init(void);
void delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* DELAY_H */
