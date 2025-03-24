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



float FLX_CalculateFlow(float mh20, uint8_t gutter_type){


	//Perform temperature compensation?
	//BB2 does this but im not sure if it is for the electronics or for the water.
	//Colder water = higher density so should be temperature dependent?


	float Flow, C,n;

	//All calculations are based on mH20
	switch(gutter_type){

	case(Gutter_Parshall): //Q= (sqrt(g) * C * W * Ha**n
		//This is copied from BB2 who seems to use a pre calculated table for the values.
		//C & n are table values

		FLX_ParshallValues(1, &C, &n); //Width set to 1 for testing
		Flow = C * std::pow(mh20, n) * 3600;
		break;
	case(Gutter_Thompson):
		Flow = FLX_ThomsonValue(45) * std::pow(mh20, 2.5);
		break;
	case(Gutter_Rekt):

		break;
	case(Gutter_RSK):

		break;
	case(Gutter_PB):

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


















