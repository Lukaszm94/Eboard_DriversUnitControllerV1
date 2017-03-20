/*
 * lightsmanager.c
 *
 *  Created on: 2 sty 2017
 *      Author: Luke
 */
#include "lightsmanager.h"
#include "globals.h"
#include "hal.h"

#define BLINKING_PATTERNS_COUNT 3

typedef struct BlinkingPattern {
  uint16_t timeOn; // in ms
  uint16_t timeOff; // in ms
} BlinkingPattern;

BlinkingPattern blinkingPatterns[BLINKING_PATTERNS_COUNT];

static void pwmpcb(PWMDriver* pwmp)
{
  (void) pwmp;
}

static void pwmc1cb(PWMDriver *pwmp)
{
  (void) pwmp;
}

void lm_init(void)
{
  PWMConfig pwmcfg = {
                      100000,
                      1000,
                      pwmpcb,
                      {
                       {PWM_OUTPUT_ACTIVE_HIGH, pwmc1cb},
                       {PWM_OUTPUT_DISABLED, NULL},
                       {PWM_OUTPUT_DISABLED, NULL},
                       {PWM_OUTPUT_DISABLED, NULL}
                      },
                      0,
                      0
  };
  pwmStart(&PWMD1, &pwmcfg);
  palSetPadMode(GPIOA, 8, PAL_MODE_ALTERNATE(2));

  blinkingPatterns[0].timeOff = 0;
  blinkingPatterns[0].timeOn = 1000;
  blinkingPatterns[1].timeOff = 500;
  blinkingPatterns[1].timeOn = 500;
  blinkingPatterns[2].timeOff = 800;
  blinkingPatterns[2].timeOn = 200;
  CANLightsPacket packet;
  packet.blinkingMode = LIGHTS_DEFAULT_MODE;
  packet.brightness= LIGHTS_DEFAULT_PWM;
  packet.reactToBraking= LIGHTS_DEFAULT_REACT_TO_BRAKING;
  lm_newLightsPacket(packet);
}

void lm_newVESCCurrentPacket(CANPacket2 packet, uint8_t deviceId)
{
  static VESCBatteryData vesc1, vesc2;
  if(deviceId == VESC_1_ID) {
    vesc1.totCurrentFiltered = packet.totCurrentFiltered;
  } else if(deviceId == VESC_2_ID) {
    vesc2.totCurrentFiltered = packet.totCurrentFiltered;
  } else {
    return;
  }
  if((vesc1.totCurrentFiltered < -40) || (vesc2.totCurrentFiltered < -40) ) {
    lm_onBraking(1);
  } else {
    lm_onBraking(0);
  }
}

void lm_newLightsPacket(CANLightsPacket packet)
{
  if(chMtxTryLock(&lm_dataMutex)) {
    lm_data.blinkingMode = packet.blinkingMode;
    lm_data.brightness = packet.brightness;
    lm_data.reactToBraking = packet.reactToBraking;
    lm_setPWM(lm_data.brightness);
    if(lm_data.blinkingMode < BLINKING_PATTERNS_COUNT) {
      uint8_t mode = lm_data.blinkingMode;
      lm_data.timeOn = blinkingPatterns[mode].timeOn;
      lm_data.timeOff = blinkingPatterns[mode].timeOff;
    }
    chMtxUnlock(&lm_dataMutex);
  }
}

LightsManagerData lm_getData(void)
{
  LightsManagerData data;
  data.blinkingMode = LIGHTS_DEFAULT_MODE;
  data.brightness = LIGHTS_DEFAULT_PWM;
  data.reactToBraking = LIGHTS_DEFAULT_REACT_TO_BRAKING;
  int i = 0;
  for(i = 0; i < 5; i++) {
    if(chMtxTryLock(&lm_dataMutex)) {
      data = lm_data;
      chMtxUnlock(&lm_dataMutex);
      return data;
    }
    chThdSleepMilliseconds(20);
  }
  return data;
}

void lm_onBraking(uint8_t braking)
{
  if(chMtxTryLock(&lm_dataMutex)) {
    if(lm_data.reactToBraking == 0) {
      lm_data.braking = 0;
    } else {
      lm_data.braking = braking;
    }
    chMtxUnlock(&lm_dataMutex);
  }
  if(lm_data.reactToBraking == 0) {
    return;
  }
  if(braking == 1) {
    lm_setPWM(LIGHTS_BRAKING_PWM);
  }
}

void lm_setPWM(uint8_t pwm)
{
  if(pwm == 0) {
    pwmDisableChannel(&PWMD1, 0);
    return;
  }
  uint16_t percentage = pwm * 39.4; // percentage is from 0 to 10000, LOL
  pwmEnableChannel(&PWMD1, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, percentage));
}
