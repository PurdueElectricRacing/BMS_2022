/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "psched.h"
#include "bms.h"
#include "afe.h"
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
#define PRSNLITY_Pin GPIO_PIN_0
#define PRSNLITY_GPIO_Port GPIOA
#define BOARD_M_Pin GPIO_PIN_1
#define BOARD_M_GPIO_Port GPIOA
#define I_M_OUT_Pin GPIO_PIN_4
#define I_M_OUT_GPIO_Port GPIOA
#define MOSI_Pin GPIO_PIN_7
#define MOSI_GPIO_Port GPIOA
#define CONN_LED_Pin GPIO_PIN_0
#define CONN_LED_GPIO_Port GPIOB
#define HEARTBEAT_Pin GPIO_PIN_1
#define HEARTBEAT_GPIO_Port GPIOB
#define WC_Pin GPIO_PIN_8
#define WC_GPIO_Port GPIOA
#define ERR_LED_Pin GPIO_PIN_15
#define ERR_LED_GPIO_Port GPIOA
#define SCK_Pin GPIO_PIN_3
#define SCK_GPIO_Port GPIOB
#define MISO_Pin GPIO_PIN_4
#define MISO_GPIO_Port GPIOB
#define CSB_AFE_Pin GPIO_PIN_5
#define CSB_AFE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
extern SPI_HandleTypeDef hspi1;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
