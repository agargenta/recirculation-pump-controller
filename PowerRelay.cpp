#include "PowerRelay.h"

PowerRelay::PowerRelay(byte pin) {
  this->pin = pin;
}

void PowerRelay::begin() {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

boolean PowerRelay::isOn() {
  return onState;
}

void PowerRelay::on() {
  if (!isOn()) {
    digitalWrite(pin, HIGH);
    lastToggleTimestamp = millis();
    totalOnCount++;
    onState = true;
  }
}

void PowerRelay::off() {
  if (isOn()) {
    digitalWrite(pin, LOW);
    unsigned long now = millis();
    totalOnDuration += now - lastToggleTimestamp;
    lastToggleTimestamp = now;
    onState = false;
  }
}

unsigned long PowerRelay::getLastToggleTimestamp() {
  return lastToggleTimestamp;
}

unsigned int PowerRelay::getTotalOnCount() {
  return totalOnCount;
}

unsigned long PowerRelay::getTotalOnDuration() {
  return totalOnDuration;
}
