#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class TemperatureSensor {
  
  private:
    OneWire wire;
    DallasTemperature sensor;
    float lastTemperature;
    float minTemperature;
    float maxTemperature;

  public:
    TemperatureSensor(byte);
    void begin();
    float readTemperature();
    float getLastTemperature();
    float getMinTemperature();
    float getMaxTemperature();
};

#endif
