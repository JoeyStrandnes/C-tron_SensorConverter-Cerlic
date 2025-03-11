/*
 * SensorConverter.cpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#include <SensorConverter.hpp>



SensorConverterSettings::SensorConverterSettings(
	GPIO_TypeDef *heart_beat_port,
	uint16_t heart_beat_pin,

	I2C_HandleTypeDef *i2c,

	UART_HandleTypeDef *ctron_uart,
	GPIO_TypeDef *ctron_uart_dir_port,
	uint16_t ctron_uart_dir_pin,

	UART_HandleTypeDef *sensor_uart,
	GPIO_TypeDef *sensor_uart_dir_port,
	uint16_t sensor_uart_dir_pin,

	GPIO_TypeDef *jp1_Port,
	uint16_t jp1_pin,

	GPIO_TypeDef *jp2_Port,
	uint16_t jp2_pin,

	GPIO_TypeDef *jp3_Port,
	uint16_t jp3_pin,

	GPIO_TypeDef *jp4_Port,
	uint16_t jp4_pin

){
//Simply check what type of configuration to run and link all the peripherals to the backend.



	this->HeartBeatPort = heart_beat_port;
	this->HeartBeatPin = heart_beat_pin;

	this->I2C = i2c;

	this->CtronUART = ctron_uart;
	this->CtronUART_DIR_Port = ctron_uart_dir_port;
	this->CtronUART_DIR_Pin = ctron_uart_dir_pin;

	this->SensorUART = sensor_uart;
	this->SensorUART_DIR_Port = sensor_uart_dir_port;
	this->SensorUART_DIR_Pin = sensor_uart_dir_pin;

	this->JP1Port = jp1_Port;
	this->JP1Pin = jp1_pin;

	this->JP2Port = jp2_Port;
	this->JP2Pin = jp2_pin;

	this->JP3Port = jp3_Port;
	this->JP3Pin = jp3_pin;

	this->JP4Port = jp4_Port;
	this->JP4Pin = jp4_pin;



	//HAL_UART_RegisterCallback(this->CtronUART, HAL_UART_RX_COMPLETE_CB_ID, this->CtronUART_RX_Callback);

	this->GetSensorType();

	this->GetSettingsFromEEPROM();

	return;

}

static uint8_t TestRxBuffer[50];
extern UART_HandleTypeDef huart2;

void SensorConverterSettings::FetchSensorData(){
//FIXME For testing, assume a LT600 is connected.

	uint8_t TestBuffer[10];

	TestBuffer[0] = 0x0A;
	TestBuffer[1] = 0x04;
	TestBuffer[2] = 0x03;
	TestBuffer[3] = 0xE8;
	TestBuffer[4] = 0x00;
	TestBuffer[5] = 0x07;
	TestBuffer[6] = 0x30;
	TestBuffer[7] = 0xC3;


	HAL_GPIO_WritePin(this->SensorUART_DIR_Port, this->SensorUART_DIR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(this->HeartBeatPort, this->HeartBeatPin, GPIO_PIN_SET);

	//HAL_Delay(2);

	HAL_UART_Transmit(this->SensorUART, TestBuffer, 8, 10);
	//HAL_UART_Transmit(&huart2, TestBuffer, 8, 10);

	HAL_GPIO_WritePin(this->SensorUART_DIR_Port, this->SensorUART_DIR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(this->HeartBeatPort, this->HeartBeatPin, GPIO_PIN_RESET);

	//HAL_UART_Receive(&huart2, TestRxBuffer, 19, 100);
	HAL_UART_Receive(this->SensorUART, TestRxBuffer, 19, 1000);

	//HAL_Delay(500);


	return;
}

void SensorConverterSettings::GetSensorType(){
//Not the most elegant way to check pins...

	uint8_t UnitType = HAL_GPIO_ReadPin(JP1Port, JP1Pin);

	UnitType |= HAL_GPIO_ReadPin(JP2Port, JP2Pin) << 1;
	UnitType |= HAL_GPIO_ReadPin(JP3Port, JP3Pin) << 2;
	UnitType |= HAL_GPIO_ReadPin(JP4Port, JP4Pin) << 3;


	//TODO Setup what sensor is connected to the device.
	this->SensorType = TYPE_LT600;
	this->MasterRegisterCount[0] = 0;
	this->MasterRegisterCount[1] = 7;

	LinkSensorConfig(this);

	this->ReloadMasterRegisters(this->MasterRegisters);

	return;
}


void SensorConverterSettings::GetSettingsFromEEPROM(){
//Fetch settings from EEPROM
//Default EEPROM address is 0xA0
/*
	uint8_t TestBuffer[15];

	TestBuffer[0] = 0;
	TestBuffer[1] = 0;

	for(uint8_t i = 2; i < 15; i++){
		TestBuffer[i] = 0xAA;
	}

	HAL_I2C_Master_Transmit(I2C, 0xA0, TestBuffer, 15, 50);

	HAL_Delay(100);

	std::memset((void *)TestBuffer, 0, 15);

	HAL_I2C_Master_Receive(I2C, (0xA0 | 1), TestBuffer, 15, 50);
*/
	return;
}

