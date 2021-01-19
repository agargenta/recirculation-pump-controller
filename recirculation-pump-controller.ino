#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "TemperatureSensor.h"
#include "FlowSensor.h"
#include "PowerRelay.h"
#include "Button.h"
#include "Menu.h"

#define FLOW_SENSOR_PIN                  2
#define BUTTON_PIN                       3
#define OUTLET_TEMPERATURE_SENSOR_PIN   11
#define RETURN_TEMPERATURE_SENSOR_PIN   12
#define PUMP_POWER_RELAY_PIN            13
#define SENSORS_READING_FREQUENCY_HZ     1
#define OLED_SCREEN_WIDTH              128
#define OLED_SCREEN_HEIGHT              32
#define OLED_RESET                       4
#define OLED_TIMEOUT                 60000 //  1 minute
#define MAX_TEMP_DIFF                    6
#define MIN_TEMP_DIFF                    3
#define MIN_PUMP_OFF_DURATION       180000 //  3 minutes
#define MIN_PUMP_ON_DURATION        300000 //  5 minutes
#define MAX_PUMP_ON_DURATION        600000 // 10 minutes

TemperatureSensor outletTemperatureSensor(OUTLET_TEMPERATURE_SENSOR_PIN);
TemperatureSensor returnTemperatureSensor(RETURN_TEMPERATURE_SENSOR_PIN);
FlowSensor flowSensor(FLOW_SENSOR_PIN);
PowerRelay pumpPowerRelay(PUMP_POWER_RELAY_PIN);
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
boolean displayOn = true;
Button button(BUTTON_PIN);
Menu menu(&display);
unsigned long lastSensorsReadingTime;
unsigned int missedTempTargetCount = 0;
unsigned long totalWarmPulses = 0;

void flowSensorIsr() {
  flowSensor.isr();
}

void buttonIsr() {
  button.isr();
}

void displayPumpStatus()                { if (pumpPowerRelay.isOn()) { displaySeconds(millis() - pumpPowerRelay.getLastToggleTimestamp()); } else { display.print("off"); } }
void displayPumpTotalOnCount()          { display.print(pumpPowerRelay.getTotalOnCount()); }
void displayPumpTotalOnTime()           { displayMinutes(pumpPowerRelay.getTotalOnDuration()); }
void displayPumpTotalOnTimeRatio()      { displayPercentage(float(pumpPowerRelay.getTotalOnDuration()) / millis()); };
void displayPumpTotalAverageOnTime()    { displayMinutes(pumpPowerRelay.getTotalOnCount() == 0? 0 : pumpPowerRelay.getTotalOnDuration() / float(pumpPowerRelay.getTotalOnCount())); }
void displayPumpMissedTargetCount()     { display.print(missedTempTargetCount); }
void displayTemperatureDifference()     { displayTemperature(outletTemperatureSensor.getLastTemperature() - returnTemperatureSensor.getLastTemperature()); }
void displayOutletTemperatureCurrent()  { displayTemperature(outletTemperatureSensor.getLastTemperature()); }
void displayOutletTemperatureMin()      { displayTemperature(outletTemperatureSensor.getMinTemperature()); }
void displayOutletTemperatureMax()      { displayTemperature(outletTemperatureSensor.getMaxTemperature()); }
void displayReturnTemperatureCurrent()  { displayTemperature(returnTemperatureSensor.getLastTemperature()); }
void displayReturnTemperatureMin()      { displayTemperature(returnTemperatureSensor.getMinTemperature()); }
void displayReturnTemperatureMax()      { displayTemperature(returnTemperatureSensor.getMaxTemperature()); }
void displayCurrentFlowRate()           { displayGpm(flowSensor.getLastGpm()); }
void displayCurrentFlowDuration()       { displaySeconds(flowSensor.getCurrentFlowTimeMillis()); }
void displayCurrentFlowVolume()         { displayGallons(flowSensor.getCurrentFlowPulses()); }
void displayTotalFlowCount()            { display.print(flowSensor.getTotalFlowCount()); }
void displayTotalFlowDuration()         { displayMinutes(flowSensor.getTotalFlowMillis()); }
void displayTotalFlowVolume()           { displayGallons(flowSensor.getTotalFlowPulses()); }
void displayWarmFlowRatio()             { displayPercentage(flowSensor.getTotalFlowPulses() == 0? 0 : totalWarmPulses / float(flowSensor.getTotalFlowPulses())); }

MenuItem menuItems[] = {
  {"Smart Recirculation\nPump Controller\nv1.1.0", "tinyurl.com/smartrpc", NULL},
  {"Pump Status", "on-time", displayPumpStatus},
  {"Pump Total", "count", displayPumpTotalOnCount},
  {"Pump Total", "on-time", displayPumpTotalOnTime},
  {"Pump Total", "on-time ratio", displayPumpTotalOnTimeRatio},
  {"Pump Total", "average on-time", displayPumpTotalAverageOnTime},
  {"Pump Missed Target", "count", displayPumpMissedTargetCount },
  {"Temperature Diff", NULL, displayTemperatureDifference},
  {"Outlet Temperature", "current", displayOutletTemperatureCurrent},
  {"Outlet Temperature", "min", displayOutletTemperatureMin},
  {"Outlet Temperature", "max", displayOutletTemperatureMax},
  {"Return Temperature", "current", displayReturnTemperatureCurrent},
  {"Return Temperature", "min", displayReturnTemperatureMin},
  {"Return Temperature", "max", displayReturnTemperatureMax},
  {"Current Flow", "rate (\xF1 10%)", displayCurrentFlowRate},
  {"Current Flow", "duration", displayCurrentFlowDuration},
  {"Current Flow", "volume (\xF1 10%)", displayCurrentFlowVolume},
  {"Total Flow", "count", displayTotalFlowCount},
  {"Total Flow", "duration", displayTotalFlowDuration},
  {"Total Flow", "volume (\xF1 10%)", displayTotalFlowVolume},
  {"Warm Flow", "ratio", displayWarmFlowRatio},
};

void setup() {
  Serial.begin(9600);

  // set up temperature sensors
  outletTemperatureSensor.begin();
  returnTemperatureSensor.begin();

  // set up flow sensor
  flowSensor.begin(flowSensorIsr);

  // set up the pump relay
  pumpPowerRelay.begin();

  // set up to button
  button.onInterrupt(buttonIsr);
  button.onPressed(onButtonPressed);

  // set up display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed. Can't set up the display. Aborting setup."));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();

  // set up the menu
  menu.configure(menuItems, (sizeof(menuItems) / sizeof(MenuItem)));
}

void loop() {
  const unsigned long now = millis();
  const unsigned long timeSinceLastReading = now - lastSensorsReadingTime;
  if (timeSinceLastReading >= (1000 / SENSORS_READING_FREQUENCY_HZ)) {
    lastSensorsReadingTime = now;
   
    // read temperature sensors
    const float outletTemperature = outletTemperatureSensor.readTemperature();
    const float returnTemperature = returnTemperatureSensor.readTemperature();
    // TODO: check for temperature errors
    const float tempDiff = outletTemperature - returnTemperature;
    
    // read flow sensor
    const boolean isFlowing = flowSensor.read(timeSinceLastReading);

    const unsigned long timeSinceLastPumpToggle = now - pumpPowerRelay.getLastToggleTimestamp();

    if (
      isFlowing &&                                              // if the water is flowing AND
      tempDiff >= MAX_TEMP_DIFF &&                              // the temperature difference is too large AND
      !pumpPowerRelay.isOn() &&                                 // the pump is not already on AND
      timeSinceLastPumpToggle >= MIN_PUMP_OFF_DURATION          // the pump has not run too recently
      ) {
      pumpPowerRelay.on(); 
    } else if (
      pumpPowerRelay.isOn() &&                                  // if the pump is currently on AND
      (
        timeSinceLastPumpToggle >= MAX_PUMP_ON_DURATION ||      // it has either been running too long OR
        (
          tempDiff <= MIN_TEMP_DIFF &&                          // we have already reached the required temperature AND
          (
            timeSinceLastPumpToggle >= MIN_PUMP_ON_DURATION ||  // the pump has either been running long enough OR
            flowSensor.isFlowing()                              // the water is flowing
          )
        )
      )) {
      pumpPowerRelay.off();
      // record how many times we timed out before we reached the temperature target
      if (timeSinceLastPumpToggle >= MAX_PUMP_ON_DURATION) {
        missedTempTargetCount++;
      }
    }

    // if the water is flowing and it's warm enough
    if (isFlowing && tempDiff <= MIN_TEMP_DIFF) {
      totalWarmPulses += flowSensor.getLastPulses();
    }
  }
  
  button.read();
  menu.show();
  turnOffDisplayOnInactivity();
}

void onButtonPressed() {
  if (displayOn) {
    menu.next();
  } else {
    display.ssd1306_command(SSD1306_DISPLAYON);
    displayOn = true;
  }
}

void turnOffDisplayOnInactivity() {
  if (displayOn && millis() - button.getLastPressedAtTimestamp() > OLED_TIMEOUT) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    displayOn = false;
  }
}

void displayTemperature(float temperature) {
  display.print(temperature);
  display.print("\xF8 C");
}

void displayGpm(float gpm) {
  if (gpm > 0) {
    display.print(gpm);
    display.print("gpm");
  } else {
    display.print("n/a");
  }
}

void displaySeconds(unsigned long durationMillis) {
  if (durationMillis > 0) {
    display.print(durationMillis / 1000);
    display.print(" secs");
  } else {
    display.print("n/a");
  }
}

void displayMinutes(unsigned long durationMillis) {
  if (durationMillis > 0) {
    display.print(durationMillis / 60000);
    display.print(" mins");
  } else {
    display.print("n/a");
  }
}

void displayGallons(unsigned long pulses) {
  display.print(flowSensor.pulsesToGallons(pulses));
  display.print(" gal");
}

void displayPercentage(float fraction) {
  display.print(fraction * 100.00f);
  display.print('%');  
}
