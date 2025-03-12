/*
 * SensorConfig.cpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */


#include <SensorRegisterMaps.hpp>

void LinkSensorConfig(class SensorConverterSettings *settings){

/*
	switch(settings->SensorType){

	case(TYPE_CMC):
		settings->ReloadMasterRegisters = &CMC_MasterRegisters;
		settings->ReloadSlaveRegisters = &CMC_SlaveRegisters;
		break;
	case(TYPE_LT600):
		settings->SlaveAddress = 10;
		settings->ReloadMasterRegisters = &LT600_MasterRegisters;
		settings->ReloadSlaveRegisters = &LT600_SlaveRegisters;
		break;
	case(TYPE_LT600_FLX):
		settings->SlaveAddress = 10;
		settings->ReloadMasterRegisters = &LT600_FLX_MasterRegisters;
		settings->ReloadSlaveRegisters = &LT600_FLX_SlaveRegisters;
		break;


	}
*/
	//settings->MasterAddress = settings->SensorType;

	return;
}



void CMC_MasterRegisters(struct Measurement_Register *registers[2]){
	return;
}


void CMC_SlaveRegisters(struct Measurement_Register *registers[2]){
	return;
}


void LT600_MasterRegisters(struct Measurement_Register *registers[2]){
	return;
}


void LT600_SlaveRegisters(struct Measurement_Register *registers[2]){
	return;
}

void LT600_FLX_MasterRegisters(struct Measurement_Register *registers[2]){

	//Allocate all the memory
	registers[0] = NULL; //Not used

	if(registers[1] == NULL){
		registers[1] = (struct Measurement_Register *)malloc(7 * sizeof(struct Measurement_Register));
	}

	uint8_t RegisterIndex{0};

	registers[1][RegisterIndex].Index = 0;
	registers[1][RegisterIndex++].RegType = UINT16;

	registers[1][RegisterIndex].Index = 1;
	registers[1][RegisterIndex++].RegType = UINT16;

	registers[1][RegisterIndex].Index = 2;
	registers[1][RegisterIndex++].RegType = UINT32;

	registers[1][RegisterIndex].Index = 4;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Primary value

	registers[1][RegisterIndex].Index = 6;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //4-20mA value

	registers[1][RegisterIndex].Index = 8;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //% of calibrated range

	registers[1][RegisterIndex].Index = 10;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Secondary value

	return;
}


void LT600_FLX_SlaveRegisters(struct Measurement_Register *registers[2]){
	return;
}
