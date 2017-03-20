/*
 * lightsmanager.h
 *
 *  Created on: 2 sty 2017
 *      Author: Luke
 */

#ifndef LIGHTSMANAGER_H_
#define LIGHTSMANAGER_H_

#include "datatypes.h"

void lm_init(void);
void lm_newVESCCurrentPacket(CANPacket2 packet, uint8_t deviceId);
void lm_newLightsPacket(CANLightsPacket packet);
LightsManagerData lm_getData(void);

// private
void lm_onBraking(uint8_t braking);
void lm_setPWM(uint8_t pwm);

#endif /* LIGHTSMANAGER_H_ */
