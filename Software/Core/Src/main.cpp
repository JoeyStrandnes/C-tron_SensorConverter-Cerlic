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
#include "crc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <SensorConverter.hpp>
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
volatile uint8_t SlaveRxBuffer[32];
volatile uint8_t SlaveTxBuffer[256];

volatile uint8_t MasterRxBuffer[256];
volatile uint8_t MasterTxBuffer[32];

class ModBusRTU_SlaveClass ModBusSlave;
class ModBusRTU_MasterClass ModBusMaster;

extern DMA_HandleTypeDef hdma_usart1_rx;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
extern "C"{
	void ModBusSlaveRxHandler();

}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LoadRegisters();
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_CRC_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  class SensorConverterSettings Settings(
		  LED_GPIO_Port,
		  LED_Pin,
		  &hi2c1,

		  JP1_GPIO_Port,
		  JP1_Pin,

		  JP2_GPIO_Port,
		  JP2_Pin,

		  JP3_GPIO_Port,
		  JP3_Pin,

		  JP4_GPIO_Port,
		  JP4_Pin

  );


  Settings.SerialNumber_H = 435;
  Settings.SerialNumber_L = 128;

//ModBus Slave
  ModBusSlave.SettingsPtr = &Settings;
  ModBusSlave.Address = Settings.SlaveAddress;

  ModBusSlave.OutputBuffer = (uint8_t *)SlaveTxBuffer;
  ModBusSlave.InputBuffer = (uint8_t *)SlaveRxBuffer;

  ModBusSlave.OutputBufferSize = 256;
  ModBusSlave.InputBufferSize = 32;

  ModBusSlave.LinkRegisterMap(&LT600_SlaveRegisterMap);
  ModBusSlave.LoadRegisterMap();


//ModBus Master
  ModBusMaster.SettingsPtr = &Settings;
  ModBusMaster.Address = Settings.MasterAddress;

  ModBusMaster.OutputBuffer = (uint8_t *)MasterTxBuffer;
  ModBusMaster.InputBuffer = (uint8_t *)MasterRxBuffer;

  ModBusMaster.OutputBufferSize = 32;
  ModBusMaster.InputBufferSize = 256;

  ModBusMaster.LinkRegisterMap(&LT600_MasterRegisterMap);
  ModBusMaster.LoadRegisterMap();

  ModBusMaster.ReadAllSensorData();


  LL_USART_EnableIT_RXNE(USART1);

  //HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)SlaveRxBuffer, ModBusSlave.InputBufferSize);
  //HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)SlaveRxBuffer, 8);

  //HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(USART2_DIR_GPIO_Port, USART2_DIR_Pin, GPIO_PIN_SET);
  HAL_UART_Transmit_IT(&huart2, (uint8_t *)ModBusMaster.OutputBuffer, ModBusMaster.ResponseSize);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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

/* USER CODE BEGIN 4 */

void ModBusSlaveRxHandler(){
//This gets called for every byte received on the UART.

	if(ModBusSlave.RequestSize >= ModBusSlave.InputBufferSize){ //ERROR
		HAL_TIM_Base_Stop_IT(&htim2);
		return;
	}

	HAL_TIM_Base_Start_IT(&htim2);

	ModBusSlave.InputBuffer[ModBusSlave.RequestSize++] = LL_USART_ReceiveData8(USART1);//USART1->DR; //Also clears flag


	return;
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){


	if(htim->Instance == TIM2){
		//We get here when a transmission on UART 1 is IDLE. Basically a crude IDLE detection.

		LL_USART_DisableIT_RXNE(USART1);
		ModBusSlave.RequestSize = 0;

		return;
	}

	if(htim->Instance == TIM3){

		//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(USART2_DIR_GPIO_Port, USART2_DIR_Pin, GPIO_PIN_SET);

		HAL_UART_Transmit_IT(&huart2, (uint8_t *)ModBusMaster.OutputBuffer, ModBusMaster.ResponseSize);
		HAL_TIM_Base_Stop_IT(&htim3);

		return;
	}

	//Delay for the response to C-tron
	if(htim->Instance == TIM4){

		HAL_GPIO_WritePin(USART1_DIR_GPIO_Port, USART1_DIR_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

		//HAL_UART_Transmit_IT(&huart1, (uint8_t *)ModBusSlave.OutputBuffer, ModBusSlave.ResponseSize);
		HAL_TIM_Base_Stop_IT(&htim4);

	}

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){

	volatile uint16_t Error;

	if(huart->Instance == USART1){

		//FIXME Only for testing
		if(ModBusSlave.Address == ModBusSlave.InputBuffer[0]){

			LoadRegisters();

			ModBusSlave.RequestSize = Size;
			ModBusSlave.ParseMasterRequest();

			HAL_TIM_Base_Start_IT(&htim4); //Triggers after 5 ms to give some delay before transmitting new data

		}
		else{

			//HAL_UART_Abort(&huart1);

			//Packet was not for us. Re enable the reception.
			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(USART1_DIR_GPIO_Port, USART1_DIR_Pin, GPIO_PIN_RESET);

			//HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)SlaveRxBuffer, ModBusSlave.InputBufferSize);
			//HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)SlaveRxBuffer, 8);

		}

		ModBusSlave.RequestSize = 0;
		std::memset((uint8_t *)SlaveRxBuffer, 0, ModBusSlave.InputBufferSize);

		Error = huart->Instance->SR;
		Error = huart->Instance->DR;

	}

	if(huart->Instance == USART2){

		ModBusMaster.RequestSize = Size;
		ModBusMaster.ParseSlaveResponse();
		std::memset((uint8_t *)ModBusMaster.InputBuffer, 0, ModBusMaster.InputBufferSize);

		Error = huart->Instance->SR;
		Error = huart->Instance->DR;

	}

	UNUSED(Error);

	return;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){

	if(huart->Instance == USART1){

		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(USART1_DIR_GPIO_Port, USART1_DIR_Pin, GPIO_PIN_RESET);

		//HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)SlaveRxBuffer, ModBusSlave.InputBufferSize);

	}
	if(huart->Instance == USART2){

		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(USART2_DIR_GPIO_Port, USART2_DIR_Pin, GPIO_PIN_RESET);

		HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t *)ModBusMaster.InputBuffer, 20);

		HAL_TIM_Base_Start_IT(&htim3); //Overflows after 250 ms. Used to indicate "loss of sensors"

	}

	return;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){

	volatile uint16_t Error;

	//UART for sensor
	if(huart->Instance == USART1){

		//Clear the flags.
		Error = huart->Instance->SR;
		Error = huart->Instance->DR;

		//HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)SlaveRxBuffer, ModBusSlave.InputBufferSize);
		//HAL_UARTEx_ReceiveToIdle_IT(&huart1, (uint8_t *)SlaveRxBuffer, 8);

	}

	//UART for SCADA
	if(huart->Instance == USART2){

		Error = huart->Instance->SR;
		Error = huart->Instance->DR;

	}

	UNUSED(Error);

	return;
}


void LoadRegisters(){

	ModBusSlave.RegisterMap[0][0].InputData.UINT16 = ModBusSlave.SettingsPtr->SerialNumber_H;
	ModBusSlave.RegisterMap[0][1].InputData.UINT16 = ModBusSlave.SettingsPtr->SerialNumber_L;
	ModBusSlave.RegisterMap[0][2].InputData.UINT16 = ModBusSlave.SettingsPtr->SoftwareVersion;
	ModBusSlave.RegisterMap[0][3].InputData.UINT16 = ModBusSlave.Address;

	ModBusSlave.RegisterMap[0][4].InputData.UINT16 = 0;
	ModBusSlave.RegisterMap[0][5].InputData.UINT16 = 0;
	ModBusSlave.RegisterMap[0][6].InputData.UINT16 = 0;
	ModBusSlave.RegisterMap[0][7].InputData.UINT16 = 0;


	ModBusSlave.RegisterMap[1][0].OutputData = ModBusMaster.RegisterMap[1][0].OutputData;
	ModBusSlave.RegisterMap[1][1].InputData.UINT16 = (uint16_t)(ModBusMaster.RegisterMap[1][2].OutputData * ModBusSlave.RegisterMap[1][1].ScaleFactor);
	ModBusSlave.RegisterMap[1][2].InputData.UINT16 = (uint16_t)(ModBusMaster.RegisterMap[1][3].OutputData * ModBusSlave.RegisterMap[1][2].ScaleFactor);

	return;
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
