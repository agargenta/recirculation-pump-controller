#include "PowerRelay.h"

PowerRelay::PowerRelay(byte pin) {
  this->pin = pin;
}

void PowerRelay::begin() {
  pinMode(pin, OUTPUT);
}

boolean PowerRelay::isOn() {
  return currentOnTimestamp != 0;
}

void PowerRelay::on() {
  if (!isOn()) {
    digitalWrite(pin, HIGH);
    currentOnTimestamp = millis();
    totalOnCount++;
  }
}

void PowerRelay::off() {
  if (isOn()) {
    digitalWrite(pin, LOW);
    totalOnDuration += getCurrentOnDuration();
    currentOnTimestamp = 0;
  }
}

unsigned long PowerRelay::getCurrentOnDuration() {
  return isOn() ? millis() - currentOnTimestamp : 0;
}

unsigned int PowerRelay::getTotalOnCount() {
  return totalOnCount;
}

unsigned long PowerRelay::getTotalOnDuration() {
  return totalOnDuration + getCurrentOnDuration();
}
