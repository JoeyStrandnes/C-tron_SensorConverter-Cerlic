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


//Exception codes
#define MODBUS_EXCEPTION_OK                     0
#define MODBUS_EXCEPTION_ILLIGAL_FUNCTION       1
#define MODBUS_EXCEPTION_ILLIGAL_DATA_ADDRESS   2
#define MODBUS_EXCEPTION_ILLIGAL_DATA_VALUE     3
#define MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE   4
#define MODBUS_EXCEPTION_ACK                    5
#define MODBUS_EXCEPTION_SLAVE_DEVICE_BUSY      6
#define MODBUS_EXCEPTION_MEMORY_ERROR           8


class ModBusRTU_Class{

public:

	ModBusRTU_Class(){};

    uint8_t Address;
    bool	isMaster{false};

    //Registers visible through the ModBus interface. Holding registers and input registers.
    uint16_t *Register[2]; //Register zero is holding
    uint16_t RegisterSize[2];

    uint8_t *OutputBuffer;          //The TX buffer
    uint16_t OutputBufferSize;      //Size of TX buffer

    uint8_t *InputBuffer;           //The RX buffer
    uint16_t InputBufferSize;       //Size of the RX buffer

    uint16_t ResponseSize;          //How many bytes to transmit
    uint16_t RequestSize;           //How many bytes were received


    void ParseModBusRTUPacket();


private:

    void ParseSlavePacket();
    void ParseMasterPacket();

    uint16_t ModBusCRC(uint8_t *input_buffer, uint16_t const size);
    void BuildModBusException(uint8_t exeption);


    //Functions for handling the specific function codes
    void HandleFC_3_4();
    void HandleFC_6();

    uint8_t ModBusSerialNumber();
    uint8_t ModBusCalibration();
    uint8_t ModBusFactoryDefaults();
    uint8_t ModBusSensorTag();
    uint8_t ModBusStoreToNVM();

    //void (*MapRegisters)(struct Measurement_Register *registers[2]) = NULL;
	struct Measurement_Register *RegisterMap[2] = {NULL,NULL};

};




#endif /* INC_MODBUS_HPP_ */
