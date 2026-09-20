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

volatile uint8_t  vl53_ready[2] = {0, 0};
volatile uint32_t vl53_irq_cnt[2] = {0, 0};

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
VL53L0X_Dev_t MyDevice2;
VL53L0X_DEV my_vl53_2 = &MyDevice2;
VL53L0X_Error vl53_err;


void ready_vl53_continue(VL53L0X_DEV dev)
{
	VL53L0X_Error err;
	err = VL53L0X_SetDeviceMode(dev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
	printf("VL53L0X_SetDeviceMode : %d\r\n", err);
	VL53L0X_StartMeasurement(dev);
}


void ready_vl53_newMasure(VL53L0X_DEV dev)
{
	VL53L0X_Error err;
	err = VL53L0X_SetDeviceMode(dev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);
	printf("VL53L0X_SetDeviceMode : %d\r\n", err);
	VL53L0X_StartMeasurement(dev);
}

void read_vl53_newMasure(VL53L0X_DEV dev, int idx)
{
	VL53L0X_Error err;
	VL53L0X_RangingMeasurementData_t res = {0};

	while(vl53_irq_cnt[idx] > 0)
	{
		err = VL53L0X_GetRangingMeasurementData(dev, &res);
		if(!err)
			VL53L0X_ClearInterruptMask(dev, 0);

		if(res.RangeStatus == 0 && err == 0)
		{
			printf("[%#x]Dist %d mm (stat:%d)\r\n", dev->I2cDevAddr, res.RangeMilliMeter, res.RangeStatus);
		}
		else
		{
			printf("[%#x] Dist(Err) %d mm (stat:%d)\r\n", dev->I2cDevAddr, res.RangeMilliMeter, res.RangeStatus);
		}

		vl53_irq_cnt[idx]--;
	}
	vl53_ready[idx] = 0;
}

void read_vl53(VL53L0X_DEV dev)
{
	VL53L0X_Error err;
	VL53L0X_RangingMeasurementData_t res = {0};

	err = VL53L0X_PerformSingleRangingMeasurement(dev, &res);
//	printf("Ranging measurement: %d\r\n", err);
	if(res.RangeStatus == 0 && err == 0)
	{
		printf("[%#x]Dist %d mm (stat:%d)\r\n", dev->I2cDevAddr, res.RangeMilliMeter, res.RangeStatus);
	}
	else
	{
		printf("[%#x] Dist(Err) %d mm (stat:%d)\r\n", dev->I2cDevAddr, res.RangeMilliMeter, res.RangeStatus);
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

void xshut_set(GPIO_TypeDef* port, uint16_t pin)
{
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
	HAL_Delay(10);
	HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
	HAL_Delay(10);
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
  // #1 적외선 준비 0x60
  // xshut on
  xshut_set(B12_GPIO_Port, B12_Pin);

  // Device Init
  my_vl53->I2cDevAddr = 0x52;
  vl53_err = VL53L0X_DataInit(my_vl53);
  printf("DataInit: %d\r\n", vl53_err);
  vl53_err = VL53L0X_StaticInit(my_vl53);
  printf("StaticInit: %d\r\n", vl53_err);

  vl53_err = VL53L0X_SetDeviceAddress(my_vl53, 0x60);
  printf("SetDeviceAddress: %d\r\n", vl53_err);
  my_vl53->I2cDevAddr = 0x60;

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

  VL53L0X_SetGpioConfig(my_vl53, 0, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
		  VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, VL53L0X_INTERRUPTPOLARITY_LOW);



//  VL53L0X_SetReferenceSpads(my_vl53, 3, 0);
//  VL53L0X_SetRefCalibration(my_vl53, 29, 1);
  // ----------------------------------------------------------------------------------------
  // #2 적외선 0x62
  xshut_set(B13_GPIO_Port, B13_Pin);

  // Device Init
  my_vl53_2->I2cDevAddr = 0x52;
  vl53_err = VL53L0X_DataInit(my_vl53_2);
  printf("DataInit: %d\r\n", vl53_err);
  vl53_err = VL53L0X_StaticInit(my_vl53_2);
  printf("StaticInit: %d\r\n", vl53_err);

  vl53_err = VL53L0X_SetDeviceAddress(my_vl53_2, 0x62);
  printf("SetDeviceAddress: %d\r\n", vl53_err);
  my_vl53_2->I2cDevAddr = 0x62;

  vl53_err = VL53L0X_PerformRefSpadManagement(my_vl53_2, &refSpadCount, &isApertureSpads);
  printf("SpadMgmt: %d (count:%lu ap:%d)\r\n", vl53_err, refSpadCount, isApertureSpads);

  vl53_err = VL53L0X_PerformRefCalibration(my_vl53_2, &VhvSettings, &PhaseCal);
  printf("RefCal: %d (vhv:%d phase:%d)\r\n", vl53_err, VhvSettings, PhaseCal);

  VL53L0X_SetGpioConfig(my_vl53_2, 0, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
		  VL53L0X_GPIOFUNCTIONALITY_NEW_MEASURE_READY, VL53L0X_INTERRUPTPOLARITY_LOW);
//  VL53L0X_SetInterruptThresholds(my_vl53_2, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,  100 << 16, 0);


  // =============================================
  ready_vl53_newMasure(my_vl53);
  ready_vl53_newMasure(my_vl53_2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if(vl53_ready[0])
		  read_vl53_newMasure(my_vl53, 0);

	  if(vl53_ready[1])
		  read_vl53_newMasure(my_vl53_2, 1);
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, B12_Pin|B13_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : GPIO_EX_Pin GPIO_EX4_Pin */
  GPIO_InitStruct.Pin = GPIO_EX_Pin|GPIO_EX4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : B12_Pin B13_Pin */
  GPIO_InitStruct.Pin = B12_Pin|B13_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_pin)
{
    if (GPIO_pin == GPIO_EX_Pin) {
        vl53_ready[0] = 1;
        vl53_irq_cnt[0]++;
    }
    else if (GPIO_pin == GPIO_EX4_Pin) {
        vl53_ready[1] = 1;
        vl53_irq_cnt[1]++;
    }
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
