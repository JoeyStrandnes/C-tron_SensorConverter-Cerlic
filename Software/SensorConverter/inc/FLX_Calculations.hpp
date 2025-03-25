/*
 * FLX_Calculations.hpp
 *
 *  Created on: Mar 24, 2025
 *      Author: Joey.Strandnes
 */

#ifndef INC_FLX_CALCULATIONS_HPP_
#define INC_FLX_CALCULATIONS_HPP_

#include <SensorConverter.hpp>
#include <cstdio>
#include <math.h>

enum FLX_GutterTypes{Gutter_Parshall, Gutter_Thompson, Gutter_Rekt, Gutter_RSK, Gutter_PB, Gutter_Cipoletti, Gutter_Sutro, Gutter_Venturi, Gutter_VenturiU};


void FLX_GetGutterName(uint8_t gutter_type, char *buffer, uint8_t buffer_size);
float FLX_CalculateFlow(float mh2o, uint8_t gutter_type);

void FLX_ParshallValues(uint16_t throat_width, float *c, float *n);
float FLX_ThomsonValue(uint16_t angle);
float FLX_RectWeirValue(uint16_t width);

//Copied from BB2
const float parshallDefs[][3] = {
    {96, 6.112, 1.607 },
    {84, 5.312, 1.601 },
    {72, 4.519, 1.595 },
    {60, 3.732, 1.587 },
    {48, 2.953, 1.578 },
    {36, 2.184, 1.566 },
    {24, 1.428, 1.550 },
    {18, 1.056, 1.538 },
    {12, .6909, 1.522 },
    { 9, .5354, 1.530 },
    { 6, .3812, 1.580 },
    { 3, .1771, 1.547 },
    { 2, .1207, 1.547 },
    { 1, .0604, 1.547 }
    };


#endif /* INC_FLX_CALCULATIONS_HPP_ */
