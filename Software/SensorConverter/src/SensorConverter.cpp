/*
 * SensorConverter.cpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#include <SensorConverter.hpp>

//FIXME
extern CRC_HandleTypeDef hcrc;
__attribute__((section(".system_settings_flash"))) uint32_t NVMSettings[256];

SensorConverterSettings::SensorConverterSettings(
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

){
//Simply check what type of configuration to run and link all the peripherals to the backend.



	this->HeartBeatPort = heart_beat_port;
	this->HeartBeatPin = heart_beat_pin;

	this->JP1Port = jp1_Port;
	this->JP1Pin = jp1_pin;

	this->JP2Port = jp2_Port;
	this->JP2Pin = jp2_pin;

	this->JP3Port = jp3_Port;
	this->JP3Pin = jp3_pin;

	this->JP4Port = jp4_Port;
	this->JP4Pin = jp4_pin;

	this->GetSensorType();

	this->GetSettingsFromEEPROM();

	return;

}

void SensorConverterSettings::GetSensorType(){
//Not the most elegant way to check pins...

	/*
	uint8_t UnitType = HAL_GPIO_ReadPin(JP1Port, JP1Pin);

	UnitType |= HAL_GPIO_ReadPin(JP2Port, JP2Pin) << 1;
	UnitType |= HAL_GPIO_ReadPin(JP3Port, JP3Pin) << 2;
	UnitType |= HAL_GPIO_ReadPin(JP4Port, JP4Pin) << 3;


	switch(UnitType){

	case(0): //TBD
		this->Sensor = new(SensorFLX);
		break;

	}
*/
	this->Sensor = new(SensorFLX);

	return;
}

void SensorConverterSettings::FactoryReset(){

	this->SerialNumber_H = 535;
	this->SerialNumber_L = 128;
	this->SoftwareVersion = SOFTWARE_VERSION;
	std::memset(this->Tag, 0, SENSOR_TAG_SIZE);

	this->SensorType = TYPE_LT600_FLX;
	this->SlaveAddress = TYPE_LT600_FLX;
	this->MasterAddress = 10;

	return;
}


void SensorConverterSettings::GetSettingsFromEEPROM(){
//Fetch settings from EEPROM
//Default EEPROM address is 0xA1

	//Ghetto method used because of tight deadline...

	uint32_t NVM_CRC = NVMSettings[10];
	uint32_t Calc_CRC = HAL_CRC_Calculate(&hcrc, NVMSettings, 10);

	if(NVM_CRC != Calc_CRC){

		this->FactoryReset();

		this->WriteSettingsToEEPROM();


	}
	else{

		uint8_t FlashIndex = 0;

		this->SerialNumber_H = NVMSettings[FlashIndex++];
		this->SerialNumber_L = NVMSettings[FlashIndex++];
		this->SlaveAddress = NVMSettings[FlashIndex++];

		class SensorFLX *Sensor_ptr = (class SensorFLX *)this->Sensor;

		Sensor_ptr->GutterType = NVMSettings[FlashIndex++];

		std::memcpy((uint8_t *)&Sensor_ptr->X1, (uint8_t*)(&NVMSettings[FlashIndex++]), 4);
		std::memcpy((uint8_t *)&Sensor_ptr->X2, (uint8_t*)(&NVMSettings[FlashIndex++]), 4);
		std::memcpy((uint8_t *)&Sensor_ptr->X3, (uint8_t*)(&NVMSettings[FlashIndex++]), 4);

		Sensor_ptr->Width = NVMSettings[FlashIndex++];
		Sensor_ptr->Sill = NVMSettings[FlashIndex++];

		std::memcpy((uint8_t *)&Sensor_ptr->OffsetCal, (uint8_t*)(&NVMSettings[FlashIndex++]), 4);


		std::memset(this->Tag, 0, SENSOR_TAG_SIZE);

	}


	return;
}


void SensorConverterSettings::WriteSettingsToEEPROM(){

	HAL_FLASH_Unlock();

	FLASH_EraseInitTypeDef FlashPage;
	uint32_t PageError = 0;
	FlashPage.TypeErase = FLASH_TYPEERASE_PAGES;
	FlashPage.PageAddress = (uint32_t)NVMSettings;
	FlashPage.NbPages = 1;

	HAL_FLASHEx_Erase(&FlashPage, &PageError);

	uint8_t FlashIndex = 0;

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), this->SerialNumber_H);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), this->SerialNumber_L);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), this->SlaveAddress); //TBD

	class SensorFLX *Sensor_ptr = (class SensorFLX *)this->Sensor;

	uint32_t FloatHolder{0};

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), Sensor_ptr->GutterType);

	std::memcpy((uint8_t *)&FloatHolder, (uint8_t*)(&(Sensor_ptr->X1)), 4);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), FloatHolder);

	std::memcpy((uint8_t *)&FloatHolder, (uint8_t*)(&(Sensor_ptr->X2)), 4);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), FloatHolder);

	std::memcpy((uint8_t *)&FloatHolder, (uint8_t*)(&(Sensor_ptr->X3)), 4);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), FloatHolder);

	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), Sensor_ptr->Width);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), Sensor_ptr->Sill);

	std::memcpy((uint8_t *)&FloatHolder, (uint8_t*)(&(Sensor_ptr->OffsetCal)), 4);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), FloatHolder);

	uint32_t Calc_CRC = HAL_CRC_Calculate(&hcrc, NVMSettings, 10);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t)(&NVMSettings[FlashIndex++]), Calc_CRC);

	HAL_FLASH_Lock();


	return;
}




















