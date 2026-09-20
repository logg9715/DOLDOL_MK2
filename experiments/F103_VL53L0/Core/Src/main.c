/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "vl53l0x_api.h"
#include <stdio.h>

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
I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */
extern void initialise_monitor_handles(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
VL53L0X_Dev_t MyDevice;
VL53L0X_DEV my_vl53 = &MyDevice;
VL53L0X_Error vl53_err;


void read_vl53(VL53L0X_DEV dev)
{
	VL53L0X_Error err;
	VL53L0X_RangingMeasurementData_t res = {0};

	err = VL53L0X_PerformSingleRangingMeasurement(dev, &res);
//	printf("Ranging measurement: %d\r\n", err);
	if(res.RangeStatus == 0 && err == 0)
	{
		printf("Dist %d mm (stat:%d)\r\n", res.RangeMilliMeter, res.RangeStatus);
	}
	else
	{
		printf("Dist[Err] %d mm (stat:%d)\r\n", res.RangeMilliMeter, res.RangeStatus);
	}

	/*
	 * NOTE 09.20 자꾸 측정오류 2가 나옴
	 * RangeMillimeter(밀리미터연산값)가 자꾸 200 배수로만 나옴
	 *
	 * 257 514 771 좌우 바이트 같음
	 * 257 00000001 00000001
	 * 514 00000010 00000010
	 * 771 00000011 00000011
	 *
	 *single 1E:02 1F:6C => 620
	 *Dist 514 mm (stat:0)
	 *>>>>> st:0 : 5B 06 AE 04 02 89 00 C8 00 1A 02 02
	 *
	 *single 1E:03 1F:4B => 843
	 *Dist 771 mm (stat:0)
	 *>>>>> st:0 : 5B 06 AE 04 02 3B 00 AF 00 1A 03 03
	 *
	 *hi값이 12번째 비트에 복사됨, 길이 16정도로 늘려보니 마지막이 비긴 해도 11 12번째는 잘 들어오는듯
	 *
	 */

	//	printf("raw sig:%lu spad:%lu\r\n", res.SignalRateRtnMegaCps, res.EffectiveSpadRtnCount);
//    uint8_t hi = 0, lo = 0;
//    uint8_t blk[16] = {0};
//    int blk_len = sizeof(blk)/sizeof(blk[0]);
//    VL53L0X_RdByte(dev, 0x1E, &hi); // 10 11번 인덱스 읽어서 뭐 하는듯 함
//    VL53L0X_RdByte(dev, 0x1F, &lo);
//
//    printf("single 1E:%02X 1F:%02X => %d \r\n", hi, lo, (hi<<8)|lo);
//
//    VL53L0X_ReadMulti(dev, 0x14, blk, blk_len); // 시작위치부터 다읽어보기
//    printf("blk:");
//    for (int i = 0; i < blk_len; i++)
//    	printf(" %02X", blk[i]);
//
//    printf("\r\n-----------------\r\n");

// =========== DEBUG END =========================

//	VL53L0X_StopMeasurement(dev);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	initialise_monitor_handles();
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

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
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  // 적외선 준비
  // xshut on
  HAL_GPIO_WritePin(B12_GPIO_Port, B12_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(B12_GPIO_Port, B12_Pin, GPIO_PIN_SET);
  HAL_Delay(10);

  // Device Init
  my_vl53->I2cDevAddr = 0x52;
  vl53_err = VL53L0X_DataInit(my_vl53);
  printf("DataInit: %d\r\n", vl53_err);
  vl53_err = VL53L0X_StaticInit(my_vl53);
  printf("StaticInit: %d\r\n", vl53_err);
  // set Dev Mode
//  VL53L0X_SetDeviceMode(my_vl53, VL53L0X_DEVICEMODE_SINGLE_RANGING);


  // -----------------------
  // 09.20 해결부분
  uint32_t refSpadCount; //3
  uint8_t isApertureSpads, VhvSettings, PhaseCal; //0 29 1

  vl53_err = VL53L0X_PerformRefSpadManagement(my_vl53, &refSpadCount, &isApertureSpads);
  printf("SpadMgmt: %d (count:%lu ap:%d)\r\n", vl53_err, refSpadCount, isApertureSpads);

  vl53_err = VL53L0X_PerformRefCalibration(my_vl53, &VhvSettings, &PhaseCal);
  printf("RefCal: %d (vhv:%d phase:%d)\r\n", vl53_err, VhvSettings, PhaseCal);

//  VL53L0X_SetReferenceSpads(my_vl53, 3, 0);
//  VL53L0X_SetRefCalibration(my_vl53, 29, 1);
  // ----------------------
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  read_vl53(my_vl53);

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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(B12_GPIO_Port, B12_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B12_Pin */
  GPIO_InitStruct.Pin = B12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(B12_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


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
#ifdef USE_FULL_ASSERT
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
