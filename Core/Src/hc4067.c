#include "user_comm.h"

void hc4067_all_init(void) {

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