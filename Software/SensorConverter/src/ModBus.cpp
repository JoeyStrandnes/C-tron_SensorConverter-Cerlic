/*
 * ModBus.cpp
 *
 *  Created on: Mar 12, 2025
 *      Author: Joey.Strandnes
 */



#include <ModBus.hpp>



/* =========================================================================
                    	BASE CLASS FOR MODBUS PARSING
   ========================================================================= */

void ModBusRTU_BaseClass::LoadRegisterMap(){


	//load the translation map for the specific sensor.
	this->MapRegisters(this->RegisterMap, this->RegisterMapSize);

	//Calculate the Size of the respective registers to hold all the data.
	for(uint8_t i = 0; i < 2; i++){

		this->RegisterSize[i] = 0;

		for(uint8_t k = 0; k < this->RegisterMapSize[i]; k++){

			switch(this->RegisterMap[i][k].RegType){

			case(CHAR):
			case(UINT8):
			case(INT16):
			case(UINT16):
			this->RegisterSize[i] += 1;
				break;
			case(UINT32):
			case(FLOAT):
			case(PONDUS_FLOAT):
			this->RegisterSize[i] += 2;
				break;

			default:
				break;

			}


		}

	}

	//Allocate all the memory
	free(this->Register[0]);
	this->Register[0] = (uint16_t *)malloc(this->RegisterSize[0] * 2);

	free(this->Register[1]);
	this->Register[1] = (uint16_t *)malloc(this->RegisterSize[1] * 2);


	return;
};




void ModBusRTU_BaseClass::BuildModBusException(uint8_t exeption){


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


uint16_t ModBusRTU_BaseClass::ModBusCRC(uint8_t *input_buffer, uint16_t const size){
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

/* =========================================================================
                    		MASTER CLASS FOR MODBUS
   ========================================================================= */
void ModBusRTU_MasterClass::ParseSlaveResponse(){
//Use this parser when we are slaves and want to parse a request from a master.


	if(this->Address != this->InputBuffer[0]){ //Address was not our address. Do not respond.
		this->OutputBufferSize = 0;
		std::memset(this->InputBuffer, 0, this->InputBufferSize);
		return;
	}

	/*//FIXME
	uint16_t CalculatedCRC = this->ModBusCRC(InputBuffer, (this->RequestSize - 2));
	uint16_t ReceivedCRC = ((uint16_t)(this->InputBuffer[this->RequestSize - 1]) << 8);
	ReceivedCRC |= this->InputBuffer[this->RequestSize - 2];

	//Check the CRC is valid.
	if(CalculatedCRC != ReceivedCRC){
		std::memset(this->InputBuffer, 0, this->InputBufferSize);
		return;
	}
	*/

	if(this->InputBuffer[1] == 3 || this->InputBuffer[1] ==  4){
		this->ParseSlaveData();
	}



	return;
}


void ModBusRTU_MasterClass::ReadAllSensorData(){


  this->OutputBuffer[0] = this->Address;
  this->OutputBuffer[1] = 0x04;
  this->OutputBuffer[2] = (this->RegisterMap[1][0].Index >> 8);
  this->OutputBuffer[3] = this->RegisterMap[1][0].Index;
  this->OutputBuffer[4] = (this->RegisterSize[1] >> 8);
  this->OutputBuffer[5] = this->RegisterSize[1];


	uint16_t MB_CRC = ModBusCRC(this->OutputBuffer, 6);

	//Add the CRC to the end of the packet.
	this->OutputBuffer[7] = (MB_CRC >> 8);
	this->OutputBuffer[6] = MB_CRC;

	this->ResponseSize = 8;

	return;
}

void ModBusRTU_MasterClass::ParseSlaveData(){
//This is the parser for when we are Masters and have sent a request to a slave.
//This function assumes that the input data is already sanity checked!
//It is only for parsing the data.

	uint8_t BufferIndex = MODBUS_REG_OFFSET;
	uint8_t RegisterType = this->InputBuffer[1] - 3;
	uint16_t NumberOfRegisters = this->RegisterSize[RegisterType];


	//Loop over all registers.
	for(uint16_t i = 0; i < NumberOfRegisters; i++){

		switch(this->RegisterMap[RegisterType][i].RegType){

			case(CHAR): // 16-bit wide but easier to keep track of.
				//(this->RegisterMap[RegisterType][i].Index * 2) + MODBUS_REG_OFFSET; // 2x is to convert the transmitted bytes to the 16-bit wide registers.
				this->RegisterMap[RegisterType][i].InputData.UINT16 = (((int16_t)InputBuffer[BufferIndex++]) << 8);
				this->RegisterMap[RegisterType][i].InputData.UINT16 |= InputBuffer[BufferIndex];

				BufferIndex += 2;

				break;
			case(INT16):
				//BufferIndex = (this->RegisterMap[RegisterType][i].Index * 2) + MODBUS_REG_OFFSET; // 2x is to convert the transmitted bytes to the 16-bit wide registers.
				this->RegisterMap[RegisterType][i].InputData.INT16 = (((int16_t)InputBuffer[BufferIndex++]) << 8);
				this->RegisterMap[RegisterType][i].InputData.INT16 |= InputBuffer[BufferIndex];
				this->RegisterMap[RegisterType][i].OutputData = (((float)(this->RegisterMap[RegisterType][i].InputData.INT16)) / this->RegisterMap[RegisterType][i].ScaleFactor);

				BufferIndex += 2;

				break;
			case(UINT16):
				//BufferIndex = (this->RegisterMap[RegisterType][i].Index * 2) + MODBUS_REG_OFFSET;
				this->RegisterMap[RegisterType][i].InputData.UINT16 = (((uint16_t)InputBuffer[BufferIndex++]) << 8);
				this->RegisterMap[RegisterType][i].InputData.UINT16 |= InputBuffer[BufferIndex];
				this->RegisterMap[RegisterType][i].OutputData = (((float)(this->RegisterMap[RegisterType][i].InputData.UINT16)) / this->RegisterMap[RegisterType][i].ScaleFactor);

				BufferIndex += 2;
				break;
			case(UINT32):
				//BufferIndex = (this->RegisterMap[RegisterType][i].Index * 2) + MODBUS_REG_OFFSET;
				this->RegisterMap[RegisterType][i].InputData.UINT32 = (((uint32_t)InputBuffer[BufferIndex++]) << 24);
				this->RegisterMap[RegisterType][i].InputData.UINT32 |= (((uint32_t)InputBuffer[BufferIndex++]) << 16);
				this->RegisterMap[RegisterType][i].InputData.UINT32 |= (((uint32_t)InputBuffer[BufferIndex++]) << 8);
				this->RegisterMap[RegisterType][i].InputData.UINT32 |= InputBuffer[BufferIndex];
				this->RegisterMap[RegisterType][i].OutputData = (((float)(this->RegisterMap[RegisterType][i].InputData.UINT32)) / this->RegisterMap[RegisterType][i].ScaleFactor);

				BufferIndex += 4;

				break;
			case(FLOAT): {
				uint8_t TmpBuffer[4];
				float TmpFloat = 0;
				//BufferIndex = (this->RegisterMap[RegisterType][i].Index * 2) + MODBUS_REG_OFFSET;
				TmpBuffer[3] = InputBuffer[BufferIndex++];
				TmpBuffer[2] = InputBuffer[BufferIndex++];
				TmpBuffer[1] = InputBuffer[BufferIndex++];
				TmpBuffer[0] = InputBuffer[BufferIndex];
				memcpy(&TmpFloat, &TmpBuffer, 4);
				this->RegisterMap[RegisterType][i].InputData.FLOAT = TmpFloat;
				this->RegisterMap[RegisterType][i].OutputData = TmpFloat;

				BufferIndex += 4;

				break;
			}
			case(PONDUS_FLOAT): {
				uint8_t TmpBuffer[4];
				float TmpFloat = 0;
				//BufferIndex = (this->RegisterMap[RegisterType][i].Index * 2) + MODBUS_REG_OFFSET;
				TmpBuffer[1] = InputBuffer[BufferIndex++];
				TmpBuffer[0] = InputBuffer[BufferIndex++];
				TmpBuffer[3] = InputBuffer[BufferIndex++];
				TmpBuffer[2] = InputBuffer[BufferIndex];
				memcpy(&TmpFloat, &TmpBuffer, 4);
				this->RegisterMap[RegisterType][i].InputData.FLOAT = TmpFloat;
				this->RegisterMap[RegisterType][i].OutputData = TmpFloat;

				BufferIndex += 4;
				break;
			}
			default:
				break;

		}

	}


	return;
}


/* =========================================================================
                    		SLAVE CLASS FOR MODBUS
   ========================================================================= */
void ModBusRTU_SlaveClass::ParseMasterRequest(){
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

void ModBusRTU_SlaveClass::SetRegisterValue(float data, uint16_t index){


	this->RegisterMap[1][index].OutputData = data;


	return;
}


void ModBusRTU_SlaveClass::FormatRegisterData(){
//Loop over the holding and input registers and

	uint16_t RegisterIndex = 0;

	for(uint8_t i = 0; i < 2; i++){

		RegisterIndex = 0;

		for(uint16_t k = 0; k < this->RegisterMapSize[i]; k++){

			switch(this->RegisterMap[i][k].RegType){


				case(CHAR):
				case(UINT8):
				case(UINT16):
					Register[i][RegisterIndex++] = this->RegisterMap[i][k].InputData.UINT16;
					break;
				case(INT16):
					Register[i][RegisterIndex++] = this->RegisterMap[i][k].InputData.INT16;
					break;
				case(UINT32):
					Register[i][RegisterIndex++] = (this->RegisterMap[i][k].InputData.UINT32 >> 16);
					Register[i][RegisterIndex++] = this->RegisterMap[i][k].InputData.UINT32;
					break;
				case(FLOAT):
				case(PONDUS_FLOAT):{
					uint16_t *Float_ptr;
					Float_ptr = (uint16_t *)&(this->RegisterMap[i][k].OutputData);
					Register[i][RegisterIndex++] = *(Float_ptr + 1);
					Register[i][RegisterIndex++] = *Float_ptr;
					break;
				}



			}



		}



	}




	return;
}


void ModBusRTU_SlaveClass::HandleFC_3_4(){


    //Check if the requested data is within space.
    uint16_t FirstRegister = ((uint16_t)(this->InputBuffer[2]) << 8 | this->InputBuffer[3]);
    uint16_t RequestedRegisters = ((uint16_t)(this->InputBuffer[4]) << 8 | this->InputBuffer[5]);
    uint16_t TotalRegisters = (FirstRegister + RequestedRegisters);

    uint8_t RegisterType = (this->InputBuffer[1] - 3);


    //Everything is fine!
    //Start of with parsing all the data and place it in the 16-bit ModBus registers.
    this->FormatRegisterData();


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


void ModBusRTU_SlaveClass::HandleFC_6(){

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


uint8_t ModBusRTU_SlaveClass::ModBusSerialNumber(){

	return MODBUS_EXCEPTION_OK;
}

uint8_t ModBusRTU_SlaveClass::ModBusCalibration(){

	return MODBUS_EXCEPTION_OK;
}

uint8_t ModBusRTU_SlaveClass::ModBusFactoryDefaults(){

	return MODBUS_EXCEPTION_OK;
}

uint8_t ModBusRTU_SlaveClass::ModBusSensorTag(){

	return MODBUS_EXCEPTION_OK;
}

uint8_t ModBusRTU_SlaveClass::ModBusStoreToNVM(){

	return MODBUS_EXCEPTION_OK;
}














