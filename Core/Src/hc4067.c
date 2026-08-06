#include "user_comm.h"

void all_wave_ch_disable(void)
{

    // 把所有的hc4067都disable掉
    HAL_GPIO_WritePin(OUT_EN_1_GPIO_Port, OUT_EN_1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OUT_EN_2_GPIO_Port, OUT_EN_2_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(OUT_EN_3_GPIO_Port, OUT_EN_3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OUT_EN_4_GPIO_Port, OUT_EN_4_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(OUT_EN_5_GPIO_Port, OUT_EN_5_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OUT_EN_6_GPIO_Port, OUT_EN_6_Pin, GPIO_PIN_SET);

    HAL_GPIO_WritePin(OUT_EN_7_GPIO_Port, OUT_EN_7_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(OUT_EN_8_GPIO_Port, OUT_EN_8_Pin, GPIO_PIN_SET);
}


// 打开128个通道中的一个
void wave_ch_on(uint8_t wave_ch)
{

    // 先全关
    all_wave_ch_disable();

    // 先做选择
    uint8_t wave_ch_idx = wave_ch & 0x0f; // 只取低4位

    HAL_GPIO_WritePin(OUT_S0_GPIO_Port, OUT_S0_Pin,
                      (wave_ch_idx & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OUT_S1_GPIO_Port, OUT_S1_Pin,
                      (wave_ch_idx & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OUT_S2_GPIO_Port, OUT_S2_Pin,
                      (wave_ch_idx & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(OUT_S3_GPIO_Port, OUT_S3_Pin,
                      (wave_ch_idx & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // 再打开指定的通道
    uint8_t en_pin = wave_ch >> 4; // 取高4位作为使能引脚
    switch (en_pin)
    {
    case 0:
        HAL_GPIO_WritePin(OUT_EN_1_GPIO_Port, OUT_EN_1_Pin, GPIO_PIN_RESET);
        break;
    case 1:
        HAL_GPIO_WritePin(OUT_EN_2_GPIO_Port, OUT_EN_2_Pin, GPIO_PIN_RESET);
        break;
    case 2:
        HAL_GPIO_WritePin(OUT_EN_3_GPIO_Port, OUT_EN_3_Pin, GPIO_PIN_RESET);
        break;
    case 3:
        HAL_GPIO_WritePin(OUT_EN_4_GPIO_Port, OUT_EN_4_Pin, GPIO_PIN_RESET);
        break;
    case 4:
        HAL_GPIO_WritePin(OUT_EN_5_GPIO_Port, OUT_EN_5_Pin, GPIO_PIN_RESET);
        break;
    case 5:
        HAL_GPIO_WritePin(OUT_EN_6_GPIO_Port, OUT_EN_6_Pin, GPIO_PIN_RESET);
        break;
    case 6:
        HAL_GPIO_WritePin(OUT_EN_7_GPIO_Port, OUT_EN_7_Pin, GPIO_PIN_RESET);
        break;
    case 7:
        HAL_GPIO_WritePin(OUT_EN_8_GPIO_Port, OUT_EN_8_Pin, GPIO_PIN_RESET);
        break;
    }
}
