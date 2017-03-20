/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "datatypes.h"
#include "halconf.h"
#include "chprintf.h"
#include "canmanager.h"
#include "analogmanager.h"
#include "temperaturemanager.h"
#include "serialmanager.h"
#include "lightsmanager.h"

BaseSequentialStream *chp = (BaseSequentialStream *)&SD1;
//BaseChannel *bch = (BaseChannel*) &SD1;

TemperaturePacket temperaturePacket;
mutex_t temperaturePacketMutex;

CANLightsPacket lightsPacket;
mutex_t lightsPacketMutex;

uint16_t am_dataBuffer[AM_CHANNELS_COUNT * AM_SAMPLES_COUNT];
mutex_t am_dataMutex;

LightsManagerData lm_data;
mutex_t lm_dataMutex;

mutex_t serialMutex;
event_listener_t can_el;


/*
 * CAN receiver thread, waits for rxfull event, then canmanager processes new packets
 */
static THD_WORKING_AREA(waThread1, 512);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("can_receiver");
  chEvtRegister(&CAND1.rxfull_event, &can_el, 0);
  while(true) {
    while(!chThdShouldTerminateX()) {
      if (chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(100)) == 0)
        continue;
      cm_run();
    }
    chEvtUnregister(&CAND1.rxfull_event, &can_el);
  }
}

/*
 * Rear lights blinking thread
 */
static THD_WORKING_AREA(waThread2, 512);
static THD_FUNCTION(Thread2, arg) {

  (void)arg;
  chRegSetThreadName("rearLightsBlinker");
  while(true) {
    LightsManagerData data = lm_getData();
    if(data.braking == 1) {
      chThdSleepMilliseconds(100);
      continue;
    }
    lm_setPWM(data.brightness);
    chThdSleepMilliseconds(data.timeOn);
    data = lm_getData();
    if(data.braking == 1) {
      chThdSleepMilliseconds(100);
      continue;
    }
    if(data.timeOff > 0) {
      lm_setPWM(0);
      chThdSleepMilliseconds(data.timeOff);
    }
  }
}


/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */

  halInit();
  chSysInit();

  chMtxObjectInit(&temperaturePacketMutex);
  chMtxObjectInit(&serialMutex);
  chMtxObjectInit(&am_dataMutex);
  chMtxObjectInit(&lm_dataMutex);

  sm_init();
  cm_init();
  am_init();
  lm_init();


  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
  chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);

  palClearLine(LINE_LED_GREEN);
  while (true) {
    chThdSleepMilliseconds(500);
    palToggleLine(LINE_LED_GREEN);
    tm_updateThermistorTemperature();
    if(chMtxTryLock(&temperaturePacketMutex)) {
      cm_sendTemperaturePacket(temperaturePacket);
      chMtxUnlock(&temperaturePacketMutex);
    }

  }
}
