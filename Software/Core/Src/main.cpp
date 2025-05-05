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
#include "dma.h"
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
	void UART1_IRQ();

}
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_CRC_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */
  class SensorConverterSettings Settings(
		  LED_1_GPIO_Port,
		  LED_1_Pin,

		  JP1_GPIO_Port,
		  JP1_Pin,

		  JP2_GPIO_Port,
		  JP2_Pin,

		  JP3_GPIO_Port,
		  JP3_Pin,

		  JP4_GPIO_Port,
		  JP4_Pin

  );

  //TODO for testing.
  Settings.MasterAddress = Settings.SensorType;
  Settings.SlaveAddress = Settings.SensorType;
  Settings.SerialNumber_H = 335;
  Settings.SerialNumber_L = 123;

//ModBus Slave
  ModBusSlave.SettingsPtr = &Settings;
  ModBusSlave.Address = Settings.SlaveAddress;

  ModBusSlave.OutputBuffer = (uint8_t *)SlaveTxBuffer;
  ModBusSlave.InputBuffer = (uint8_t *)SlaveRxBuffer;

  ModBusSlave.OutputBufferSize = sizeof(SlaveTxBuffer)/sizeof(SlaveTxBuffer[0]);
  ModBusSlave.InputBufferSize = sizeof(SlaveRxBuffer)/sizeof(SlaveRxBuffer[0]);

//ModBus Master
  ModBusMaster.SettingsPtr = &Settings;
  ModBusMaster.Address = Settings.MasterAddress;

  ModBusMaster.OutputBuffer = (uint8_t *)MasterTxBuffer;
  ModBusMaster.InputBuffer = (uint8_t *)MasterRxBuffer;

  ModBusMaster.OutputBufferSize = sizeof(MasterTxBuffer)/sizeof(MasterTxBuffer[0]);
  ModBusMaster.InputBufferSize = sizeof(MasterRxBuffer)/sizeof(MasterRxBuffer[0]);

  LinkSensorConfig(&ModBusMaster, &ModBusSlave, ModBusSlave.SettingsPtr->SensorType);

  ModBusMaster.ReadAllSensorData();

  //Start the timer responsible for periodically polling the sensor.
  HAL_TIM_Base_Start_IT(&htim3);

  //Enable C-tron communication, for whatever reason the IDLE event does not trigger in DMA mode.
  HAL_UARTEx_ReceiveToIdle_IT(&huart1, ModBusSlave.InputBuffer, ModBusSlave.InputBufferSize);
 __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);


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

	  /** Configure the main internal regulator output voltage
	  */
	  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

	  /** Initializes the RCC Oscillators according to the specified parameters
	  * in the RCC_OscInitTypeDef structure.
	  */
	  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
	  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
	  RCC_OscInitStruct.PLL.PLLN = 8;
	  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	  {
	    Error_Handler();
	  }

	  /** Initializes the CPU, AHB and APB buses clocks
	  */
	  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
	                              |RCC_CLOCKTYPE_PCLK1;
	  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){

	if(htim->Instance == TIM3){ //Used for periodically sampling the slave sensor.

		HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_SET);

		HAL_UART_Transmit_DMA(&huart2, ModBusMaster.OutputBuffer, ModBusMaster.ResponseSize);
		__HAL_UART_ENABLE_IT(&huart2, DMA_IT_TC);

	}

	if(htim->Instance == TIM4){ //Used for delayed response to C-tron.

		HAL_TIM_Base_Stop_IT(htim);

		HAL_UART_Transmit_DMA(&huart1, ModBusSlave.OutputBuffer, ModBusSlave.ResponseSize);
		__HAL_UART_ENABLE_IT(&huart1, DMA_IT_TC);

	}

	return;

}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){


	if(huart->Instance == USART1){


		ModBusSlave.RequestSize = Size; //Get how many bytes were received.

		LoadModBusRegisters(&ModBusMaster, &ModBusSlave, ModBusSlave.SettingsPtr->SensorType);
		ModBusSlave.ParseMasterRequest();

		//Clear error flags
		huart->Instance->ICR = (USART_ICR_PECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_ORECF);

		if(ModBusSlave.ResponseSize == 0){

			HAL_UARTEx_ReceiveToIdle_IT(huart, ModBusSlave.InputBuffer, ModBusSlave.InputBufferSize);
			__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

			HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET);

			return;

		}


		HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET);
		HAL_TIM_Base_Start_IT(&htim4);

	}

	else if(huart->Instance == USART2){

		ModBusMaster.RequestSize = Size;
		ModBusMaster.ParseSlaveResponse();

		std::memset((uint8_t *)ModBusMaster.InputBuffer, 0, ModBusMaster.InputBufferSize);

		//Clear error flags
		huart->Instance->ICR = (USART_ICR_PECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_ORECF);


	}

	return;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){

	if(huart->Instance == USART1){

		HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET);

		HAL_UARTEx_ReceiveToIdle_IT(huart, ModBusSlave.InputBuffer, ModBusSlave.InputBufferSize);
		__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

	}

	else if(huart->Instance == USART2){

		HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET);

		HAL_UARTEx_ReceiveToIdle_IT(huart, ModBusMaster.InputBuffer, ModBusMaster.InputBufferSize);
		__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

	}

	return;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){

	//Clear error flags
	huart->Instance->ICR = (USART_ICR_PECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_ORECF);

	//UART for SCADA
	if(huart->Instance == USART1){
		HAL_UARTEx_ReceiveToIdle_IT(huart, ModBusSlave.InputBuffer, ModBusSlave.InputBufferSize);
	}
	else if(huart->Instance == USART2){
		//HAL_UARTEx_ReceiveToIdle_IT(huart, ModBusMaster.InputBuffer, ModBusMaster.InputBufferSize);
	}

	__HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);

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
