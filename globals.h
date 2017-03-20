/*
 * globals.h
 *
 *  Created on: 27 gru 2016
 *      Author: Luke
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "datatypes.h"
#include "ch.h"

//----- serial -----
extern BaseSequentialStream *chp;
//extern BaseChannel *bch;

//----- temperature -----
extern TemperaturePacket temperaturePacket;
extern mutex_t temperaturePacketMutex;

//----- analog -----
extern uint16_t am_dataBuffer[AM_CHANNELS_COUNT * AM_SAMPLES_COUNT];
extern mutex_t am_dataMutex;

//----- lights -----
extern LightsManagerData lm_data;
extern mutex_t lm_dataMutex;

//----- serial driver -----
extern mutex_t serialMutex;
//----- can -----
extern event_listener_t can_el;

#endif /* GLOBALS_H_ */
