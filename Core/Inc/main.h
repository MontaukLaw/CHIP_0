/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TEST_Pin GPIO_PIN_7
#define TEST_GPIO_Port GPIOB
#define LED_G_Pin GPIO_PIN_5
#define LED_G_GPIO_Port GPIOD
#define LED_R_Pin GPIO_PIN_3
#define LED_R_GPIO_Port GPIOD
#define LED_B_Pin GPIO_PIN_4
#define LED_B_GPIO_Port GPIOD
#define DAC_OUT_Pin GPIO_PIN_4
#define DAC_OUT_GPIO_Port GPIOA
#define SYNC4_Pin GPIO_PIN_10
#define SYNC4_GPIO_Port GPIOE
#define SYNC4_EXTI_IRQn EXTI15_10_IRQn
#define SYNC3_Pin GPIO_PIN_14
#define SYNC3_GPIO_Port GPIOE
#define SYNC3_EXTI_IRQn EXTI15_10_IRQn
#define SYNC2_Pin GPIO_PIN_15
#define SYNC2_GPIO_Port GPIOD
#define SYNC2_EXTI_IRQn EXTI15_10_IRQn
#define SYNC1_Pin GPIO_PIN_11
#define SYNC1_GPIO_Port GPIOD
#define SYNC1_EXTI_IRQn EXTI15_10_IRQn
#define OUT_EN_4_Pin GPIO_PIN_15
#define OUT_EN_4_GPIO_Port GPIOE
#define OUT_EN_7_Pin GPIO_PIN_14
#define OUT_EN_7_GPIO_Port GPIOD
#define OUT_EN_8_Pin GPIO_PIN_10
#define OUT_EN_8_GPIO_Port GPIOD
#define OUT_EN_2_Pin GPIO_PIN_8
#define OUT_EN_2_GPIO_Port GPIOE
#define OUT_EN_3_Pin GPIO_PIN_12
#define OUT_EN_3_GPIO_Port GPIOE
#define OUT_EN_5_Pin GPIO_PIN_9
#define OUT_EN_5_GPIO_Port GPIOD
#define OUT_EN_6_Pin GPIO_PIN_13
#define OUT_EN_6_GPIO_Port GPIOD
#define OUT_EN_1_Pin GPIO_PIN_7
#define OUT_EN_1_GPIO_Port GPIOA
#define OUT_S3_Pin GPIO_PIN_9
#define OUT_S3_GPIO_Port GPIOE
#define OUT_S2_Pin GPIO_PIN_13
#define OUT_S2_GPIO_Port GPIOE
#define OUT_S1_Pin GPIO_PIN_8
#define OUT_S1_GPIO_Port GPIOD
#define OUT_S0_Pin GPIO_PIN_12
#define OUT_S0_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
