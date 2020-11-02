#ifndef POWER_RELAY_H
#define POWER_RELAY_H

#include <Arduino.h>

class PowerRelay {
  
  private:
    byte pin;
    boolean onState = false;
    unsigned int totalOnCount = 0;
    unsigned long totalOnDuration = 0;
    unsigned long lastToggleTimestamp = 0;

  public:
    PowerRelay(byte pin);
    void begin();
    boolean isOn();
    void on();
    void off();
    unsigned int getTotalOnCount();
    unsigned long getTotalOnDuration();
    unsigned long getLastToggleTimestamp();
};

#endif
