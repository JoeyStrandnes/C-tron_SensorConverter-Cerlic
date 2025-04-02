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

class SensorClass;

class SensorConverterSettings{

public:

	SensorConverterSettings(
			GPIO_TypeDef *heart_beat_port,
			uint16_t heart_beat_pin,

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

	uint8_t 	SensorType{TYPE_LT600_FLX}; 	//What type of sensor we are emulating.
	uint8_t		SlaveAddress{TYPE_LT600_FLX};	//Address that we use as slaves
	uint8_t		MasterAddress{10};			//Address we use as masters

	class SensorClass *Sensor;

	void HeartBeat(){HAL_GPIO_TogglePin(HeartBeatPort, HeartBeatPin);};
	void WriteSettingsToEEPROM();
	void FactoryReset();

private:

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

	void GetSettingsFromEEPROM();

	uint32_t NVM_CRC{0}; //This is used to verify the valididity of the NVM.


};




class SensorClass{
//Base class mostly used for handling the different calibrations.

public:

	SensorClass(){};

	virtual uint8_t Calibrate(class ModBusRTU_BaseClass *modbus){return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;};
	virtual float CalculateMeasurement(){return 0;};

	float RawData{0}; //Input value, can be seen as "ADC raw".


private:


};


class SensorFLX : public SensorClass{

public:

	SensorFLX(){};
	uint8_t Calibrate(class ModBusRTU_BaseClass *modbus);
	float CalculateMeasurement();

	void GetGutterName(char *buffer, uint8_t buffer_size);

	float mH2O; //Meter vattenpelare

	//Arguments used for calibration. Same as FLX X1.. in BB2.
	uint8_t GutterType{0};

	float X1{0.3812};
	float X2{1.580};
	float X3{0};

	uint16_t Width{6};
	uint16_t Sill{0};

	float OffsetCal{0};
	uint32_t OffsetCalDate{0};

	float LevelCal{0};
	uint32_t LevelCalDate{0};


private:

	void LoadDefaultFlumeParamters();
	void CalculateGutterCoefficient(uint16_t Arg1, uint16_t Arg2, uint16_t Arg3);


};



























#endif /* INC_SENSORCONVERTER_HPP_ */
