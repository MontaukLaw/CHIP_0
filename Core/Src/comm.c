#include "user_comm.h"

#include "usb_device.h"
#include "usbd_cdc_if.h"

#define UART1_RX_BUFFER_SIZE 2048U
#define UART1_PREVIEW_SIZE 10U
#define UART1_EVENT_QUEUE_SIZE 4U
#if defined(__CC_ARM)
#define UART1_DMA_BUFFER_ATTRIBUTE                                             \
    __attribute__((section(".dma_buffer"), aligned(32), zero_init))
#else
#define UART1_DMA_BUFFER_ATTRIBUTE                                             \
    __attribute__((section(".dma_buffer"), aligned(32)))
#endif

typedef struct {
    uint16_t size;
    uint8_t preview[UART1_PREVIEW_SIZE];
} Uart1RxEvent;

UART1_DMA_BUFFER_ATTRIBUTE static uint8_t uart1_rx_buffer[UART1_RX_BUFFER_SIZE];
static Uart1RxEvent uart1_event_queue[UART1_EVENT_QUEUE_SIZE];
static volatile uint8_t uart1_event_write;
static volatile uint8_t uart1_event_read;
static volatile uint8_t uart1_rx_started;
static volatile uint8_t uart1_restart_pending;

static const char hex_digits[] = "0123456789ABCDEF";

extern DMA_HandleTypeDef hdma_usart1_rx;

static HAL_StatusTypeDef comm_uart1_start_receive(void) {
    HAL_StatusTypeDef status;

    if ((huart1.hdmarx != &hdma_usart1_rx) ||
        (hdma_usart1_rx.Instance != DMA1_Stream6)) {
        return HAL_ERROR;
    }

    status = HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buffer,
                                          sizeof(uart1_rx_buffer));

    return status;
}

static uint16_t comm_append_text(uint8_t *buffer, uint16_t position,
                                 const char *text) {
    while (*text != '\0') {
        buffer[position++] = (uint8_t)*text++;
    }

    return position;
}

static uint16_t comm_append_uint16(uint8_t *buffer, uint16_t position,
                                   uint16_t value) {
    uint8_t digits[5];
    uint8_t count = 0U;

    do {
        digits[count++] = (uint8_t)('0' + (value % 10U));
        value /= 10U;
    } while (value != 0U);

    while (count != 0U) {
        buffer[position++] = digits[--count];
    }

    return position;
}

HAL_StatusTypeDef comm_init(void) {
    uart1_event_write = 0U;
    uart1_event_read = 0U;
    uart1_rx_started = 0U;
    uart1_restart_pending = 0U;

    if (hdma_usart1_rx.Instance != DMA1_Stream6) {
        return HAL_ERROR;
    }

    __HAL_LINKDMA(&huart1, hdmarx, hdma_usart1_rx);

    /* Keep USB enumeration interrupts above the high-rate UART path. */
    HAL_NVIC_SetPriority(USART1_IRQn, 2U, 0U);
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 2U, 0U);

    return HAL_OK;
}

void comm_process(void) {
    static uint8_t cdc_report[96];
    Uart1RxEvent *event;
    uint16_t preview_size;
    uint16_t position = 0U;
    uint16_t i;

    if (uart1_restart_pending != 0U) {
        (void)HAL_UART_AbortReceive(&huart1);

        if (comm_uart1_start_receive() == HAL_OK) {
            uart1_restart_pending = 0U;
            uart1_rx_started = 1U;
        }

        return;
    }

    /* Do not let UART traffic interfere with initial USB enumeration. */
    if (uart1_rx_started == 0U) {
        if ((hUsbDeviceHS.dev_state == USBD_STATE_CONFIGURED) &&
            (comm_uart1_start_receive() == HAL_OK)) {
            uart1_rx_started = 1U;
        }

        return;
    }

    if (uart1_event_read == uart1_event_write) {
        return;
    }

    if (hUsbDeviceHS.dev_state != USBD_STATE_CONFIGURED) {
        return;
    }

    event = &uart1_event_queue[uart1_event_read];
    preview_size =
        (event->size < UART1_PREVIEW_SIZE) ? event->size : UART1_PREVIEW_SIZE;

    position = comm_append_text(cdc_report, position, "UART1 RX: ");
    position = comm_append_uint16(cdc_report, position, event->size);
    position = comm_append_text(cdc_report, position, " bytes, first ");
    position = comm_append_uint16(cdc_report, position, preview_size);
    position = comm_append_text(cdc_report, position, ":");

    for (i = 0U; i < preview_size; ++i) {
        uint8_t value = event->preview[i];

        cdc_report[position++] = ' ';
        cdc_report[position++] = (uint8_t)hex_digits[value >> 4];
        cdc_report[position++] = (uint8_t)hex_digits[value & 0x0FU];
    }

    cdc_report[position++] = '\r';
    cdc_report[position++] = '\n';

    if (CDC_Transmit_HS(cdc_report, position) == USBD_OK) {
        uart1_event_read =
            (uint8_t)((uart1_event_read + 1U) % UART1_EVENT_QUEUE_SIZE);
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
    HAL_UART_RxEventTypeTypeDef event_type;
    uint8_t next_write;
    uint16_t preview_size;
    uint16_t i;

    if (huart->Instance != USART1) {
        return;
    }

    event_type = HAL_UARTEx_GetRxEventType(huart);
    if (event_type == HAL_UART_RXEVENT_HT) {
        return;
    }

    next_write = (uint8_t)((uart1_event_write + 1U) % UART1_EVENT_QUEUE_SIZE);

    if (next_write != uart1_event_read) {
        Uart1RxEvent *event = &uart1_event_queue[uart1_event_write];

        event->size = size;
        preview_size = (size < UART1_PREVIEW_SIZE) ? size : UART1_PREVIEW_SIZE;

        for (i = 0U; i < preview_size; ++i) {
            event->preview[i] = uart1_rx_buffer[i];
        }

        uart1_event_write = next_write;
    }

    if (comm_uart1_start_receive() != HAL_OK) {
        uart1_rx_started = 0U;
        uart1_restart_pending = 1U;
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        uart1_rx_started = 0U;
        uart1_restart_pending = 1U;
    }
}
