/*
 * FLX_Calculations.cpp
 *
 *  Created on: Mar 24, 2025
 *      Author: Joey.Strandnes
 */



#include <FLX_Calculations.hpp>



void FLX_GetGutterName(uint8_t gutter_type, char *buffer, uint8_t buffer_size){
//Simply copy the name of the gutter to the output buffer.

	switch(gutter_type){

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



float FLX_CalculateFlow(float mh2o, uint8_t gutter_type){


	//Perform temperature compensation?
	//BB2 does this but im not sure if it is for the electronics or for the water.
	//Colder water = higher density so should be temperature dependent?


	float Flow;

	//Below should be set during calibration/ setting gutter type
	float X1, X2, X3;
	uint16_t Alpha = 45; //Angle set to 45 for testing
	uint16_t Width = 1;
	uint16_t Sill = 500;

	//All calculations are based on mH20
	switch(gutter_type){

	case(Gutter_Parshall): //Q= (sqrt(g) * C * W * Ha**n
		//This is copied from BB2 who seems to use a pre calculated table for the values.
		//Width in inches
		FLX_ParshallValues(Width, &X1, &X2); //Only do when setting the type.
		Flow = X1 * std::pow(mh2o, X2) * 3600; //X1 = C, X2 = n
		break;
	case(Gutter_Thompson): // Thompson   Q= Ce * 8/15 * tan(alfa/2) * sqrt(2g) *He**2.5
		//Angle in degrees
		X1 = FLX_ThomsonValue(Alpha); //Only do when setting the type.
		Flow =  X1 * std::pow(mh2o, 2.5);
		break;
	case(Gutter_Rekt): // Rect Wier  Q= Ce * sqrt(2g) * b * ((h+0.0012)**1.5)
		//Width in mm
		//Sill in mm
		X1 = FLX_RectWeirValue(Width); //Only do when setting the type.
		Flow = X1 * (0.602 + 0.083 * (mh2o/Sill)) * std::pow(mh2o + 0.0012, 1.5);
		break;
	case(Gutter_RSK):
		//TBD
		break;
	case(Gutter_PB): //Q= ((h / hmax)**1.868) * maxflow

		X1 = (2 * Width / 100);
		X2 = std::pow(Width/10, 2.5) * 136.244; //Taken from BB2, no idea what the magic numbers are for.
		Flow = std::pow(mh2o/X1, 1.868) * X2;
		break;
	case(Gutter_Cipoletti):
		X1 = (2.0/3) * 0.63 * sqrt(2 * 9.81) * Width * 3600;
		Flow = X1 * std::pow(mh2o + 0.0012, 1.5);
		break;
	case(Gutter_Sutro):

		X1 = (2.0/3) * 0.611 * sqrt(2 * 9.81) * Width * std::pow(Sill, 1.5);
		X2 = X1/Sill;
		X3 = 0.611 * sqrt(2 * 9.81) * Width * std::sqrt(Sill);

		if(mh2o > Sill){
			Flow = ((X1 + X3 * (mh2o - Sill)) * 3600);
		}
		else{
			Flow = X2 * mh2o * 3600;
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





void FLX_ParshallValues(uint16_t throat_width, float *c, float *n){
//Only needs to be computed when the user selects the type of gutter
//BB2 only supports up to 96 inches, general table up to 50 ft
/*
	uint16_t ThroatWidth[] = {1,2,3,6,9,12,18,24,36,48,60,72,84, 96};
	float C[] = {0.338, 0.676, 0.992, 2.06, 3.07, 3.95, 6.0, 8.0, 12.0, 16.0, 20.0, 24.0, 28.0, 32.0};
	float N[] = {1.55, 1.55, 1.55, 1.58,1.53, 1.55, 1.54, 1.55, 1.57, 1.58, 1.59, 1.59, 1.6, 1.61};



	uint8_t TableSize = sizeof(ThroatWidth)/sizeof(ThroatWidth[0]);

	for(uint8_t i = 0; i < TableSize; i++){

		if(throat_width == ThroatWidth[i]){
			*c = C[i];
			*n = N[i];
			return;
		}
		else if(throat_width > ThroatWidth[i] && throat_width < ThroatWidth[i]){ //Should ideally return an error.
			*c = C[i-1];
			*n = N[i-1];

			return;
		}


	}
*/


	uint8_t TableSize = sizeof(parshallDefs[0])/sizeof(parshallDefs[0][0]);

	for(uint8_t i = 0; i < TableSize; i++){

		if(throat_width == parshallDefs[0][i]){
			*c = parshallDefs[1][i];
			*n = parshallDefs[2][i];
			return;
		}
		else if(throat_width > parshallDefs[0][i-1] && throat_width < parshallDefs[0][i]){ //Should ideally return an error.
			*c = parshallDefs[1][i-1];
			*n = parshallDefs[2][i-1];

			return;
		}


	}





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


















