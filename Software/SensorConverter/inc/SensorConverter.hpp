/*
 * SensorConverter.hpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#ifndef INC_SENSORCONVERTER_HPP_
#define INC_SENSORCONVERTER_HPP_

#include "main.h"

#include <cstring>

#define SOFTWARE_VERSION 100




class SensorConverterSettings{

public:

	SensorConverterSettings(
			GPIO_TypeDef *heart_beat_port,
			uint16_t heart_beat_pin,

			I2C_HandleTypeDef *i2c,

			UART_HandleTypeDef *ctron_uart,
			GPIO_TypeDef *ctron_uart_dir_port,
			uint16_t ctron_uart_dir_pin,

			UART_HandleTypeDef *sensor_uart,
			GPIO_TypeDef *sensor_uart_dir_port,
			uint16_t sensor_uart_dir_pin
			);

	//General system info.
	uint16_t SerialNumber_H{0};
	uint16_t SerialNumber_L{0};
	uint16_t SoftwareVersion = {SOFTWARE_VERSION};
	char Tag[10];


	void SendSensorData();

	void HeartBeat(){HAL_GPIO_TogglePin(HeartBeatPort, HeartBeatPin);};



private:

	UART_HandleTypeDef *CtronUART{NULL};
	GPIO_TypeDef *CtronUART_DIR_Port{NULL};
	uint16_t CtronUART_DIR_Pin{0};


	UART_HandleTypeDef *SensorUART{NULL};
	GPIO_TypeDef *SensorUART_DIR_Port{NULL};
	uint16_t SensorUART_DIR_Pin{0};

	I2C_HandleTypeDef *I2C{NULL};

	GPIO_TypeDef *HeartBeatPort{NULL};
	uint16_t HeartBeatPin{0};

	//void CtronUART_RX_Callback();

	//NVM management
	void GetSettingsFromEEPROM();

};


#endif /* INC_SENSORCONVERTER_HPP_ */
