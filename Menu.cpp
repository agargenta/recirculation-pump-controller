#include "Menu.h"

Menu::Menu(Adafruit_SSD1306 *display) {
  this->display = display;
}

void Menu::configure(MenuItem *menuItems, byte menuItemsCount) {
  this->menuItems = menuItems;
  this->menuItemsCount = menuItemsCount;
}

void Menu::next() {
  selection++;
  if (selection == menuItemsCount) {
    selection = 0;
  }
}

void Menu::show() {
  if (!display || !menuItems) {
    return;
  }
  display->clearDisplay();
  display->setTextColor(SSD1306_WHITE);
  display->cp437(true);
  display->setCursor(0, 0);
  MenuItem menuItem = menuItems[selection];
  display->setTextSize(1);
  display->println(menuItem.header1);
  display->println(menuItem.header2);
  display->setTextSize(2);
  if (menuItem.displayValueFn) {
    display->setTextSize(2);
    menuItem.displayValueFn();
  }
  if (selection > 0) {
    display->setTextSize(1);
    display->setCursor(display->width() - 12, 0); 
    display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    if (selection < 10) {
      display->print(0);
    }
    display->print(selection);
  }
  display->display();
}
