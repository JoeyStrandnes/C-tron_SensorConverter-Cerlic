/*
 * SensorConverter.cpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#include <SensorConverter.hpp>

//FIXME
extern CRC_HandleTypeDef hcrc;
extern I2C_HandleTypeDef hi2c1;


SensorConverterSettings::SensorConverterSettings(
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

){
//Simply check what type of configuration to run and link all the peripherals to the backend.



	this->HeartBeatPort = heart_beat_port;
	this->HeartBeatPin = heart_beat_pin;

	this->I2C = i2c;

	this->JP1Port = jp1_Port;
	this->JP1Pin = jp1_pin;

	this->JP2Port = jp2_Port;
	this->JP2Pin = jp2_pin;

	this->JP3Port = jp3_Port;
	this->JP3Pin = jp3_pin;

	this->JP4Port = jp4_Port;
	this->JP4Pin = jp4_pin;

	this->GetSensorType();

	//this->GetSettingsFromEEPROM();
	this->FactoryReset();

	return;

}

void SensorConverterSettings::GetSensorType(){
//Not the most elegant way to check pins...

	uint8_t UnitType = HAL_GPIO_ReadPin(JP1Port, JP1Pin);

	UnitType |= HAL_GPIO_ReadPin(JP2Port, JP2Pin) << 1;
	UnitType |= HAL_GPIO_ReadPin(JP3Port, JP3Pin) << 2;
	UnitType |= HAL_GPIO_ReadPin(JP4Port, JP4Pin) << 3;


	return;
}

void SensorConverterSettings::FactoryReset(){

	this->SerialNumber_H = 0;
	this->SerialNumber_L = 0;
	this->SoftwareVersion = SOFTWARE_VERSION;
	std::memset(this->Tag, 0, 8);

	this->SensorType = TYPE_LT600;
	this->SlaveAddress = TYPE_LT600;
	this->MasterAddress = 10;

	return;
}


void SensorConverterSettings::GetSettingsFromEEPROM(){
//Fetch settings from EEPROM
//Default EEPROM address is 0xA1

	uint8_t MemoryAddress[2];
	MemoryAddress[0] = 0;
	MemoryAddress[1] = 0;

	uint16_t SettingsSize = sizeof(*this);

	//Step 1: Read the data from the EEPROM.
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);

	HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, 0xA0, MemoryAddress, 2, I2C_FIRST_AND_NEXT_FRAME);

	HAL_Delay(1); //HAL_I2C_GetState(I2C) != HAL_I2C_STATE_BUSY_TX

	HAL_I2C_Master_Seq_Receive_IT(&hi2c1, 0xA0, (uint8_t *)this, SettingsSize, I2C_LAST_FRAME);

	HAL_Delay(100); //while(HAL_I2C_GetState(I2C) == HAL_I2C_STATE_BUSY_RX);



	//Step 2: Calculate the checksum of the first 12 bytes of the Settings.
	uint8_t TempBuffer[12];
	std::memcpy(TempBuffer, (uint8_t *)(this), sizeof(TempBuffer));
	uint32_t TempCRC = this->NVM_CRC;
	this->NVM_CRC = HAL_CRC_Calculate(&hcrc, (uint32_t *)TempBuffer, sizeof(TempBuffer)/4);


	//Step 3: Verify the data integrity. Keep if good, reset if not!
	if(TempCRC != this->NVM_CRC){

		//Factory reset the device.
		this->FactoryReset();
		this->WriteSettingsToEEPROM();
	}

	return;
}


void SensorConverterSettings::WriteSettingsToEEPROM(){

	return;

	uint16_t SettingsSize = sizeof(*this);

	uint8_t MemoryAddress[2];
	MemoryAddress[0] = 0;
	MemoryAddress[1] = 0;

	HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, 0xA0, MemoryAddress, 2, I2C_FIRST_AND_NEXT_FRAME);

	HAL_Delay(1);

	//Yes this is very stupid, it is important that this happens before the rest of the system is initiated.
	//ST HAL does not offer a "sequenced" transmit in blocking mode...
	//while(HAL_I2C_GetState(I2C) != HAL_I2C_STATE_READY && HAL_I2C_GetState(I2C) != HAL_I2C_STATE_RESET);

	HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, 0xA0, (uint8_t *)this, SettingsSize, I2C_LAST_FRAME);

	HAL_Delay(100);

	return;
}




















