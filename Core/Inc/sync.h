#ifndef _SYNC_H_
#define _SYNC_H_

#include <stdint.h>

void send_sync_signal(uint8_t adc_idx, GPIO_PinState pin_state);

#endif /* _SYNC_H_ */
