#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>

typedef struct {
  const char *header1;
  const char *header2;
  void (*displayValueFn)(void);
} MenuItem;

class Menu {
  
  private:
    MenuItem *menuItems;
    byte menuItemsCount;
    Adafruit_SSD1306 *display;
    byte selection;

  public:
    Menu(Adafruit_SSD1306 *display);
    void configure(MenuItem *menuItems, byte menuItemsCount);
    void next();
    void show();
};

#endif
