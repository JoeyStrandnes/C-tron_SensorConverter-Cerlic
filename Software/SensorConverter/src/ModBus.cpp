/*
 * ModBus.cpp
 *
 *  Created on: Mar 12, 2025
 *      Author: Joey.Strandnes
 */



#include <ModBus.hpp>






void ModBusRTU_Class::ParseModBusRTUPacket(){


	if(this->isMaster){
		this->ParseSlavePacket();
	}
	else{
		this->ParseMasterPacket();
	}

	return;
}

void ModBusRTU_Class::ParseMasterPacket(){
//Use this parser when we are slaves and want to parse a request from a master.


	if(this->Address != this->InputBuffer[0]){ //Address was not our address. Do not respond.
		this->OutputBufferSize = 0;
		std::memset(this->InputBuffer, 0, this->InputBufferSize);
		return;
	}

	uint16_t CalculatedCRC = this->ModBusCRC(InputBuffer, (this->RequestSize - 2));
	uint16_t ReceivedCRC = ((uint16_t)(this->InputBuffer[this->RequestSize - 1]) << 8);
	ReceivedCRC |= this->InputBuffer[this->RequestSize - 2];

    //Check the CRC is valid.
    if(CalculatedCRC != ReceivedCRC){
        this->BuildModBusException(MODBUS_EXCEPTION_MEMORY_ERROR);
        return;
    }

    //Request is for me but unsupported command. Hard coded during testing, FIXME.
    switch(this->InputBuffer[1]){

        case(3):
        case(4):
            HandleFC_3_4();
            return;
        case(6): //The only register that is changeable is register 3 (ModBus address)
            HandleFC_6();
            return;
        case(65): //Serial Number input
            BuildModBusException(ModBusSerialNumber());
            return;
        case(66): //Calibration.
            BuildModBusException(ModBusCalibration());
            return;
        case(67): //Reset settings
            BuildModBusException(ModBusFactoryDefaults());
            return;
        case(68): //Sensor tag
            BuildModBusException(ModBusSensorTag());
            return;
        case(69):
            return BuildModBusException(ModBusStoreToNVM());
        default:
            BuildModBusException(MODBUS_EXCEPTION_ILLIGAL_FUNCTION);
            return;

    }


	return;
}


void ModBusRTU_Class::ParseSlavePacket(){
//This is the parser for when we are Masters and have sent a request to a slave.








	return;
}


void ModBusRTU_Class::HandleFC_3_4(){


    //Check if the requested data is within space.
    uint16_t FirstRegister = ((uint16_t)(this->InputBuffer[2]) << 8 | this->InputBuffer[3]);
    uint16_t RequestedRegisters = ((uint16_t)(this->InputBuffer[4]) << 8 | this->InputBuffer[5]);
    uint16_t TotalRegisters = (FirstRegister + RequestedRegisters);

    uint8_t RegisterType = (this->InputBuffer[1] - 3);


    //Everything is fine!
    this->OutputBuffer[0] = this->Address;
    this->OutputBuffer[1] = this->InputBuffer[1];
    this->OutputBuffer[2] = (uint8_t)(RequestedRegisters * 2);

    this->ResponseSize = 3;

    for(uint8_t i = FirstRegister; i < TotalRegisters; i++){
    	this->OutputBuffer[this->ResponseSize++] = (this->Register[RegisterType][i] >> 8);
    	this->OutputBuffer[this->ResponseSize++] = this->Register[RegisterType][i];
    }

    uint16_t CalculatedCRC = ModBusCRC(this->OutputBuffer, this->ResponseSize);

    this->OutputBuffer[this->ResponseSize++] = CalculatedCRC;
    this->OutputBuffer[this->ResponseSize++] = (CalculatedCRC >> 8);


	return;
}

void ModBusRTU_Class::HandleFC_6(){

    //Dont like the hard coded nature of this but practically zero other sensors need a register write other than address change.
    if(this->InputBuffer[3] != 3){ //Not the ModBus address register
        BuildModBusException(MODBUS_EXCEPTION_ILLIGAL_DATA_ADDRESS);
        return;
    }

    //Zero is an illigal address for ModBus.
    if(this->InputBuffer[5] == 0){
        BuildModBusException(MODBUS_EXCEPTION_ILLIGAL_DATA_VALUE);
        return;
    }

    //"Build" the response with the old address and confirm the change. The compiler has issues with memcpy...
    for(uint8_t i = 0; i < this->RequestSize; i++){
    	this->OutputBuffer[i] = this->InputBuffer[i];
    }

    this->ResponseSize = this->RequestSize;

    //Change the address
    this->Address = this->InputBuffer[5];
    this->Register[0][3] = this->InputBuffer[5];

    //StoreSettingsToEEPROM((modbus_rtu->SettingsPtr));


	return;
}

uint8_t ModBusRTU_Class::ModBusSerialNumber(){

	return MODBUS_EXCEPTION_OK;
}

uint8_t ModBusRTU_Class::ModBusCalibration(){

	return MODBUS_EXCEPTION_OK;
}

uint8_t ModBusRTU_Class::ModBusFactoryDefaults(){

	return MODBUS_EXCEPTION_OK;
}

uint8_t ModBusRTU_Class::ModBusSensorTag(){

	return MODBUS_EXCEPTION_OK;
}

uint8_t ModBusRTU_Class::ModBusStoreToNVM(){

	return MODBUS_EXCEPTION_OK;
}



void ModBusRTU_Class::BuildModBusException(uint8_t exeption){


    this->OutputBuffer[0] = this->Address;
	this->OutputBuffer[1] = this->InputBuffer[1];
	this->OutputBuffer[1] |= (1 << 7);
	this->OutputBuffer[2] = exeption;

	uint16_t CalculatedCRC = ModBusCRC((this->OutputBuffer), 3);

	this->OutputBuffer[4] = (CalculatedCRC >> 8);
	this->OutputBuffer[3] = (uint8_t)CalculatedCRC;

    this->ResponseSize = 5;

    return;
}


uint16_t ModBusRTU_Class::ModBusCRC(uint8_t *input_buffer, uint16_t const size){
//Simply calculates the CRC of a ModBus RTU packet.

	uint16_t MB_CRC = 0xFFFF;


	for(uint16_t i = 0; i < size; i++){

		MB_CRC ^= (uint16_t)(input_buffer[i]);

		for(uint16_t j = 8; j != 0; j--){

			if((MB_CRC & 0x0001) != 0){

				MB_CRC >>= 1;
				MB_CRC ^= 0xA001;

			}
			else{
				MB_CRC >>= 1;
			}

		}


	}

	return MB_CRC;

}

/*
void ParseModBusRTUPacket(struct SENSOR_STRUCT *sensor, uint8_t *input_buffer, uint8_t register_type){
//The reason this method is used over the built ModBus data parser is that it is easier to handle different data types since it only supports words.
//Default ModBus register size is 16-bit but two registers can be used to get larger data type like floats.


	//Don't parse data if the sensor is being cleaned. This will be used for diagnosis in the future, hence why the task is simply not halted.
	if((sensor->Cleaning == true) && register_type == MODBUS_DATA_REG){
		return;
	}

	uint8_t BufferIndex = 0;
	uint8_t NumberOfRegisters = 0;

	if(register_type == 0){
		NumberOfRegisters = sensor->SettingsRegisters;
	}
	else{
		NumberOfRegisters = sensor->DataRegisters;
	}


	//Loop over all registers.
	for(uint8_t i = 0; i < NumberOfRegisters; i++){

		float TempData;

		switch(sensor->Registers[register_type][i].RegType){

			case(CHAR): // 16-bit wide but easier to keep track of.
				BufferIndex = (sensor->Registers[register_type][i].Index * 2) + MODBUS_REG_OFFSET; // 2x is to convert the transmitted bytes to the 16-bit wide registers.
				sensor->Registers[register_type][i].InputData.UINT16 = (((int16_t)input_buffer[BufferIndex++]) << 8);
				sensor->Registers[register_type][i].InputData.UINT16 |= input_buffer[BufferIndex];
				break;
			case(INT16):
				BufferIndex = (sensor->Registers[register_type][i].Index * 2) + MODBUS_REG_OFFSET; // 2x is to convert the transmitted bytes to the 16-bit wide registers.
				sensor->Registers[register_type][i].InputData.INT16 = (((int16_t)input_buffer[BufferIndex++]) << 8);
				sensor->Registers[register_type][i].InputData.INT16 |= input_buffer[BufferIndex];
				TempData = (((float)(sensor->Registers[register_type][i].InputData.INT16)) / sensor->Registers[register_type][i].ScaleFactor);
				break;
			case(UINT16):
				BufferIndex = (sensor->Registers[register_type][i].Index * 2) + MODBUS_REG_OFFSET;
				sensor->Registers[register_type][i].InputData.UINT16 = (((uint16_t)input_buffer[BufferIndex++]) << 8);
				sensor->Registers[register_type][i].InputData.UINT16 |= input_buffer[BufferIndex];
				TempData = (((float)(sensor->Registers[register_type][i].InputData.UINT16)) / sensor->Registers[register_type][i].ScaleFactor);
				break;
			case(UINT32):
				BufferIndex = (sensor->Registers[register_type][i].Index * 2) + MODBUS_REG_OFFSET;
				sensor->Registers[register_type][i].InputData.UINT32 = (((uint32_t)input_buffer[BufferIndex++]) << 24);
				sensor->Registers[register_type][i].InputData.UINT32 |= (((uint32_t)input_buffer[BufferIndex++]) << 16);
				sensor->Registers[register_type][i].InputData.UINT32 |= (((uint32_t)input_buffer[BufferIndex++]) << 8);
				sensor->Registers[register_type][i].InputData.UINT32 |= input_buffer[BufferIndex];
				TempData = (((float)(sensor->Registers[register_type][i].InputData.UINT32)) / sensor->Registers[register_type][i].ScaleFactor);
				break;
			case(FLOAT): {
				uint8_t TmpBuffer[4];
				float TmpFloat = 0;
				BufferIndex = (sensor->Registers[register_type][i].Index * 2) + MODBUS_REG_OFFSET;
				TmpBuffer[3] = input_buffer[BufferIndex++];
				TmpBuffer[2] = input_buffer[BufferIndex++];
				TmpBuffer[1] = input_buffer[BufferIndex++];
				TmpBuffer[0] = input_buffer[BufferIndex];
				memcpy(&TmpFloat, &TmpBuffer, 4);
				sensor->Registers[register_type][i].InputData.FLOAT = TmpFloat;
				TempData = TmpFloat;
				break;
			}

			default:
				break;

		}


		//Convert to whatever unit is selected by the end user.
		sensor->Registers[register_type][i].ConvertUnit();
		TempData = sensor->Registers[register_type][i].ConvertUnit(TempData); //Don't forget to convert the input data.

		//TODO Basic filtering, will improve later.
		if(register_type == MODBUS_DATA_REG){ //Don't filter settings :)

			//Filter the measurement data
			sensor->Registers[register_type][i].OutputData = (sensor->Registers[register_type][i].OutputData == 0) ? TempData:((sensor->Registers[register_type][i].OutputData * (sensor->FilterStrength - 1) + TempData) / sensor->FilterStrength);

			//Add the new measurement to the data series.
			if(i == sensor->PrimaryMeasurementIndex){

				//Used for the trend data and graphing.
				if(sensor->PrimaryDataSeries != NULL){


					//Prime the accumulator
					if(sensor->DataSeriesAccumulatorCounter == 0){
						sensor->DataSeriesAccumulator = sensor->Registers[1][sensor->PrimaryMeasurementIndex].OutputData;
						sensor->DataSeriesAccumulatorCounter++;
					}
					//Store the sample every minute.
					else if(sensor->DataSeriesAccumulatorCounter == PRIMARY_DATA_ACCUMULATION){


						//Shift the data back one element and append the new value. Could use a circular buffer...
						if(sensor->PrimaryDataSeriesIndex >= PRIMARY_DATA_POINTS){
							std::memmove(sensor->PrimaryDataSeries, sensor->PrimaryDataSeries + 1, sizeof(*sensor->PrimaryDataSeries) * PRIMARY_DATA_POINTS - 1);
							sensor->PrimaryDataSeries[sensor->PrimaryDataSeriesIndex - 1] = (sensor->DataSeriesAccumulator/PRIMARY_DATA_ACCUMULATION);
						}
						else{
							sensor->PrimaryDataSeries[sensor->PrimaryDataSeriesIndex++] = (sensor->DataSeriesAccumulator/PRIMARY_DATA_ACCUMULATION);
						}

						//Reset the accumulator
						sensor->DataSeriesAccumulatorCounter = 0;

					}
					else{
						sensor->DataSeriesAccumulator += sensor->Registers[1][sensor->PrimaryMeasurementIndex].OutputData;
						sensor->DataSeriesAccumulatorCounter++;
					}



				}

			}

		}
		else if(register_type == MODBUS_SETTINGS_REG){

			//TODO Will parse in a better way, TBD
			sensor->Registers[register_type][i].OutputData = TempData;
		}


	}

	//Fill up the sensor class with fresh data.
	if(register_type == MODBUS_SETTINGS_REG){
		if(sensor->SerialNumber_H == 0 || sensor->SerialNumber_H == 0xFF){
			sensor->SerialNumber_H =  sensor->Registers[register_type][0].InputData.UINT16;
		}
		if(sensor->SerialNumber_L == 0 || sensor->SerialNumber_L == 0xFF){
			sensor->SerialNumber_L =  sensor->Registers[register_type][1].InputData.UINT16;
		}
		if(sensor->SoftwareVersion == 0){
			sensor->SoftwareVersion =  sensor->Registers[register_type][2].InputData.UINT16;
		}

		if(sensor->SensorTagRegisterIndex != 0){ //Avoid adding tag string for sensors without that implementation.
			uint8_t StringIndex = 0;

			for(uint8_t i = 0; i < 4; i++){
				sensor->SensorTag[StringIndex++] = (sensor->Registers[register_type][sensor->SensorTagRegisterIndex + i].InputData.UINT16 >> 8);
				sensor->SensorTag[StringIndex++] = sensor->Registers[register_type][sensor->SensorTagRegisterIndex + i].InputData.UINT16;

			}

			if(sensor->SensorTag[0] != '\0'){ //Check if the tag is set.
				sensor->UseSensorTag = true;
			}
		}


	}



	return;
}
*/







