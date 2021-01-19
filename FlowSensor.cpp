#include "FlowSensor.h"

FlowSensor::FlowSensor(byte pin) {
  this->pin = pin;
}

void FlowSensor::begin(void (*isr)(void)) {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
  attachInterrupt(digitalPinToInterrupt(pin), isr, FALLING);
}

void FlowSensor::isr() {
  pulses++;
}

boolean FlowSensor::read(unsigned long period) {
  noInterrupts();
  lastPulses = pulses;
  pulses = 0;
  interrupts();
  lastPeriodMillis = period;
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
  return currentFlowDetected;
}

byte FlowSensor::getLastPulses() {
  return lastPulses;
}

float FlowSensor::getLastGpm() {
  return lastPulses == 0 || lastPeriodMillis == 0? 0.0 : pulsesToGallons(lastPulses * (60000.0f / float(lastPeriodMillis)));
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
