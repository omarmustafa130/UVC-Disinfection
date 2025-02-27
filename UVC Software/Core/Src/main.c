/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "LSM6DS33.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// Global or static for orientation
typedef enum {
    TOP_SIDE_UP,
    BOTTOM_SIDE_UP
} OrientationType;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Define GPIO Ports & Pins
#define TOP_UVC_LED_Pin       		GPIO_PIN_0
#define TOP_UVC_LED_GPIO_Port 		GPIOA

#define BOTTOM_UVC_LED_Pin       	GPIO_PIN_1
#define BOTTOM_UVC_LED_GPIO_Port 	GPIOA

#define TOP_INDICATOR_LED_Pin       GPIO_PIN_3
#define BOTTOM_INDICATOR_LED_Pin    GPIO_PIN_7

#define INDICATOR_LED_GPIO_Port 	GPIOA
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;


/* USER CODE BEGIN PV */
int indicatorBlinking = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE BEGIN PFP */
OrientationType get_orientation(void);
void start_indicator_blink(void);
void stop_indicator_blink(void);
void enter_low_power_mode(void);
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
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  // Initialize the sensor
  LSM6DS33_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	// --- PHASE 1: Wait 30 seconds ---
	HAL_Delay(25000);

	// Start blinking indicator LED for 5 seconds before UVC turns ON
	start_indicator_blink();

	// --- PHASE 2: Determine orientation ---
	OrientationType orientation = get_orientation();

	// --- PHASE 3: Turn ON the correct UVC LED ---
	if (orientation == TOP_SIDE_UP)
	{
		HAL_GPIO_WritePin(TOP_UVC_LED_GPIO_Port, TOP_UVC_LED_Pin, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(BOTTOM_UVC_LED_GPIO_Port, BOTTOM_UVC_LED_Pin, GPIO_PIN_SET);
	}
    HAL_GPIO_WritePin(INDICATOR_LED_GPIO_Port, TOP_INDICATOR_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(INDICATOR_LED_GPIO_Port, BOTTOM_INDICATOR_LED_Pin, GPIO_PIN_RESET);
	// --- PHASE 4: Keep Indicator LED blinking WHILE UVC is ON (90 seconds) ---
	if (orientation == TOP_SIDE_UP)
	{

		for (int i = 0; i < 180; i++)  // 90s / 500ms = 180 blinks
		{
			HAL_GPIO_TogglePin(INDICATOR_LED_GPIO_Port, TOP_INDICATOR_LED_Pin);
			HAL_Delay(500);  // 500ms blink interval
		}
	}
	else
	{
		for (int i = 0; i < 180; i++)  // 90s / 500ms = 180 blinks
		{
			HAL_GPIO_TogglePin(INDICATOR_LED_GPIO_Port, BOTTOM_INDICATOR_LED_Pin);
			HAL_Delay(500);  // 500ms blink interval
		}
	}
	// --- PHASE 5: Turn off both UVC LEDs ---
	HAL_GPIO_WritePin(TOP_UVC_LED_GPIO_Port, TOP_UVC_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BOTTOM_UVC_LED_GPIO_Port, BOTTOM_UVC_LED_Pin, GPIO_PIN_RESET);

	// --- PHASE 6: Stop Indicator LED ---
	stop_indicator_blink();

	// --- PHASE 7: Enter Low-Power Mode for 1 Hour ---
	enter_low_power_mode();
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000608;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}



/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
/* USER CODE BEGIN MX_GPIO_Init_1 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ✅ Enable GPIO Ports Clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* ✅ Configure GPIO pin for TOP UVC LED */
    GPIO_InitStruct.Pin = TOP_UVC_LED_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Push-Pull mode
    GPIO_InitStruct.Pull = GPIO_NOPULL;          // No pull-up/pull-down
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // Low speed to save power
    HAL_GPIO_Init(TOP_UVC_LED_GPIO_Port, &GPIO_InitStruct);

    /* ✅ Configure GPIO pin for BOTTOM UVC LED */
    GPIO_InitStruct.Pin = BOTTOM_UVC_LED_Pin;
    HAL_GPIO_Init(BOTTOM_UVC_LED_GPIO_Port, &GPIO_InitStruct);

    /* ✅ Configure GPIO pin for INDICATOR LED */
    GPIO_InitStruct.Pin = TOP_INDICATOR_LED_Pin;
    HAL_GPIO_Init(INDICATOR_LED_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = BOTTOM_INDICATOR_LED_Pin;
    HAL_GPIO_Init(INDICATOR_LED_GPIO_Port, &GPIO_InitStruct);

    /* ✅ Ensure all LEDs are OFF at startup */
    HAL_GPIO_WritePin(TOP_UVC_LED_GPIO_Port, TOP_UVC_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BOTTOM_UVC_LED_GPIO_Port, BOTTOM_UVC_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(INDICATOR_LED_GPIO_Port, TOP_INDICATOR_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(INDICATOR_LED_GPIO_Port, BOTTOM_INDICATOR_LED_Pin, GPIO_PIN_RESET);

/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
OrientationType get_orientation(void)
{
    float ax = read_data_acc(1);  // ✅ Correct
    float ay = read_data_acc(2);
    float az = read_data_acc(3);

    float absX = fabs(ax);
    float absY = fabs(ay);
    float absZ = fabs(az);

    // Determine which side is up
    if (absZ >= absX && absZ >= absY)
    {
        if (az > 0.5) return TOP_SIDE_UP;  // Slight tilt tolerance
        else if (az < -0.5) return BOTTOM_SIDE_UP;
    }

    // If it's on its side, decide based on tilt
    return (az >= 0) ? TOP_SIDE_UP : BOTTOM_SIDE_UP;
}

void enter_low_power_mode(void)
{
    uint32_t tickstart = HAL_GetTick();

    // ✅ Disable Unused Peripherals Before Sleep
    __HAL_RCC_GPIOA_CLK_DISABLE();
    __HAL_RCC_GPIOB_CLK_DISABLE();
    __HAL_RCC_I2C1_CLK_DISABLE();

    // ✅ Suspend SysTick to prevent unwanted wake-ups
    HAL_SuspendTick();

    // ✅ Set SLEEPDEEP only once before entering sleep loop
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

    while ((HAL_GetTick() - tickstart) < 3600000)  // 1 Hour = 3,600,000 ms
    {
        __WFI();  // ✅ Enter STOP Mode (Deep Sleep)
    }

    // ✅ MCU Wakes Up Here

    // ✅ Restore System Clock (STOP mode disables HSI)
    SystemClock_Config();

    // ✅ Resume SysTick After Wake-up
    HAL_ResumeTick();

    // ✅ Re-enable Peripherals After Wake-up
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    // ✅ Reset I2C & Reinitialize Accelerometer
    __HAL_RCC_I2C1_FORCE_RESET();
    HAL_Delay(10);
    __HAL_RCC_I2C1_RELEASE_RESET();
    MX_I2C1_Init();
    LSM6DS33_init();
}


void start_indicator_blink(void)
{
    // Blink the indicator LED 10 times for full 5 seconds
    for (int i = 0; i < 10; i++)
    {
        HAL_GPIO_TogglePin(INDICATOR_LED_GPIO_Port, TOP_INDICATOR_LED_Pin);
        HAL_GPIO_TogglePin(INDICATOR_LED_GPIO_Port, BOTTOM_INDICATOR_LED_Pin);
        HAL_Delay(500);  // 500ms blink interval
    }
}



void stop_indicator_blink(void)
{
    // Ensure LED is off
    HAL_GPIO_WritePin(INDICATOR_LED_GPIO_Port, TOP_INDICATOR_LED_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(INDICATOR_LED_GPIO_Port, BOTTOM_INDICATOR_LED_Pin, GPIO_PIN_RESET);

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
