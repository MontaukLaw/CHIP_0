#include "cdc_stdio.h"

#include <stdio.h>
#include <stdint.h>

#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

#define CDC_STDIO_QUEUE_SIZE 1024U
#define CDC_STDIO_PACKET_SIZE 256U

static uint8_t cdc_stdio_queue[CDC_STDIO_QUEUE_SIZE];
static uint8_t cdc_stdio_tx_buffer[CDC_STDIO_PACKET_SIZE];
static volatile uint16_t cdc_stdio_write;
static volatile uint16_t cdc_stdio_read;

#if defined(__CC_ARM)
#pragma import(__use_no_semihosting)

struct __FILE {
    int handle;
};

FILE __stdout;

void _sys_exit(int status) {
    (void)status;

    while (1) {
    }
}

void _ttywrch(int character) {
    (void)character;
}
#endif

int fputc(int character, FILE *stream) {
    uint16_t next_write;

    (void)stream;

    next_write = (uint16_t)((cdc_stdio_write + 1U) % CDC_STDIO_QUEUE_SIZE);
    if (next_write == cdc_stdio_read) {
        return EOF;
    }

    cdc_stdio_queue[cdc_stdio_write] = (uint8_t)character;
    cdc_stdio_write = next_write;

    return character;
}

void cdc_stdio_process(void) {
    USBD_CDC_HandleTypeDef *cdc;
    uint16_t queued_size;
    uint16_t send_size;
    uint16_t read_position;
    uint16_t i;

    if ((hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED) ||
        (hUsbDeviceHS.pClassData == NULL)) {
        return;
    }

    cdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceHS.pClassData;
    if ((cdc->TxState != 0U) || (cdc_stdio_read == cdc_stdio_write)) {
        return;
    }

    if (cdc_stdio_write >= cdc_stdio_read) {
        queued_size = cdc_stdio_write - cdc_stdio_read;
    } else {
        queued_size = CDC_STDIO_QUEUE_SIZE - cdc_stdio_read;
    }

    send_size = (queued_size < CDC_STDIO_PACKET_SIZE)
                    ? queued_size
                    : CDC_STDIO_PACKET_SIZE;
    read_position = cdc_stdio_read;

    for (i = 0U; i < send_size; ++i) {
        cdc_stdio_tx_buffer[i] = cdc_stdio_queue[read_position++];
    }

    if (CDC_Transmit_HS(cdc_stdio_tx_buffer, send_size) == USBD_OK) {
        cdc_stdio_read =
            (uint16_t)((cdc_stdio_read + send_size) % CDC_STDIO_QUEUE_SIZE);
    }
}
