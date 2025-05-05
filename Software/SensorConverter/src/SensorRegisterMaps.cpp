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
		modbus_master->LinkRegisterMap(&CMC_MasterRegisterMap);
		modbus_slave->LinkRegisterMap(&CMC_SlaveRegisterMap);

		modbus_master->LoadRegisterMap();
		modbus_slave->LoadRegisterMap();

		modbus_slave->SettingsPtr->Sensor->RawData = &(modbus_master->RegisterMap[1][0].OutputData);

		break;
	case(TYPE_LT600):
		modbus_master->LinkRegisterMap(&LT600_MasterRegisterMap);
		modbus_slave->LinkRegisterMap(&LT600_SlaveRegisterMap);

		modbus_master->LoadRegisterMap();
		modbus_slave->LoadRegisterMap();

		modbus_slave->SettingsPtr->Sensor->RawData = &(modbus_master->RegisterMap[1][0].OutputData);
		break;
	case(TYPE_LT600_FLX):
		modbus_master->LinkRegisterMap(&LT600_MasterRegisterMap); //Still uses LT600 as a base but performs other calculations to approximate flow.
		modbus_slave->LinkRegisterMap(&LT600_FLX_SlaveRegisters);

		modbus_master->LoadRegisterMap();
		modbus_slave->LoadRegisterMap();

		modbus_slave->SettingsPtr->Sensor->RawData = &(modbus_master->RegisterMap[1][0].OutputData);
		break;

	}





	return;
}

void LoadModBusRegisters(class ModBusRTU_BaseClass *modbus_master, class ModBusRTU_BaseClass *modbus_slave, uint8_t sensor_type){

	switch(sensor_type){

	case(TYPE_CMC):
		modbus_slave->RegisterMap[MODBUS_DATA_REG][0].OutputData = modbus_master->RegisterMap[MODBUS_DATA_REG][2].OutputData;
		modbus_slave->RegisterMap[MODBUS_DATA_REG][1].InputData.UINT16 = (uint16_t)(modbus_master->RegisterMap[MODBUS_DATA_REG][3].OutputData * modbus_slave->RegisterMap[MODBUS_DATA_REG][1].ScaleFactor); // Temperature C
		modbus_slave->RegisterMap[MODBUS_DATA_REG][2].InputData.UINT16 = (uint16_t)(modbus_master->RegisterMap[MODBUS_DATA_REG][9].OutputData * modbus_slave->RegisterMap[MODBUS_DATA_REG][2].ScaleFactor); // Temperature C

		modbus_slave->RegisterMap[MODBUS_DATA_REG][3].OutputData = modbus_master->RegisterMap[MODBUS_DATA_REG][5].OutputData; //Raw
		break;
	case(TYPE_LT600):{

		modbus_slave->RegisterMap[MODBUS_DATA_REG][0].OutputData = modbus_master->RegisterMap[1][0].OutputData; //mH20
		modbus_slave->RegisterMap[MODBUS_DATA_REG][1].InputData.UINT16 = (uint16_t)(modbus_master->RegisterMap[1][2].OutputData * modbus_slave->RegisterMap[1][1].ScaleFactor); //Scale 0-100%
		modbus_slave->RegisterMap[MODBUS_DATA_REG][2].InputData.UINT16 = (uint16_t)(modbus_master->RegisterMap[1][3].OutputData * modbus_slave->RegisterMap[1][2].ScaleFactor); // Temperature C

		//Sensor TAG
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][4].InputData.UINT16 = ((uint16_t)modbus_slave->SettingsPtr->Tag[0] << 8 | modbus_slave->SettingsPtr->Tag[1]);
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][5].InputData.UINT16 = ((uint16_t)modbus_slave->SettingsPtr->Tag[2] << 8 | modbus_slave->SettingsPtr->Tag[3]);
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][6].InputData.UINT16 = ((uint16_t)modbus_slave->SettingsPtr->Tag[4] << 8 | modbus_slave->SettingsPtr->Tag[5]);
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][7].InputData.UINT16 = ((uint16_t)modbus_slave->SettingsPtr->Tag[6] << 8 | modbus_slave->SettingsPtr->Tag[7]);

		break;
	}
	case(TYPE_LT600_FLX):{

		class SensorFLX *Sensor = (class SensorFLX *)modbus_slave->SettingsPtr->Sensor;

		modbus_slave->RegisterMap[MODBUS_DATA_REG][0].OutputData = Sensor->CalculateMeasurement();
		modbus_slave->RegisterMap[MODBUS_DATA_REG][1].OutputData = modbus_master->RegisterMap[1][0].OutputData - Sensor->OffsetCal;
		modbus_slave->RegisterMap[MODBUS_DATA_REG][2].InputData.UINT16 = (uint16_t)(modbus_master->RegisterMap[1][3].OutputData * modbus_slave->RegisterMap[1][2].ScaleFactor); // Temperature C

		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][4].InputData.UINT16 = Sensor->Sill;
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][5].InputData.UINT16 = Sensor->Width;
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][6].InputData.UINT16 = 0;
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][7].InputData.UINT16 = 0;

		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][8].OutputData = Sensor->X1;
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][9].OutputData = Sensor->X2;
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][10].OutputData = Sensor->X3;

		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][11].OutputData = Sensor->OffsetCal;
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][12].InputData.UINT32 = Sensor->OffsetCalDate;

		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][13].OutputData = Sensor->LevelCal;
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][14].InputData.UINT32 = Sensor->LevelCalDate;

		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][15].InputData.UINT16 = Sensor->GutterType;


		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][16].InputData.UINT16 = ((uint16_t)modbus_slave->SettingsPtr->Tag[0] << 8 | modbus_slave->SettingsPtr->Tag[1]);
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][17].InputData.UINT16 = ((uint16_t)modbus_slave->SettingsPtr->Tag[2] << 8 | modbus_slave->SettingsPtr->Tag[3]);
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][18].InputData.UINT16 = ((uint16_t)modbus_slave->SettingsPtr->Tag[4] << 8 | modbus_slave->SettingsPtr->Tag[5]);
		modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][19].InputData.UINT16 = ((uint16_t)modbus_slave->SettingsPtr->Tag[6] << 8 | modbus_slave->SettingsPtr->Tag[7]);


		break;
	}
	default:
		return;


	}

	//These registers a required!
	modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][0].InputData.UINT16 = modbus_slave->SettingsPtr->SerialNumber_H;
	modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][1].InputData.UINT16 = modbus_slave->SettingsPtr->SerialNumber_L;
	modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][2].InputData.UINT16 = modbus_slave->SettingsPtr->SoftwareVersion;
	modbus_slave->RegisterMap[MODBUS_SETTINGS_REG][3].InputData.UINT16 = modbus_slave->Address;


	return;
}



void CMC_MasterRegisterMap(struct Measurement_Register *registers[2], uint16_t *register_map_size){


	//Allocate all the memory
	register_map_size[0] = CMC_HOLDING_MASTER_MAP_SIZE;
	register_map_size[1] = CMC_INPUT_MASTER_MAP_SIZE;

	free(registers[0]);
	registers[0] = (struct Measurement_Register *)malloc(register_map_size[0] * sizeof(struct Measurement_Register));

	free(registers[1]);
	registers[1] = (struct Measurement_Register *)malloc(register_map_size[1] * sizeof(struct Measurement_Register));

	uint8_t RegisterIndex{0};

	registers[1][RegisterIndex].Index = 0;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = UINT16; //Status bits

	registers[1][RegisterIndex].Index = 1;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = UINT16; //Alarm bits

	registers[1][RegisterIndex].Index = 2;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Consistency

	registers[1][RegisterIndex].Index = 4;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Water temperature

	registers[1][RegisterIndex].Index = 6;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Group delay

	registers[1][RegisterIndex].Index = 8;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Ndir signal

	registers[1][RegisterIndex].Index = 10;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //N signal

	registers[1][RegisterIndex].Index = 12;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //M signal

	registers[1][RegisterIndex].Index = 14;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Unfiltered Cs

	registers[1][RegisterIndex].Index = 16;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //PCB Temp

	registers[1][RegisterIndex].Index = 18;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Meas path phase

	registers[1][RegisterIndex].Index = 20;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Meas path true phase

	registers[1][RegisterIndex].Index = 22;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Meas path phase diff

	registers[1][RegisterIndex].Index = 24;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Meas path phase nonlinearity

	registers[1][RegisterIndex].Index = 26;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Meas path amplitude

	registers[1][RegisterIndex].Index = 28;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Meas path attenuation

	registers[1][RegisterIndex].Index = 30;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Ref line group delay

	registers[1][RegisterIndex].Index = 32;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Ref line phase

	registers[1][RegisterIndex].Index = 34;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Ref line true phase

	registers[1][RegisterIndex].Index = 36;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Ref line non linearity

	registers[1][RegisterIndex].Index = 38;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Ref line amplitude

	registers[1][RegisterIndex].Index = 40;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Ref line attenuation step correction


	//Holding registers
	RegisterIndex = 0;

	registers[0][RegisterIndex].Index = 0;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial H

	registers[0][RegisterIndex].Index = 1;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial L

	registers[0][RegisterIndex].Index = 2;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Software version

	registers[0][RegisterIndex].Index = 3;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //ModBus address

	registers[0][RegisterIndex].Index = 4;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Sensor type enum

	registers[0][RegisterIndex].Index = 5;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT32 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT32; //Boot count


	//Offset calibration
	registers[0][RegisterIndex].Index = 7;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT32 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT32; //Water offset calibration date

	registers[0][RegisterIndex].Index = 9;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = PONDUS_FLOAT; //Water offset calibration value

	//Gain calibration
	registers[0][RegisterIndex].Index = 11;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT32 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT32; //Gain calibration date

	registers[0][RegisterIndex].Index = 13;
	registers[0][RegisterIndex].ScaleFactor = 100;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Gain value

	registers[0][RegisterIndex].Index = 14;
	registers[0][RegisterIndex].ScaleFactor = 100;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Gain sample

	registers[0][RegisterIndex].Index = 15;
	registers[0][RegisterIndex].ScaleFactor = 100;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Lab sample

	registers[0][RegisterIndex].Index = 16;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Valid calibration flag

	registers[0][RegisterIndex].Index = 17;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Calibration flag

	registers[0][RegisterIndex].Index = 18;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = PONDUS_FLOAT; //Calibration result

	//TAG
	registers[0][RegisterIndex].Index = 20;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 21;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 22;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 23;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	return;
}


void CMC_SlaveRegisterMap(struct Measurement_Register *registers[2], uint16_t *register_map_size){


	//Allocate all the memory
	register_map_size[0] = CMC_HOLDING_SLAVE_MAP_SIZE;
	register_map_size[1] = CMC_INPUT_SLAVE_MAP_SIZE;

	free(registers[0]);
	registers[0] = (struct Measurement_Register *)malloc(register_map_size[0] * sizeof(struct Measurement_Register));

	free(registers[1]);
	registers[1] = (struct Measurement_Register *)malloc(register_map_size[1] * sizeof(struct Measurement_Register));

	uint8_t RegisterIndex{0};

	registers[1][RegisterIndex].Index = 0;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = FLOAT; //Consistency

	registers[1][RegisterIndex].Index = 2;
	registers[1][RegisterIndex].ScaleFactor = 10;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = UINT16; //Water temperature

	registers[1][RegisterIndex].Index = 3;
	registers[1][RegisterIndex].ScaleFactor = 10;
	registers[1][RegisterIndex].InputData.UINT16 = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = UINT16; //Electronic temperature

	registers[1][RegisterIndex].Index = 4;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = FLOAT; //RAW consistency

	//Holding registers
	RegisterIndex = 0;

	registers[0][RegisterIndex].Index = 0;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial H

	registers[0][RegisterIndex].Index = 1;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial L

	registers[0][RegisterIndex].Index = 2;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Software version

	registers[0][RegisterIndex].Index = 3;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //ModBus address

	//Offset calibration
	registers[0][RegisterIndex].Index = 4;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT32 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT32; //Water offset calibration date

	registers[0][RegisterIndex].Index = 6;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = FLOAT; //Water offset calibration value

	//Gain calibration
	registers[0][RegisterIndex].Index = 8;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT32 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT32; //Gain calibration date

	registers[0][RegisterIndex].Index = 10;
	registers[0][RegisterIndex].ScaleFactor = 100;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Gain value

	registers[0][RegisterIndex].Index = 11;
	registers[0][RegisterIndex].ScaleFactor = 100;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Gain sample

	registers[0][RegisterIndex].Index = 12;
	registers[0][RegisterIndex].ScaleFactor = 100;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Lab sample

	registers[0][RegisterIndex].Index = 13;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Valid calibration flag

	registers[0][RegisterIndex].Index = 14;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Calibration flag

	registers[0][RegisterIndex].Index = 15;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration result

	//TAG
	registers[0][RegisterIndex].Index = 17;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 18;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 19;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 20;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;



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
	registers[1][RegisterIndex].InputData.PONDUS_FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //Primary value

	registers[1][RegisterIndex].Index = 1006;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.PONDUS_FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //4-20mA value

	registers[1][RegisterIndex].Index = 1008;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.PONDUS_FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = PONDUS_FLOAT; //% of calibrated range

	registers[1][RegisterIndex].Index = 1010;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.PONDUS_FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
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
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = FLOAT; //Primary value

	registers[1][RegisterIndex].Index = 2;
	registers[1][RegisterIndex].ScaleFactor = 10;
	registers[1][RegisterIndex].InputData.UINT16 = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = UINT16; //% of calibrated range

	registers[1][RegisterIndex].Index = 3;
	registers[1][RegisterIndex].ScaleFactor = 10;
	registers[1][RegisterIndex].InputData.UINT16 = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = UINT16; //Secondary value

	//Holding registers
	RegisterIndex = 0;

	registers[0][RegisterIndex].Index = 0;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial H

	registers[0][RegisterIndex].Index = 1;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial L

	registers[0][RegisterIndex].Index = 2;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Software version

	registers[0][RegisterIndex].Index = 3;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //ModBus address

	registers[0][RegisterIndex].Index = 4;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 5;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 6;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 7;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;


	return;
}


void LT600_FLX_SlaveRegisters(struct Measurement_Register *registers[2], uint16_t *register_map_size){


	//Allocate all the memory
	register_map_size[0] = LT600_FLX_HOLDING_SLAVE_MAP_SIZE;
	register_map_size[1] = LT600_FLX_INPUT_SLAVE_MAP_SIZE;

	free(registers[0]);
	registers[0] = (struct Measurement_Register *)malloc(register_map_size[0] * sizeof(struct Measurement_Register));

	free(registers[1]);
	registers[1] = (struct Measurement_Register *)malloc(register_map_size[1] * sizeof(struct Measurement_Register));


	uint8_t RegisterIndex{0};

	registers[1][RegisterIndex].Index = 0;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = FLOAT; //Flow of water default m^3/h

	registers[1][RegisterIndex].Index = 2;
	registers[1][RegisterIndex].ScaleFactor = 1;
	registers[1][RegisterIndex].InputData.FLOAT = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = FLOAT; //Meter vattenpelare

	registers[1][RegisterIndex].Index = 3;
	registers[1][RegisterIndex].ScaleFactor = 10;
	registers[1][RegisterIndex].InputData.UINT16 = 0;
	registers[1][RegisterIndex].OutputData = 0;
	registers[1][RegisterIndex++].RegType = UINT16; //Temperature



	//Holding registers
	RegisterIndex = 0;

	registers[0][RegisterIndex].Index = 0;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial H

	registers[0][RegisterIndex].Index = 1;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Serial L

	registers[0][RegisterIndex].Index = 2;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Software version

	registers[0][RegisterIndex].Index = 3;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //ModBus address

	registers[0][RegisterIndex].Index = 4;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; // Height/sill or angle depending on gutter type

	registers[0][RegisterIndex].Index = 5;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Width inner

	registers[0][RegisterIndex].Index = 6;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Width outer

	registers[0][RegisterIndex].Index = 7;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16; //Throat

	registers[0][RegisterIndex].Index = 8;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration X1

	registers[0][RegisterIndex].Index = 10;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration X2

	registers[0][RegisterIndex].Index = 12;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration X3

	registers[0][RegisterIndex].Index = 14;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration offset

	registers[0][RegisterIndex].Index = 16;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT32 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT32; //Calibration offset date

	registers[0][RegisterIndex].Index = 18;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.FLOAT = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = FLOAT; //Calibration level

	registers[0][RegisterIndex].Index = 20;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT32 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT32; //Calibration level date


	//Type of gutter that is used, uses the index of the table.
	registers[0][RegisterIndex].Index = 22;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;


	//TAG
	registers[0][RegisterIndex].Index = 23;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 24;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 25;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;

	registers[0][RegisterIndex].Index = 26;
	registers[0][RegisterIndex].ScaleFactor = 1;
	registers[0][RegisterIndex].InputData.UINT16 = 0;
	registers[0][RegisterIndex].OutputData = 0;
	registers[0][RegisterIndex++].RegType = UINT16;



	return;
}














