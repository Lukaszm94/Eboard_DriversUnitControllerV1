/*
 * temperaturemanager.c
 *
 *  Created on: 31 gru 2016
 *      Author: Luke
 */
#include "temperaturemanager.h"
#include "globals.h"
#include "config.h"
#include "analogmanager.h"
#include "serialmanager.h"
#include "math.h"

#define TM_SERIAL_DEBUG 0

#define NTC_RES(adc_val)    ((4095.0 * 10000.0) / adc_val - 10000.0)
// calculate NTC resistance
//#define NTC_RES(adc_val)    ((10000.0 * adc_val) / (4095.0 - adc_val))
#define NTC_TEMP(adc_value)   (1.0 / ((logf(NTC_RES(adc_value) / 10000.0) / 3434.0) + (1.0 / 298.15)) - 273.15)

void tm_updateThermistorTemperature(void)
{
  uint16_t adcReading = am_getReading(TM_MOSFET_TEMP_CHANNEL);
  float temperature = NTC_TEMP(adcReading);
  int16_t temperatureI16 = (int16_t)(temperature * 100);
#if TM_SERIAL_DEBUG
  sm_chprintf("tm_updateMosfetTemperature, T=%d\n\r", temperatureI16);
#endif
  if(chMtxTryLock(&temperaturePacketMutex) == TRUE) {
    temperaturePacket.driversUnitCaseTemperature = temperatureI16;
    chMtxUnlock(&temperaturePacketMutex);
  }
}

void tm_onNewTemperaturePacket(CANPacket3 data, uint8_t deviceId)
{
  //TODO we can react here to high VESC's temperature (turn fans on)
  (void) data;
  (void) deviceId;
}

