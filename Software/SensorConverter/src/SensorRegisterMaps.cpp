/*
 * SensorConfig.cpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */


#include <SensorRegisterMaps.hpp>

//void LinkSensorConfig(class SensorConverterSettings *settings){
void LinkSensorConfig(class ModBusRTU_BaseClass *modbus_master, class ModBusRTU_BaseClass *modbus_slave, uint8_t sensor_type){

	switch(sensor_type){

	case(TYPE_CMC):
			/*
		modbus->ReloadMasterRegisters = &CMC_MasterRegisters;
		settings->ReloadSlaveRegisters = &CMC_SlaveRegisters;
		*/
		break;
	case(TYPE_LT600):
		modbus_master->LinkRegisterMap(&LT600_MasterRegisterMap);
		modbus_slave->LinkRegisterMap(&LT600_SlaveRegisterMap);
		break;
	case(TYPE_LT600_FLX):
		modbus_master->LinkRegisterMap(&LT600_MasterRegisterMap); //Still uses LT600 as a base but performs other calculations to approximate flow.
		modbus_slave->LinkRegisterMap(&LT600_FLX_SlaveRegisters);
		break;


	}


	modbus_master->LoadRegisterMap();
	modbus_slave->LoadRegisterMap();

	return;
}

void LoadModBusRegisters(class ModBusRTU_BaseClass *modbus_master, class ModBusRTU_BaseClass *modbus_slave, uint8_t sensor_type){

	switch(sensor_type){

	case(TYPE_CMC):

		break;
	case(TYPE_LT600):{

		modbus_slave->RegisterMap[1][0].OutputData = modbus_master->RegisterMap[1][0].OutputData; //mH20
		modbus_slave->RegisterMap[1][1].InputData.UINT16 = (uint16_t)(modbus_master->RegisterMap[1][2].OutputData * modbus_slave->RegisterMap[1][1].ScaleFactor); //Scale 0-100%
		modbus_slave->RegisterMap[1][2].InputData.UINT16 = (uint16_t)(modbus_master->RegisterMap[1][3].OutputData * modbus_slave->RegisterMap[1][2].ScaleFactor); // Temperature C

		//Sensor TAG
		uint16_t *Tag_ptr = (uint16_t*)(modbus_slave->SettingsPtr->Tag);
		modbus_slave->RegisterMap[0][4].InputData.UINT16 = Tag_ptr[0];
		modbus_slave->RegisterMap[0][5].InputData.UINT16 = Tag_ptr[1];
		modbus_slave->RegisterMap[0][6].InputData.UINT16 = Tag_ptr[2];
		modbus_slave->RegisterMap[0][7].InputData.UINT16 = Tag_ptr[3];

		break;
	}
	case(TYPE_LT600_FLX):{

		modbus_slave->RegisterMap[1][0].OutputData = FLX_CalculateFlow(modbus_master->RegisterMap[1][0].OutputData, 0); //FIXME Parshalls gutter for testing
		modbus_slave->RegisterMap[1][2].InputData.UINT16 = (uint16_t)(modbus_master->RegisterMap[1][3].OutputData * modbus_slave->RegisterMap[1][2].ScaleFactor); // Temperature C

		modbus_slave->RegisterMap[1][3].InputData.UINT16 = 0;
		modbus_slave->RegisterMap[1][4].InputData.UINT16 = 0;

		char TmpBuffer[12];
		FLX_GetGutterName(0, TmpBuffer, 12);
		modbus_slave->RegisterMap[0][10].InputData.UINT16 = TmpBuffer[0];
		modbus_slave->RegisterMap[0][11].InputData.UINT16 = TmpBuffer[1];
		modbus_slave->RegisterMap[0][12].InputData.UINT16 = TmpBuffer[2];
		modbus_slave->RegisterMap[0][13].InputData.UINT16 = TmpBuffer[3];
		modbus_slave->RegisterMap[0][14].InputData.UINT16 = TmpBuffer[4];
		modbus_slave->RegisterMap[0][15].InputData.UINT16 = TmpBuffer[5];


		uint16_t *Tag_ptr = (uint16_t*)(modbus_slave->SettingsPtr->Tag);
		modbus_slave->RegisterMap[0][16].InputData.UINT16 = Tag_ptr[0];
		modbus_slave->RegisterMap[0][17].InputData.UINT16 = Tag_ptr[1];
		modbus_slave->RegisterMap[0][18].InputData.UINT16 = Tag_ptr[2];
		modbus_slave->RegisterMap[0][19].InputData.UINT16 = Tag_ptr[3];

		break;
	}
	default:
		return;


	}


	modbus_slave->RegisterMap[0][0].InputData.UINT16 = modbus_slave->SettingsPtr->SerialNumber_H;
	modbus_slave->RegisterMap[0][1].InputData.UINT16 = modbus_slave->SettingsPtr->SerialNumber_L;
	modbus_slave->RegisterMap[0][2].InputData.UINT16 = modbus_slave->SettingsPtr->SoftwareVersion;
	modbus_slave->RegisterMap[0][3].InputData.UINT16 = modbus_slave->Address;


	return;
}



void CMC_MasterRegisters(struct Measurement_Register *registers[2]){
	return;
}


void CMC_SlaveRegisters(struct Measurement_Register *registers[2]){
	return;
}


void LT600_MasterRegisterMap(struct Measurement_Register *registers[2], uint16_t *register_map_size){


	//Allocate all the memory
	register_map_size[0] = LT600_HOLDING_MASTER_MAP_SIZE;
	register_map_size[1] = LT600_INPUT_MASTER_MAP_SIZE;

	free(registers[1]);
	registers[1] = (struct Measurement_Register *)malloc(register_map_size[1] * sizeof(struct Measurement_Register));


	uint8_t RegisterIndex{0};

	registers[1][RegisterIndex].Index = 1004;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Primary value

	registers[1][RegisterIndex].Index = 1006;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //4-20mA value

	registers[1][RegisterIndex].Index = 1008;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //% of calibrated range

	registers[1][RegisterIndex].Index = 1010;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Secondary value


	return;
}


void LT600_SlaveRegisterMap(struct Measurement_Register *registers[2], uint16_t *register_map_size){

	//Allocate all the memory
	register_map_size[0] = LT600_HOLDING_SLAVE_MAP_SIZE;
	register_map_size[1] = LT600_INPUT_SLAVE_MAP_SIZE;

	free(registers[0]);
	registers[0] = (struct Measurement_Register *)malloc(register_map_size[0] * sizeof(struct Measurement_Register));

	free(registers[1]);
	registers[1] = (struct Measurement_Register *)malloc(register_map_size[1] * sizeof(struct Measurement_Register));

	uint8_t RegisterIndex{0};

	registers[1][RegisterIndex].Index = 0;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex++].RegType = FLOAT; //Primary value

	registers[1][RegisterIndex].Index = 2;
	registers[1][RegisterIndex].ScaleFactor = 10;
	registers[1][RegisterIndex++].RegType = UINT16; //% of calibrated range

	registers[1][RegisterIndex].Index = 3;
	registers[1][RegisterIndex].ScaleFactor = 10;
	registers[1][RegisterIndex++].RegType = UINT16; //Secondary value

	//Holding registers
	RegisterIndex = 0;

	registers[0][RegisterIndex].Index = 0;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial H

	registers[0][RegisterIndex].Index = 1;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial L

	registers[0][RegisterIndex].Index = 2;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Software version

	registers[0][RegisterIndex].Index = 3;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //ModBus address

	registers[0][RegisterIndex].Index = 4;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;

	registers[0][RegisterIndex].Index = 5;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;

	registers[0][RegisterIndex].Index = 6;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;

	registers[0][RegisterIndex].Index = 7;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;


	return;
}


void LT600_FLX_SlaveRegisters(struct Measurement_Register *registers[2], uint16_t *register_map_size){


	//Allocate all the memory
	register_map_size[0] = LT600_HOLDING_SLAVE_MAP_SIZE;
	register_map_size[1] = LT600_INPUT_SLAVE_MAP_SIZE;

	free(registers[0]);
	registers[0] = (struct Measurement_Register *)malloc(register_map_size[0] * sizeof(struct Measurement_Register));

	free(registers[1]);
	registers[1] = (struct Measurement_Register *)malloc(register_map_size[1] * sizeof(struct Measurement_Register));


	uint8_t RegisterIndex{0};

	registers[1][RegisterIndex].Index = 0;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex++].RegType = FLOAT; //Flow of water default m^3/h

	registers[1][RegisterIndex].Index = 2;
	registers[1][RegisterIndex].ScaleFactor = 10;
	registers[1][RegisterIndex++].RegType = UINT16; //Temperature

	registers[1][RegisterIndex].Index = 3;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex++].RegType = FLOAT; //Total flow

	registers[1][RegisterIndex].Index = 5;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex++].RegType = UINT16; //Antal bräddningar


	//Holding registers
	RegisterIndex = 0;

	registers[0][RegisterIndex].Index = 0;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial H

	registers[0][RegisterIndex].Index = 1;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial L

	registers[0][RegisterIndex].Index = 2;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Software version

	registers[0][RegisterIndex].Index = 3;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //ModBus address

	registers[0][RegisterIndex].Index = 4;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; // Height/sill or angle depending on gutter type

	registers[0][RegisterIndex].Index = 5;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Width inner

	registers[0][RegisterIndex].Index = 6;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Width outer

	registers[0][RegisterIndex].Index = 7;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT16; //Throat

	registers[0][RegisterIndex].Index = 8;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration X1

	registers[0][RegisterIndex].Index = 10;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration X2

	registers[0][RegisterIndex].Index = 12;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration X3

	registers[0][RegisterIndex].Index = 14;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration offset

	registers[0][RegisterIndex].Index = 16;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT32; //Calibration offset date

	registers[0][RegisterIndex].Index = 18;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration level

	registers[0][RegisterIndex].Index = 20;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = UINT32; //Calibration level date


	//Type of gutter that is used, uses two bytes to represent it.
	//Parshall 		= PA
	//Thomson 		= TH
	//Rektangular 	= RE
	//RSK 			= RS
	//Palmer-Bowlus = PB
	//Cipoletti 	= CI
	//Sutro 		= SU
	//Venturi 		= VE
	//Venturi U		= VU
	registers[0][RegisterIndex].Index = 22;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;


	//TAG
	registers[0][RegisterIndex].Index = 23;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;

	registers[0][RegisterIndex].Index = 24;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;

	registers[0][RegisterIndex].Index = 25;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;

	registers[0][RegisterIndex].Index = 26;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex++].RegType = CHAR;



	return;
}














