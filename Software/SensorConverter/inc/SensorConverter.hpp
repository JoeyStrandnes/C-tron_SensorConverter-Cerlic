/*
 * SensorConverter.hpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#ifndef INC_SENSORCONVERTER_HPP_
#define INC_SENSORCONVERTER_HPP_

#include "main.h"
#include <SensorRegisterMaps.hpp>
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
			uint16_t sensor_uart_dir_pin,

			GPIO_TypeDef *jp1_Port,
			uint16_t jp1_pin,

			GPIO_TypeDef *jp2_Port,
			uint16_t jp2_pin,

			GPIO_TypeDef *jp3_Port,
			uint16_t jp3_pin,

			GPIO_TypeDef *jp4_Port,
			uint16_t jp4_pin
			);

	//General system info.
	uint16_t 	SerialNumber_H{0};
	uint16_t 	SerialNumber_L{0};
	uint16_t 	SoftwareVersion = {SOFTWARE_VERSION};
	uint8_t 	SensorType{0};
	char 		Tag[10];

	uint8_t MasterAddress{0};
	uint8_t SlaveAddress{0};


	void FetchSensorData();
	void HeartBeat(){HAL_GPIO_TogglePin(HeartBeatPort, HeartBeatPin);};


	//TODO Make nicer
	void (*ReloadMasterRegisters)(struct Measurement_Register *registers[2]) = NULL;
	void (*ReloadSlaveRegisters)(struct Measurement_Register *registers[2]) = NULL;

private:

	// 2 types of registers stored with each sensor. Settings & Data. Index 0 is Settings and Index 1 is Data.
	struct Measurement_Register *MasterRegisters[2] = {NULL,NULL};
	struct Measurement_Register *SlaveRegisters[2] = {NULL,NULL};
	uint8_t MasterRegisterCount[2];
	uint8_t SlaveRegisterCount[2];


	UART_HandleTypeDef *CtronUART{NULL};
	GPIO_TypeDef *CtronUART_DIR_Port{NULL};
	uint16_t CtronUART_DIR_Pin{0};


	UART_HandleTypeDef *SensorUART{NULL};
	GPIO_TypeDef *SensorUART_DIR_Port{NULL};
	uint16_t SensorUART_DIR_Pin{0};

	I2C_HandleTypeDef *I2C{NULL};

	GPIO_TypeDef *HeartBeatPort{NULL};
	uint16_t HeartBeatPin{0};

	GPIO_TypeDef *JP1Port{NULL};
	uint16_t JP1Pin{0};

	GPIO_TypeDef *JP2Port{NULL};
	uint16_t JP2Pin{0};

	GPIO_TypeDef *JP3Port{NULL};
	uint16_t JP3Pin{0};

	GPIO_TypeDef *JP4Port{NULL};
	uint16_t JP4Pin{0};

	//void CtronUART_RX_Callback();

	//NVM management
	void GetSensorType();
	void GetSettingsFromEEPROM();

};





































#endif /* INC_SENSORCONVERTER_HPP_ */
