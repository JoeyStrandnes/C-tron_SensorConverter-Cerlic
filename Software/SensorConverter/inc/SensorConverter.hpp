/*
 * SensorConverter.hpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#ifndef INC_SENSORCONVERTER_HPP_
#define INC_SENSORCONVERTER_HPP_

#include "main.h"

#include <ModBus.hpp>
#include <cstring>

#define SOFTWARE_VERSION 101
#define SENSOR_TAG_SIZE 8				//Number of bytes for the sensor tag.

class SensorConverterSettings{

public:

	SensorConverterSettings(
			GPIO_TypeDef *heart_beat_port,
			uint16_t heart_beat_pin,

			I2C_HandleTypeDef *i2c,

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
	char 		Tag[SENSOR_TAG_SIZE];

	uint8_t 	SensorType{TYPE_LT600}; 	//What type of sensor we are emulating.
	uint8_t		SlaveAddress{TYPE_LT600};	//Address that we use as slaves
	uint8_t		MasterAddress{10};			//Address we use as masters

	void HeartBeat(){HAL_GPIO_TogglePin(HeartBeatPort, HeartBeatPin);};

	void WriteSettingsToEEPROM();

private:

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

	//NVM management
	void GetSensorType();
	void FactoryReset();

	void GetSettingsFromEEPROM();

	uint32_t NVM_CRC{0}; //This is used to verify the valididity of the NVM.


};





































#endif /* INC_SENSORCONVERTER_HPP_ */
