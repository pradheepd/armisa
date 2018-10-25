
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_hal.h"
#include "usb_device.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
DMA_HandleTypeDef hdma_memtomem_dma1_channel1;
DMA_HandleTypeDef hdma_memtomem_dma1_channel2;
DMA_HandleTypeDef hdma_memtomem_dma1_channel3;
DMA_HandleTypeDef hdma_memtomem_dma1_channel4;
DMA_HandleTypeDef hdma_memtomem_dma1_channel5;
DMA_HandleTypeDef hdma_memtomem_dma1_channel6;
DMA_HandleTypeDef hdma_memtomem_dma1_channel7;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  uint8_t HiMsg[100] = "";

  uint32_t volatile status_reg = 0;

  while (1)
  {

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	  status_reg = __get_APSR();
	  sprintf(HiMsg,"APSR : %x\r\n",status_reg);
	  CDC_Transmit_FS(HiMsg, strlen(HiMsg));
	  HAL_Delay(100);
  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/** 
  * Enable DMA controller clock
  * Configure DMA for memory to memory transfers
  *   hdma_memtomem_dma1_channel1
  *   hdma_memtomem_dma1_channel2
  *   hdma_memtomem_dma1_channel3
  *   hdma_memtomem_dma1_channel4
  *   hdma_memtomem_dma1_channel5
  *   hdma_memtomem_dma1_channel6
  *   hdma_memtomem_dma1_channel7
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* Configure DMA request hdma_memtomem_dma1_channel1 on DMA1_Channel1 */
  hdma_memtomem_dma1_channel1.Instance = DMA1_Channel1;
  hdma_memtomem_dma1_channel1.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel1.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel1.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel1.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel1.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /* Configure DMA request hdma_memtomem_dma1_channel2 on DMA1_Channel2 */
  hdma_memtomem_dma1_channel2.Instance = DMA1_Channel2;
  hdma_memtomem_dma1_channel2.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel2.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel2.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel2.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel2.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel2.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /* Configure DMA request hdma_memtomem_dma1_channel3 on DMA1_Channel3 */
  hdma_memtomem_dma1_channel3.Instance = DMA1_Channel3;
  hdma_memtomem_dma1_channel3.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel3.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel3.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel3.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel3.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel3.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel3.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /* Configure DMA request hdma_memtomem_dma1_channel4 on DMA1_Channel4 */
  hdma_memtomem_dma1_channel4.Instance = DMA1_Channel4;
  hdma_memtomem_dma1_channel4.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel4.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel4.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel4.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel4.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel4.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel4.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /* Configure DMA request hdma_memtomem_dma1_channel5 on DMA1_Channel5 */
  hdma_memtomem_dma1_channel5.Instance = DMA1_Channel5;
  hdma_memtomem_dma1_channel5.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel5.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel5.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel5.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_memtomem_dma1_channel5.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_memtomem_dma1_channel5.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel5.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /* Configure DMA request hdma_memtomem_dma1_channel6 on DMA1_Channel6 */
  hdma_memtomem_dma1_channel6.Instance = DMA1_Channel6;
  hdma_memtomem_dma1_channel6.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel6.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel6.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel6.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_memtomem_dma1_channel6.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
  hdma_memtomem_dma1_channel6.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel6.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel6) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /* Configure DMA request hdma_memtomem_dma1_channel7 on DMA1_Channel7 */
  hdma_memtomem_dma1_channel7.Instance = DMA1_Channel7;
  hdma_memtomem_dma1_channel7.Init.Direction = DMA_MEMORY_TO_MEMORY;
  hdma_memtomem_dma1_channel7.Init.PeriphInc = DMA_PINC_ENABLE;
  hdma_memtomem_dma1_channel7.Init.MemInc = DMA_MINC_ENABLE;
  hdma_memtomem_dma1_channel7.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_memtomem_dma1_channel7.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  hdma_memtomem_dma1_channel7.Init.Mode = DMA_NORMAL;
  hdma_memtomem_dma1_channel7.Init.Priority = DMA_PRIORITY_LOW;
  if (HAL_DMA_Init(&hdma_memtomem_dma1_channel7) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
