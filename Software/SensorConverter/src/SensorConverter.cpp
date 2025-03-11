/*
 * SensorConverter.cpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#include <SensorConverter.hpp>




//Simply check what type of configuration to run
SensorConverterSettings::SensorConverterSettings(
	GPIO_TypeDef *heart_beat_port,
	uint16_t heart_beat_pin,

	I2C_HandleTypeDef *i2c,

	UART_HandleTypeDef *ctron_uart,
	GPIO_TypeDef *ctron_uart_dir_port,
	uint16_t ctron_uart_dir_pin,

	UART_HandleTypeDef *sensor_uart,
	GPIO_TypeDef *sensor_uart_dir_port,
	uint16_t sensor_uart_dir_pin){




	this->HeartBeatPort = heart_beat_port;
	this->HeartBeatPin = heart_beat_pin;

	this->I2C = i2c;

	this->CtronUART = ctron_uart;
	this->SensorUART = sensor_uart;



	this->GetSettingsFromEEPROM();

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
