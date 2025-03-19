/*
 * ModBus.hpp
 *
 *  Created on: Mar 12, 2025
 *      Author: Joey.Strandnes
 */

#ifndef INC_MODBUS_HPP_
#define INC_MODBUS_HPP_

#include "main.h"
#include <SensorRegisterMaps.hpp>
#include <SensorConverter.hpp>

#define MODBUS_REG_OFFSET 3
#define MODBUS_SETTINGS_REG 0
#define MODBUS_DATA_REG 1

//Exception codes
#define MODBUS_EXCEPTION_OK                     0
#define MODBUS_EXCEPTION_ILLIGAL_FUNCTION       1
#define MODBUS_EXCEPTION_ILLIGAL_DATA_ADDRESS   2
#define MODBUS_EXCEPTION_ILLIGAL_DATA_VALUE     3
#define MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE   4
#define MODBUS_EXCEPTION_ACK                    5
#define MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY      6
#define MODBUS_EXCEPTION_MEMORY_ERROR           8

enum SerialNumberCommands{SER_High, SER_Low, SER_StoreToNVM, SER_Size}; //Enum for Serial-number

class ModBusRTU_BaseClass{
//This base class is used as a pure parser of the data.
//The ModBus Master/Slave classes reformat all the data for the specific sensor.

public:

	ModBusRTU_BaseClass(){};

	class SensorConverterSettings *SettingsPtr{NULL};

	uint8_t Address{0};

    uint8_t *OutputBuffer;          //The TX buffer
    uint16_t OutputBufferSize;      //Size of TX buffer

    uint8_t *InputBuffer;           //The RX buffer
    uint16_t InputBufferSize;       //Size of the RX buffer

    uint16_t ResponseSize;          //How many bytes to transmit
    uint16_t RequestSize;           //How many bytes were received

    void LinkRegisterMap(void (*map_registers)(struct Measurement_Register *registers[2], uint16_t *register_map_size)){this->MapRegisters = map_registers;};
    void LoadRegisterMap();

    void BuildModBusException(uint8_t exeption);
    uint16_t ModBusCRC(uint8_t *input_buffer, uint16_t const size);

    //This is for the data format translation database, not actual ModBus register data.
    void (*MapRegisters)(struct Measurement_Register *registers[2], uint16_t *register_map_size) = NULL;
	struct Measurement_Register *RegisterMap[2] = {NULL,NULL};
	uint16_t RegisterMapSize[2] = {0,0};

    //Registers visible through the ModBus interface. Holding registers and input registers.
    uint16_t *Register[2]; 			//Register zero is holding
    uint16_t RegisterSize[2];

private:

};


class ModBusRTU_MasterClass : public ModBusRTU_BaseClass{

public:

	ModBusRTU_MasterClass(){};

	void ReadAllSensorData();
	void ParseSlaveResponse();

private:

	void ParseSlaveData();

};

class ModBusRTU_SlaveClass : public ModBusRTU_BaseClass{

public:

	ModBusRTU_SlaveClass(){};

	void ParseMasterRequest();
	void SetRegisterValue(float data, uint16_t index);
	void LoadRegisters(); //FIXME only for testing

private:

	void FormatRegisterData();

    //Functions for handling the specific function codes
    void HandleFC_3_4();
    void HandleFC_6();

    uint8_t ModBusSerialNumber();
    uint8_t ModBusCalibration();
    uint8_t ModBusFactoryDefaults();
    uint8_t ModBusSensorTag();
    uint8_t ModBusStoreToNVM();

};


#endif /* INC_MODBUS_HPP_ */







































