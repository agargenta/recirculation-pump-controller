#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
  
  private:
    byte pin;
    void (*onPressedCallback)(void);
    volatile unsigned long lastPressedAtTimestamp = 0;
    volatile boolean pressed = false;


  public:
    Button(byte pin);
    void onInterrupt(void (*buttonIsr)(void));
    void onPressed(void (*onPressedCallback)(void));
    unsigned long getLastPressedAtTimestamp();
    void isr();
    boolean read();
};

#endif
