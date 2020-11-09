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
    currentFlowPulses += lastPulses;
    currentFlowTimeMillis += period;
    // if we've crossed the minimum pulses to detect a flow
    if (currentFlowPulses >= FLOW_SENSOR_MIN_PULSES_FOR_FLOW_DETECTION) {
      // if this is the first time we've detected this flow
      if (!currentFlowDetected) {
        currentFlowDetected = true;
        totalFlowCount++;                             // count this flow
        totalPulses += currentFlowPulses;             // record the accumulated pulses for the current flow
        totalFlowTimeMillis += currentFlowTimeMillis; // record the accumulated time for the current flow
      } else {
        totalPulses += lastPulses;                    // record the last pulses reported
        totalFlowTimeMillis += period;                // record the last period
      }
    }
  } else { // there is no flow
    // reset all accumulated stats for the current flow
    currentFlowPulses = 0;
    currentFlowTimeMillis = 0;
    currentFlowDetected = false;
  }
  return lastPulses;
}

boolean FlowSensor::isFlowing() {
  return currentFlowDetected;
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

float FlowSensor::pulsesToGallons(unsigned long pulses) {
  return pulses / float(FLOW_SENSOR_PULSES_PER_GALLON);
}
