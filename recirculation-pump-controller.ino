#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "TemperatureSensor.h"
#include "FlowSensor.h"
#include "PowerRelay.h"

#define FLOW_SENSOR_PIN                  2
#define OUTLET_TEMPERATURE_SENSOR_PIN   11
#define RETURN_TEMPERATURE_SENSOR_PIN   12
#define PUMP_POWER_RELAY_PIN            13
#define SENSORS_READING_FREQUENCY_HZ     1
#define OLED_SCREEN_WIDTH              128
#define OLED_SCREEN_HEIGHT              64
#define OLED_RESET                       4
#define MAX_TEMP_DIFF                    6
#define MIN_TEMP_DIFF                    3
#define MIN_PUMP_OFF_DURATION       180000 // 3 minutes
#define MIN_PUMP_ON_DURATION        300000 // 5 minutes
#define MAX_PUMP_ON_DURATION        600000 // 10 minutes

TemperatureSensor outletTemperatureSensor(OUTLET_TEMPERATURE_SENSOR_PIN);
TemperatureSensor returnTemperatureSensor(RETURN_TEMPERATURE_SENSOR_PIN);
FlowSensor flowSensor(FLOW_SENSOR_PIN);
PowerRelay pumpPowerRelay(PUMP_POWER_RELAY_PIN);
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long lastSensorsReadingTime;
unsigned int missedTempTargetCount = 0;

void flowSensorIsr() {
  flowSensor.onPulse();
}

void setup() {
  Serial.begin(9600);

  // set up temperature sensors
  outletTemperatureSensor.begin();
  returnTemperatureSensor.begin();

  // set up flow sensor
  flowSensor.begin();
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowSensorIsr, FALLING);

  // set up the pump relay
  pumpPowerRelay.begin();

  // set up display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed. Can't set up the display. Aborting setup."));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
}

void loop() {
  unsigned long now = millis();
  unsigned long timeSinceLastReading = now - lastSensorsReadingTime;
  if (timeSinceLastReading >= (1000 / SENSORS_READING_FREQUENCY_HZ)) {
    lastSensorsReadingTime = now;
   
    // read temperature sensors
    float outletTemperature = outletTemperatureSensor.readTemperature();
    float returnTemperature = returnTemperatureSensor.readTemperature();
    // TODO: check for temperature errors
    float tempDiff = outletTemperature - returnTemperature;
    
    // service flow sensor
    byte pulses = flowSensor.service(timeSinceLastReading);

    unsigned long timeSinceLastPumpToggle = now - pumpPowerRelay.getLastToggleTimestamp();

    // if the water is flowing and the temp difference is too large and the pump is not currently running and it has not been recently running
    if (flowSensor.isFlowing() && tempDiff >= MAX_TEMP_DIFF && !pumpPowerRelay.isOn() && timeSinceLastPumpToggle >= MIN_PUMP_OFF_DURATION) {
      pumpPowerRelay.on(); 
    } 
    // if the pump is currently on and it has either been running too long or the temp difference is small and the pump has been running long enough
    else if (pumpPowerRelay.isOn() && (timeSinceLastPumpToggle >= MAX_PUMP_ON_DURATION || (tempDiff <= MIN_TEMP_DIFF && timeSinceLastPumpToggle >= MIN_PUMP_ON_DURATION))) {
      pumpPowerRelay.off();
      // record how many times we timed out before we reached the temperature target
      if (timeSinceLastPumpToggle >= MAX_PUMP_ON_DURATION) {
        missedTempTargetCount++;
      }
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.setTextSize(1);

    display.print("O:");
    display.print(outletTemperature);
    display.print(" ");
    display.print(outletTemperatureSensor.getMinTemperature());
    display.print(" ");
    display.print(outletTemperatureSensor.getMaxTemperature());
    display.println(" C");
    
    display.print("R:");
    display.print(returnTemperature);
    display.print(" ");
    display.print(returnTemperatureSensor.getMinTemperature());
    display.print(" ");
    display.print(returnTemperatureSensor.getMaxTemperature());
    display.println(" C");

    display.print("LP:");
    display.print(pulses);
    display.print(" LFR:");
    display.print(flowSensor.pulsesToGallons(pulses * (60000.0f / float(timeSinceLastReading))));
    display.println();

    display.print("CFT:");
    display.print(flowSensor.getCurrentFlowTimeMillis() / 60000);
    display.print(" CFG:");
    display.print(flowSensor.pulsesToGallons(flowSensor.getCurrentFlowPulses()));
    display.println();

    display.print("TSFC:");
    display.print(flowSensor.getTotalSignificantFlowCount());
    display.print(" TFC:");
    display.print(flowSensor.getTotalFlowCount());
    display.println();

    display.print("TFT:");
    display.print(flowSensor.getTotalFlowMillis() / 60000);
    display.print(" TFG:");
    display.print(flowSensor.pulsesToGallons(flowSensor.getTotalFlowPulses()));
    display.println();

    display.print("P:");
    display.print(pumpPowerRelay.isOn()? "on" : "off");
    display.print(' ');
    display.print(timeSinceLastPumpToggle / 1000);
    display.print("s ");
    display.print(tempDiff);
    display.println("C");

    display.print("PC:");
    display.print(pumpPowerRelay.getTotalOnCount());
    display.print(" PD:");
    display.print(pumpPowerRelay.getTotalOnDuration() / 60000);
    display.print(" PT:");
    display.print(missedTempTargetCount);

    display.display();
  }
  delay(10);
}
