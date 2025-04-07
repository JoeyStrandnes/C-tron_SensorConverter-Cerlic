/*
 * FLX_Calculations.cpp
 *
 *  Created on: Mar 24, 2025
 *      Author: Joey.Strandnes
 */



#include <FLX_Calculations.hpp>


uint8_t SensorFLX::Calibrate(class ModBusRTU_BaseClass *modbus){
//Assumed to be sanity checked when entering the function.


	uint16_t Command = (((uint16_t)modbus->InputBuffer[4] << 8) | (uint16_t)modbus->InputBuffer[5]);

	if(Command == 0){ //Perform the computations for the gutter type.

		this->GutterType = modbus->InputBuffer[6];

		uint16_t Arg1 = (((uint16_t)modbus->InputBuffer[7] << 8) | (uint16_t)modbus->InputBuffer[8]);
		uint16_t Arg2 = (((uint16_t)modbus->InputBuffer[9] << 8) | (uint16_t)modbus->InputBuffer[10]);
		uint16_t Arg3 = (((uint16_t)modbus->InputBuffer[11] << 8) | (uint16_t)modbus->InputBuffer[12]);

		this->CalculateGutterCoefficient(Arg1, Arg2, Arg3);


		return MODBUS_EXCEPTION_OK;

	}
	else if(Command == 1){ //Only change gutter, no other parameters are passed.

		this->GutterType = modbus->InputBuffer[6];

		this->LoadDefaultFlumeParamters();

		return MODBUS_EXCEPTION_OK;

	}

	else if(Command == 2){ //Only change the width etc

		uint16_t Arg1 = (((uint16_t)modbus->InputBuffer[6] << 8) | (uint16_t)modbus->InputBuffer[7]);
		uint16_t Arg2 = (((uint16_t)modbus->InputBuffer[8] << 8) | (uint16_t)modbus->InputBuffer[9]);
		uint16_t Arg3 = (((uint16_t)modbus->InputBuffer[10] << 8) | (uint16_t)modbus->InputBuffer[11]);

		this->CalculateGutterCoefficient(Arg1, Arg2, Arg3);

		return MODBUS_EXCEPTION_OK;

	}



	else if(Command == 3){ //Flow offset adjustment

		uint16_t Arg1 = (((uint16_t)modbus->InputBuffer[6] << 8) | (uint16_t)modbus->InputBuffer[7]);
		uint16_t Arg2 = (((uint16_t)modbus->InputBuffer[8] << 8) | (uint16_t)modbus->InputBuffer[9]);

		this->OffsetCalDate = (Arg1 << 16 | Arg2);
		this->OffsetCal = this->RawData;

		return MODBUS_EXCEPTION_OK;

	}




	return MODBUS_EXCEPTION_SLAVE_DEVICE_FAILURE;
}



float SensorFLX::CalculateMeasurement(){
//Virtual function that calculates the FLX flow

//Perform temperature compensation?
//BB2 does this but im not sure if it is for the electronics or for the water.
//Colder water = higher density so should be temperature dependent?
//Henrik claims its due to nonlinearity on the pressure transducer. Might be in the electronics, who knows.

	if((this->RawData > 1000) || (this->RawData <= this->OffsetCal)){
		return 0;
	}

	float Flow;
	this->mH2O = this->RawData - this->OffsetCal; //Remove the static offset.

	//All calculations are based on mH20
	switch(this->GutterType){

	case(Gutter_Parshall): //Q= (sqrt(g) * C * W * Ha**n
		//This is copied from BB2 who seems to use a pre calculated table for the values.
		//Width in inches
		Flow = this->X1 * std::pow(this->mH2O, this->X2) * 3600; //X1 = C, X2 = n
		break;
	case(Gutter_Thompson): // Thompson   Q= Ce * 8/15 * tan(alfa/2) * sqrt(2g) *He**2.5
		//Angle in degrees
		Flow =  this->X1 * std::pow(this->mH2O, 2.5);
		break;
	case(Gutter_Rekt): // Rect Wier  Q= Ce * sqrt(2g) * b * ((h+0.0012)**1.5)
		//Width in mm
		//Sill in mm
		Flow = this->X1 * (0.602 + 0.083 * (this->mH2O/this->Sill)) * std::pow(this->mH2O + 0.0012, 1.5);
		break;
	case(Gutter_RSK):
		//TBD
		break;
	case(Gutter_PB): //Q= ((h / hmax)**1.868) * maxflow
		Flow = std::pow(this->mH2O/this->X1, 1.868) * this->X2;
		break;
	case(Gutter_Cipoletti):
		Flow = this->X1 * std::pow(this->mH2O + 0.0012, 1.5);
		break;
	case(Gutter_Sutro):
		if(this->mH2O > this->Sill){
			Flow = ((this->X1 + this->X3 * (this->mH2O - this->Sill)) * 3600);
		}
		else{
			Flow = this->X2 * this->mH2O * 3600;
		}
		break;
	case(Gutter_Venturi):

		break;
	case(Gutter_VenturiU):

		break;

	default:
		Flow = 0;
		break;

	}





	return Flow;
}


void SensorFLX::GetGutterName(char *buffer, uint8_t buffer_size){
//Simply copy the name of the gutter to the output buffer.

	switch(this->GutterType){

	case(Gutter_Parshall):
		std::snprintf(buffer, buffer_size, "Parshall");
		break;
	case(Gutter_Thompson):
		std::snprintf(buffer, buffer_size, "Thompson");
		break;
	case(Gutter_Rekt):
		std::snprintf(buffer, buffer_size, "Rekt. Skib");
		break;
	case(Gutter_RSK):
		std::snprintf(buffer, buffer_size, "RSK");
		break;
	case(Gutter_PB):
		std::snprintf(buffer, buffer_size, "P & B");
		break;
	case(Gutter_Cipoletti):
		std::snprintf(buffer, buffer_size, "Cipoletti");
		break;
	case(Gutter_Sutro):
		std::snprintf(buffer, buffer_size, "Sutro");
		break;
	case(Gutter_Venturi):
		std::snprintf(buffer, buffer_size, "Venturi");
		break;
	case(Gutter_VenturiU):
		std::snprintf(buffer, buffer_size, "Venturi U");
		break;

	default:
		std::snprintf(buffer, buffer_size, "None");
		break;

	}



	return;
}

void SensorFLX::CalculateGutterCoefficient(uint16_t Arg1, uint16_t Arg2, uint16_t Arg3){


	switch(GutterType){

		case(Gutter_Parshall):
		this->Width = Arg1;
		FLX_ParshallValues(Arg1, &(this->X1), &(this->X2));
		break;

		case(Gutter_Thompson):
		this->Width = Arg1;
		this->X1 = FLX_ThomsonValue(Arg1);
		break;

		case(Gutter_Rekt):
		this->Width = Arg1;
		this->Sill = Arg2;

		this->X1 = FLX_RectWeirValue(Arg1);
		break;

		case(Gutter_RSK):
		break;

		case(Gutter_PB):
		this->Width = Arg1;
		this->X1 = (2 * Arg1 / 100);
		this->X2 = std::pow(Arg1/10, 2.5) * 136.244; //Taken from BB2, no idea what the magic numbers are for.
		break;

		case(Gutter_Cipoletti):
		this->Width = Arg1;
		this->X1 = (2.0/3) * 0.63 * sqrt(2 * 9.81) * Arg1 * 3600;
		break;

		case(Gutter_Sutro):
		this->Width = Arg1;
		this->Sill = Arg2;

		this->X1 = (2.0/3) * 0.611 * sqrt(2 * 9.81) * Arg1 * std::pow(Arg2, 1.5);
		this->X2 = X1/Arg2;
		this->X3 = 0.611 * sqrt(2 * 9.81) * Arg1 * std::sqrt(Arg2);
		break;

		case(Gutter_Venturi):
		break;

		case(Gutter_VenturiU):
		break;


		default:
		return;


	}

	return;
}

void SensorFLX::LoadDefaultFlumeParamters(){


	switch(GutterType){

		case(Gutter_Parshall):

			this->Width = 6;
			this->Sill = 0;

			this->X1 = 0.3812;
			this->X2 = 1.580;
			this->X3 = 0;

			break;

		case(Gutter_Thompson):

			this->Width = 45; //Degrees
			this->Sill = 0;

			this->X1 = 0.58031;
			this->X2 = 0;
			this->X3 = 0;

			break;

		case(Gutter_Rekt):

			this->Width = 500; //mm
			this->Sill = 1000;

			this->X1 = 0.58031;
			this->X2 = 0;
			this->X3 = 0;

			break;

		case(Gutter_RSK):
		break;

		case(Gutter_PB):

			this->Width = 11; //inch
			this->Sill = 0;

			this->X1 = 0.22;
			this->X2 = 0.009781;
			this->X3 = 0;

			break;

		case(Gutter_Cipoletti):
		break;

		case(Gutter_Sutro):
		break;

		case(Gutter_Venturi):
		break;

		case(Gutter_VenturiU):
		break;


		default:
		return;
		break;


	}

	return;
}



void FLX_ParshallValues(uint16_t throat_width, float *c, float *n){
//Only needs to be computed when the user selects the type of gutter
//BB2 only supports up to 96 inches, general table up to 50 ft

	uint8_t TableSize = sizeof(parshallDefs)/sizeof(parshallDefs[0]);


	for(uint8_t i = 0; i < TableSize; i++){

		if(parshallDefs[i][0] == throat_width){
			*c = parshallDefs[i][1];
			*n = parshallDefs[i][2];

			return;
		}

	}


	//Error if we get here!!!!

	return;
}



float FLX_ThomsonValue(uint16_t angle){
//Only needs to be computed when the user selects the type of gutter

	float Kb;

	if(angle > 100){
		Kb = 0.58 - ((100 - angle) * 1.6667E-4);
	}
	else if(angle > 60){
		Kb = 0.5767;
	}
	else if(angle > 40){
		Kb = 0.5815 + (( 40 - angle) * 2.38E-4);
	}
	else if(angle >= 20){
		Kb = 0.593 + (( 20 - angle) * 5.556E-4);
	}

	//return Kb * (8.0/15) * std::tan(((angle * M_PI)/180) / 2) * sqrt(2 * 9.81) * 3600;
	return Kb * 8499.6 * std::tan((angle * M_PI) / 360); //Approximation of the above.

}

float FLX_RectWeirValue(uint16_t width){
//Only used when changing to this gutter type.
//Calculates the BB2 X1 value.

	return (2.0/3) * sqrt(2 * 9.81) * width * 3600;

}


















