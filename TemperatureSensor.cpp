#include "TemperatureSensor.h"

TemperatureSensor::TemperatureSensor(byte pin): wire(pin), sensor(&wire) {
  minTemperature = 1000;
  maxTemperature = -1000;
}

void TemperatureSensor::begin() {
  sensor.begin();
}

float TemperatureSensor::readTemperature() {
  sensor.requestTemperatures();
  lastTemperature = sensor.getTempCByIndex(0);
  return lastTemperature;
}


float TemperatureSensor::getMinTemperature() {
  minTemperature = min(lastTemperature, minTemperature);
  return minTemperature;
}

float TemperatureSensor::getMaxTemperature() {
  maxTemperature = max(lastTemperature, maxTemperature);
  return maxTemperature;
}

float TemperatureSensor::getLastTemperature() {
  return lastTemperature;
}

