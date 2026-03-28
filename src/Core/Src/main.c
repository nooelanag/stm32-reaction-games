/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include <stdio.h>
#include <stdlib.h>
#include "Utiles_SDM.h"
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
 ADC_HandleTypeDef hadc;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
int option = 0; //Variable used to know what game has been selected.
int winner = 0; //Variable that will be modified once the winner is known.
int random_time = 5000; //The random time variable is set at this value for
//the first use, after that it will be random, as its name states.
int reaction_time_winner;  //Variable used to store the time of the winner.
int permit_print = 0; //A variable used to know if the count-down is shown on screen or not.
int counter = 10; //It is a counter that will be used to print the count-down in game 2.
//The value assigned to this variable is the time at which it starts, in this case 10.
//After the count-down ends, it has to be reset.
int player_1_time_game_2; //This variables stores the time which player 1 takes to press the button in game 2.
int player_2_time_game_2; //This variables stores the time which player 2 takes to press the button in game 2.
int player_1_button = 0; //Variable used to check whether player 1 has pressed the button or not.
int player_2_button = 0; //Variable used to check whether player 2 has pressed the button or not.
uint8_t text[20]; //Used for Bin2Ascii
int speed; //Variable used to store the duration of the step for the count-down in game two. Its value will be assigned with the potentiometer.
int sound = 0;
int playing_melody = 0;
int start_game = 1;
int game_running = 1;
int play = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void value_of_the_ADC() {
    int value_of_the_ADC;
    value_of_the_ADC = ADC1 -> DR; //Where the value of the ADC is stored.
    if(value_of_the_ADC <= 21) { //First range of values. The equivalent step is 500 ms
    	speed = 500;
    	TIM4 -> CCR3 = 500;
    }
    else if(value_of_the_ADC <= 42) { //Second range of values. The equivalent step is 1000 ms
    	speed = 1000;
    	TIM4 -> CCR3 = 1000;
    }
    else { //Third range of values. The equivalent step is 2000 ms
    	speed = 2000;
    	TIM4 -> CCR3 = 2000;
    }
}

void Melody1(void) {
	TIM2 -> CCER |= 0x0001;
	TIM2 -> SR &= ~(1 << 1);
	TIM2 -> DIER |= (1 << 1);
	TIM2 -> DIER |= (1 << 2);
	TIM2 -> CCR1 = 72;
	playing_melody = 1;
	sound = 0;
}

void Melody2(void) {
	TIM2 -> CCER |= 0x0001;
	TIM2 -> SR &= ~(1 << 1);
	TIM2 -> DIER |= (1 << 1);
	TIM2 -> DIER |= (1 << 2);
	TIM2 -> CCR1 = 45;
	playing_melody = 1;
	sound = 1;
}

void EXTI15_10_IRQHandler(void) {
	if(EXTI -> PR != 0) {
		game_running = 0;
		option++;
		if(option > 1)
			option = 0;
		EXTI -> PR |= (1 << 13);
	}
}

void TIM4_IRQHandler(void) {
	if(option == 0) {
		if((TIM4 -> SR & (1 << 2)) != 0) {
			winner = 1;
			reaction_time_winner = TIM4 -> CCR2;
			TIM4 -> SR &= ~(1 << 2);
			random_time = reaction_time_winner * 10;
		}
		if((TIM4 -> SR & (1 << 1)) != 0) {
			winner = 2;
			reaction_time_winner = TIM4 -> CCR1;
			TIM4 -> SR &= ~(1 << 1);
			random_time = reaction_time_winner * 10;
		}
		if((TIM4 -> SR & (1 << 3)) != 0) {
			if(start_game == 0)
				start_game = 2;
			if(play == 0)
				play = 1;
			TIM4 -> SR &= ~(1 << 3);
		}
	}
	/*else if(option == 1) {
		if((TIM4 -> SR & (1 << 3)) != 0) {
			if(TIM4 -> CCR3 < speed * 10)
				TIM4 -> CCR3 += speed;
			else {
				TIM4 -> DIER &= ~(1 << 3);
				TIM4 -> CCR3 = speed;
			}
			permit_print = 1;
			TIM4 -> SR &= ~(1 << 3);
		}
		else if((TIM4 -> SR & (1 << 2)) != 0) {
			player_1_button = 1;
			player_1_time_game_2 = TIM4 -> CCR2;
			random_time = abs(TIM4 -> CCR2 - speed * 10) * 10;
			TIM4 -> SR &= ~(1 << 2);
		}
		else if((TIM4 -> SR & (1 << 1)) != 0) {
			player_2_button = 1;
			player_2_time_game_2 = TIM4 -> CCR1;
			random_time = abs(TIM4 -> CCR1 - speed * 10) * 10;
			TIM4 -> SR &= ~(1 << 1);
		}
		else if((TIM4 -> SR & (1 << 4)) != 0) {
			play = 1;
			TIM4 -> SR &= ~(1 << 4);
		}
	}*/
}
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
  MX_USART2_UART_Init();
  MX_ADC_Init();
  MX_TIM2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  //Setup USER_BUTTON as a digital input
   GPIOC -> MODER &= ~(1 << (13 * 2));
   GPIOC -> MODER &= ~(1 << (13 * 2 + 1));

   //Setup BUTTON_1 as a digital input
   GPIOB -> MODER &= ~(1 << (7 * 2));
   GPIOB -> MODER |= (1 << (7 * 2 + 1));

   GPIOB -> AFR[0] |= (1 << (4 * 7 + 1));

   //Setup BUTTON_2 as a digital input
   GPIOB -> MODER &= ~(1 << (6 * 2));
   GPIOB -> MODER |= (1 << (6 * 2 + 1));

   GPIOB -> AFR[0] |= (1 << (4 * 6 + 1));

   //Setup LED_1 as a digital output
   GPIOA -> MODER |= (1 << (12 * 2));
   GPIOA -> MODER &= ~(1 << (12 * 2 + 1));

   //Setup LED_2 as a digital output
   GPIOD -> MODER |= (1 << (2 * 2));
   GPIOD -> MODER &= ~(1 << (2 * 2 + 1));

   //Set up EXTI13
   SYSCFG -> EXTICR[3] |= (1 << 5);
   EXTI -> IMR |= (1 << 13);
   EXTI -> RTSR &= ~(1 << 13);
   EXTI -> FTSR |= (1 << 13);
   NVIC -> ISER[1] |= (1 << 8);

  //Set up PA4 as analog
   GPIOA -> MODER |= (1 << 4 * 2);
   GPIOA -> MODER |= (1 << (4 * 2 + 1));

   //Set up PA5 as alternate function
   GPIOA -> MODER |= (1 << (5 * 2 + 1));
   GPIOA -> AFR[0] |= (1 << 4 * 5);

   //Set up TIM4
   TIM4 -> CR1 = 0x0000;
   TIM4 -> CR2 = 0x0000;
   TIM4 -> SMCR = 0x0000;
   TIM4 -> PSC = 31999; //32MHz / PSC = 1 / frequency = 10^-3 s. Each count is 1 ms
   TIM4 -> CNT = 0;
   TIM4 -> ARR = 0xffff;
   TIM4 -> DIER |= (1 << 1);
   TIM4 -> DIER |= (1 << 2);
   TIM4 -> DIER |= (1 << 3);
   //TIM4 -> DIER |= (1 << 4);
   TIM4 -> CCMR1 |= 0x0000;
   TIM4 -> CCMR1 |= (1 << 0);
   TIM4 -> CCMR1 |= (1 << 8);
   TIM4 -> CCMR2 = 0x0000;
   TIM4 -> CCER = 0x0000;
   TIM4 -> CCER |= (1 << 0);

   TIM4 -> CCER |= (1 << 4);
   NVIC -> ISER[0] |= (1 << 30);
   TIM4 -> CCR3 = random_time;
   //TIM4 -> CCR4 = 3000;
   TIM4 -> CR1 |= 0x0001;
   TIM4 -> SR = 0x0000;
   TIM4 -> EGR |= 0x0001;

   //Set up ADC1
   ADC1 -> CR2 &= ~(1 << 0); //ADON
   ADC1 -> CR1 |= (1 << 24); //RES[0]
   ADC1 -> CR1 |= (1 << 25); //RES[1]
   ADC1 -> CR2 |= (1 << 1); //CONT
   ADC1 -> CR2 |= (1 << 0); //ADON
   ADC1 -> CR2 |= (1 << 30); //SWSTART

   //Set up TIM2
   TIM2 -> CR1 = 0x0000;
   TIM2 -> CR2 = 0x0000;
   TIM2 -> SMCR = 0x0000;
   TIM2 -> PSC = 31;
   TIM2 -> CNT = 0;
   TIM2 -> ARR = 0xffff;
   TIM2 -> CCMR1 = 0x0000;
   TIM2 -> CCMR1 |= (1 << 4);
   TIM2 -> CCMR1 |= (1 << 5);
   TIM2 -> CCMR2 = 0x0000;
   TIM2 -> CCER = 0x0000;
   TIM2 -> CCR1 = 150;
   TIM2 -> CCR2 = 20000;//Seleccionar
   TIM2 -> CR1 = 0x0001;
   TIM2 -> SR = 0x0000;
   TIM2 -> EGR |= 0x0001;
   NVIC -> ISER[0] |= (1 << 28);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(play == 1) {
	  switch(option) {
	  		  	  case 0:
	  		  		  game_running = 1;
	  		  		  while(game_running != 0) {
	  		  			  if(start_game == 1) {
	  		  				  printf("GAME 1:\r\n");
	  		  				  start_game = 0;
	  		  				  TIM4 -> CNT = 0;
	  		  				  TIM4 -> CCR3 = random_time;
	  		  			  }
	  		  			  if(start_game == 2) {
	  		  				  GPIOA -> BSRR |= (1 << 12);
	  		  				  TIM4 -> CNT = 0;
	  		  			  }
	  		  			  if(winner != 0) {
	  		  				  if(winner == 1) {
	  		  					  Bin2Ascii(reaction_time_winner, text);
	  		  					  printf("-1");
	  		  				  }
	  		  				  else if(winner == 2) {
	  		  					  Bin2Ascii(reaction_time_winner, text);
	  		  					  printf("-2");
	  		  				  }
	  		  				  printf(text);
	  		  				  printf("\r\n");

	  		  				  winner = 0;
	  		  				  GPIOA -> BSRR |= (1 << (12 + 16));
	  		  				  start_game = 1;
	  		  				  game_running = 0;
	  		  			  }

	  		  		  }
	  		  		play = 0;
	  		  		TIM4 -> CNT = 0;
	  		  		TIM4 -> CCR3 = 3000;
	  		  		  break;
	  		  	  case 1:
	  		  		  game_running = 1;
	  		  		  printf("GAME 2\r\n");
	  		  		  break;
	  		  	  }
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
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLL_DIV3;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{

  /* USER CODE BEGIN ADC_Init 0 */

  /* USER CODE END ADC_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC_Init 1 */

  /* USER CODE END ADC_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait = ADC_AUTOWAIT_DISABLE;
  hadc.Init.LowPowerAutoPowerOff = ADC_AUTOPOWEROFF_DISABLE;
  hadc.Init.ChannelsBank = ADC_CHANNELS_BANK_A;
  hadc.Init.ContinuousConvMode = DISABLE;
  hadc.Init.NbrOfConversion = 1;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = DISABLE;
  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_4CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC_Init 2 */

  /* USER CODE END ADC_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 0;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

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
