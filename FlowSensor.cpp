#include "FlowSensor.h"

FlowSensor::FlowSensor(byte pin) {
  this->pin = pin;
}

void FlowSensor::begin() {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}

void FlowSensor::onPulse() {
  pulses++;
}

byte FlowSensor::service(unsigned long period) {
  byte lastPulses; 
  noInterrupts();
  lastPulses = pulses;
  pulses = 0;
  interrupts();
  if (lastPulses > 0) {
    totalPulses += lastPulses;
    totalFlowTimeMillis += period;
    if (currentFlowPulses == 0) {
      totalFlowCount++;
    }
    currentFlowPulses += lastPulses;
    currentFlowTimeMillis += period;
    if (currentFlowPulses >= FLOW_SENSOR_MIN_SIGNIFICANT_PULSES && !currentFlowSignificant) {
      currentFlowSignificant = true;
      totalSignificantFlowCount++;
    }
  } else {
    currentFlowPulses = 0;
    currentFlowTimeMillis = 0;
    currentFlowSignificant = false;
  }
  return lastPulses;
}

boolean FlowSensor::isFlowing() {
  return currentFlowSignificant;
}

unsigned long FlowSensor::getCurrentFlowPulses() {
  return currentFlowPulses;
}

unsigned long FlowSensor::getCurrentFlowTimeMillis() {
  return currentFlowTimeMillis;
}

unsigned long FlowSensor::getTotalFlowPulses() {
  return totalPulses;
}

unsigned long FlowSensor::getTotalFlowMillis() {
  return totalFlowTimeMillis;
}

unsigned long FlowSensor::getTotalFlowCount() {
  return totalFlowCount;
}

unsigned long FlowSensor::getTotalSignificantFlowCount() {
  return totalSignificantFlowCount;
}


float FlowSensor::pulsesToGallons(unsigned long pulses) {
  return pulses / float(FLOW_SENSOR_PULSES_PER_GALLON);
}
