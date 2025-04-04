/*
 * SensorConverter.cpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#include <SensorConverter.hpp>

//FIXME
extern CRC_HandleTypeDef hcrc;


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


	this->FactoryReset();


	return;
/*

	uint8_t MemoryAddress[2];
	MemoryAddress[0] = 0;
	MemoryAddress[1] = 0;

	uint8_t *Settings_ptr = (uint8_t *)this;
	uint16_t SettingsSize = sizeof(*this);

	//Step 1: Read the data from the EEPROM.
	I2C1->CR1 |= I2C_CR1_START;
	while(!(I2C1->SR1 & I2C_SR1_SB)); //Wait for start-bit to be sent

	I2C1->DR = 0xA0;
	while(!(I2C1->SR1 & I2C_SR1_ADDR)); //Wait for device address to be sent

    (void)I2C1->SR1;
    (void)I2C1->SR2;

	//Set the memory address pointer to 0
	I2C1->DR = 0;
	while(!(I2C1->SR1 & I2C_SR1_TXE));
	I2C1->DR = 0;
	while(!(I2C1->SR1 & I2C_SR1_TXE));

	//Send the device address again in read mode to read all the data.
	I2C1->CR1 |= I2C_CR1_START;
	while(!(I2C1->SR1 & I2C_SR1_SB)); //Wait for start-bit to be sent

	I2C1->DR = 0xA1;
	while(!(I2C1->SR1 & I2C_SR1_ADDR));

    (void)I2C1->SR1;
    (void)I2C1->SR2;

	for(uint16_t i = 0; i < SettingsSize; i++){

		while(!(I2C1->SR1 & I2C_SR1_RXNE));
		Settings_ptr[i] = (uint8_t)(I2C1->DR);

	}
*/


	/*
	HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, 0xA0, MemoryAddress, 2, I2C_FIRST_AND_NEXT_FRAME);

	//HAL_I2C_GetState(I2C) != HAL_I2C_STATE_BUSY_TX
	while(!(hi2c1.Instance->SR1 & I2C_SR1_TXE));
	while(!(hi2c1.Instance->SR1 & I2C_SR1_BTF));

	HAL_I2C_Master_Seq_Receive_IT(&hi2c1, 0xA0, (uint8_t *)this, SettingsSize, I2C_LAST_FRAME);

	while(!(hi2c1.Instance->SR1 & I2C_SR1_RXNE));
	//while(!(hi2c1.Instance->SR1 & I2C_SR1_STOPF));
	//while(!(hi2c1.Instance->SR1 & I2C_SR1_STOPF));
*/
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
/*
	uint16_t SettingsSize = sizeof(*this);

	uint8_t MemoryAddress[2];
	MemoryAddress[0] = 0;
	MemoryAddress[1] = 0;

	HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
	HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, 0xA0, MemoryAddress, 2, I2C_FIRST_AND_NEXT_FRAME);

	//HAL_Delay(1);
	//while(!(hi2c1.Instance->SR1 & I2C_SR1_TXE));
	while(!(hi2c1.Instance->SR1 & I2C_SR1_BTF));

	//Yes this is very stupid, it is important that this happens before the rest of the system is initiated.
	//ST HAL does not offer a "sequenced" transmit in blocking mode...
	//while(HAL_I2C_GetState(I2C) != HAL_I2C_STATE_READY && HAL_I2C_GetState(I2C) != HAL_I2C_STATE_RESET);

	HAL_I2C_Master_Seq_Transmit_IT(&hi2c1, 0xA0, (uint8_t *)this, SettingsSize, I2C_LAST_FRAME);

	while(!(hi2c1.Instance->SR1 & I2C_SR1_BTF));
*/
	//HAL_Delay(100);

	return;
}




















