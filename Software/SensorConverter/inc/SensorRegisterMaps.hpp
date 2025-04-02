/*
 * SensorConfig.hpp
 *
 *  Created on: Mar 11, 2025
 *      Author: Joey.Strandnes
 */

#ifndef INC_SENSORREGISTERMAPS_HPP_
#define INC_SENSORREGISTERMAPS_HPP_

//Supported sensors
#define	TYPE_CMC		3
#define	TYPE_LT600 		4
#define	TYPE_LT600_FLX 	5 //Shows up as the flow sensor, performs extra calculations but still communicates with LT600.


//Defines for how many entries, not how many 16-bit wide ModBus registers are used
#define LT600_HOLDING_MASTER_MAP_SIZE 0
#define LT600_INPUT_MASTER_MAP_SIZE 4

#define LT600_HOLDING_SLAVE_MAP_SIZE 8
#define LT600_INPUT_SLAVE_MAP_SIZE 3

#define LT600_FLX_HOLDING_SLAVE_MAP_SIZE 20
#define LT600_FLX_INPUT_SLAVE_MAP_SIZE 3


#include <cstdlib>
#include <SensorConverter.hpp>
#include <FLX_Calculations.hpp>


enum ModBusRegisterType{CHAR, UINT8, INT16, UINT16, UINT32, FLOAT, PONDUS_FLOAT};

//Contains all information regarding the sensors data and how to handle it.
struct Measurement_Register{


	uint16_t Index{0};							// ModBus register index not index in the database.
	enum ModBusRegisterType RegType{UINT16}; 	//Register data type.

	//Used to always have a correctly scaled float value.
	uint16_t ScaleFactor{1};
	float OutputData{0};

	//Raw register content from the sensor, used when forwarding it to a SCADA system.
	union Data_Union{
		int16_t INT16;
		uint16_t UINT16;
		uint32_t UINT32;
		float FLOAT;
		float PONDUS_FLOAT;
	}InputData;


};

void LinkSensorConfig(class ModBusRTU_BaseClass *modbus_master, class ModBusRTU_BaseClass *modbus_slave, uint8_t sensor_type);
void LoadModBusRegisters(class ModBusRTU_BaseClass *modbus_master, class ModBusRTU_BaseClass *modbus_slave, uint8_t sensor_type); //Moves the data into correct registers


void CMC_MasterRegisters(struct Measurement_Register *registers[2]);
void CMC_SlaveRegisters(struct Measurement_Register *registers[2]);

void LT600_MasterRegisterMap(struct Measurement_Register *registers[2], uint16_t *register_map_size);
void LT600_SlaveRegisterMap(struct Measurement_Register *registers[2], uint16_t *register_map_size);

void LT600_FLX_SlaveRegisters(struct Measurement_Register *registers[2], uint16_t *register_map_size);

#endif /* INC_SENSORREGISTERMAPS_HPP_ */
