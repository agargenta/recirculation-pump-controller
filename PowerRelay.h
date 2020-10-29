#ifndef POWER_RELAY_H
#define POWER_RELAY_H

#include <Arduino.h>

class PowerRelay {
  
  private:
    byte pin;
    unsigned int totalOnCount = 0;
    unsigned long totalOnDuration = 0;
    unsigned long currentOnTimestamp = 0;

  public:
    PowerRelay(byte pin);
    void begin();
    boolean isOn();
    void on();
    void off();
    unsigned int getTotalOnCount();
    unsigned long getTotalOnDuration();
    unsigned long getCurrentOnDuration();
};

#endif
