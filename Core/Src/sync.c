#include "user_comm.h"

// 每个adc芯片包含32个通道
void send_sync_signal(uint8_t adc_idx, GPIO_PinState pin_state)
{
    // 计算当前adc索引对应的子芯片索引
    uint8_t sub_chip_idx = adc_idx / ADC_CH_PER_CHIP;

    switch (sub_chip_idx)
    {
    // case 0:
    //     // 发送同步信号到子芯片0
    //     HAL_GPIO_WritePin(SYNC_1_GPIO_Port, SYNC_1_Pin, pin_state);
    //     break;
    // case 1:
    //     // 发送同步信号到子芯片1
    //     HAL_GPIO_WritePin(SYNC_2_GPIO_Port, SYNC_2_Pin, pin_state);
    //     break;
    // case 2:
    //     // 发送同步信号到子芯片2
    //     HAL_GPIO_WritePin(SYNC_3_GPIO_Port, SYNC_3_Pin, pin_state);
    //     break;
    // case 3:
    //     // 发送同步信号到子芯片3
    //     HAL_GPIO_WritePin(SYNC_4_GPIO_Port, SYNC_4_Pin, pin_state);
    //     break;
    // default:
    //     break
    }
}
