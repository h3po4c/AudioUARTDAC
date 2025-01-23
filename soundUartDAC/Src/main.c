/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "user_functions.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t buffer[BUFFER_SIZE];
uint8_t buffer[BUFFER_SIZE]; // audio buffer
SystemState currentState = IDLE;
uint8_t startFlag = 0;
uint8_t startAudioFlag = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* System interrupt init*/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_DAC_Init();
  MX_USART3_UART_Init();
  MX_TIM8_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    switch (currentState) // State machine
    {
    case IDLE:
      /* code */
      break;
    case PLAYING:
      Playback();
      break;
    case STOPPED:
      Stop();
      currentState = IDLE;
      break;  
    default:
      break;
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_3)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
  LL_PWR_DisableOverDriveMode();
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 120, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_Init1msTick(120000000);
  LL_SetSystemCoreClock(120000000);
  LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}

/* USER CODE BEGIN 4 */

void StartUSARTAudio_DMA(void) {
    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_2);
    LL_USART_EnableDMAReq_RX(USART1);
    while(!LL_USART_IsActiveFlag_TC(USART1));
    LL_USART_TransmitData8(USART1, REQUEST_NEW_DATA);
}

void StopUSARTAudio_DMA(void) {
    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_2);
    LL_USART_DisableDMAReq_RX(USART1);
}

void StartDACAudio_DMA()
{
  LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_5);
  LL_DAC_EnableDMAReq(DAC, LL_DAC_CHANNEL_1);
  LL_TIM_EnableCounter(TIM8);
  LL_DAC_Enable(DAC1, LL_DAC_CHANNEL_1);
}

void StopDACAudio_DMA()
{
  LL_DAC_DisableDMAReq(DAC, LL_DAC_CHANNEL_1);
  LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_5);
  LL_TIM_DisableCounter(TIM8);
  LL_DAC_Disable(DAC1, LL_DAC_CHANNEL_1);
}

void Playback()
{
  if(startFlag == 0)
  {
    startFlag = 1;
    StartUSARTAudio_DMA();
  }
}

void Pause()
{
  
}

void Stop()
{
  startFlag = 0;
  startAudioFlag = 0;
  StopUSARTAudio_DMA();
  StopDACAudio_DMA();
}

void ProcessCommand(uint8_t command)
{
  switch (command)
  {
  case CMD_PING:
    SendString("pong");
    break;
  case CMD_PLAY:
    currentState = PLAYING;
    SendString("playing");
    break;
  case CMD_PAUSE:
    currentState = PAUSED;
    break;
  case CMD_STOP:
    currentState = STOPPED;
    SendString("stopped");
    break;
  case CMD_UNKNOWN:
  default:
    break;
  }
}

void SendString(const char* str) {
    while (*str) {
        while (!LL_USART_IsActiveFlag_TXE(USART3));
        LL_USART_TransmitData8(USART3, *str++);   
    }
    while (!LL_USART_IsActiveFlag_TXE(USART3));
    LL_USART_TransmitData8(USART3, '\n'); 
    while (!LL_USART_IsActiveFlag_TXE(USART3));
    LL_USART_TransmitData8(USART3, '\r'); 
     
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
