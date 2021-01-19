#include "Button.h"

#define BUTTON_DEBOUNCE_MILLIS 50UL

Button::Button(byte pin) {
  this->pin = pin;
}

void Button::onInterrupt(void (*buttonIsr)(void)) {
  pinMode(pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pin), buttonIsr, CHANGE);
}

void Button::onPressed(void (*onPressedCallback)(void)) {
  this->onPressedCallback = onPressedCallback;
}

boolean Button::read() {
  noInterrupts();
  const boolean lastPressed = pressed;
  if (lastPressed) {
    pressed = false;
  }
  interrupts();
  if (lastPressed && onPressedCallback) {
    onPressedCallback();
  }
  return lastPressed;
}

unsigned long Button::getLastPressedAtTimestamp() {
  noInterrupts();
  unsigned long v = lastPressedAtTimestamp;
  interrupts();
  return v;
}

void Button::isr() {
  unsigned long now = millis();
  if (now - lastPressedAtTimestamp > BUTTON_DEBOUNCE_MILLIS) {
    lastPressedAtTimestamp = now;
    pressed = true;
  }
}
