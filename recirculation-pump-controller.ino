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
#define MIN_PUMP_OFF_DURATION       180000 //  3 minutes
#define MIN_PUMP_ON_DURATION        300000 //  5 minutes
#define MAX_PUMP_ON_DURATION        600000 // 10 minutes

TemperatureSensor outletTemperatureSensor(OUTLET_TEMPERATURE_SENSOR_PIN);
TemperatureSensor returnTemperatureSensor(RETURN_TEMPERATURE_SENSOR_PIN);
FlowSensor flowSensor(FLOW_SENSOR_PIN);
PowerRelay pumpPowerRelay(PUMP_POWER_RELAY_PIN);
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long lastSensorsReadingTime;
unsigned int missedTempTargetCount = 0;
unsigned long totalWarmPulses = 0;

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
  const unsigned long now = millis();
  const unsigned long timeSinceLastReading = now - lastSensorsReadingTime;
  if (timeSinceLastReading >= (1000 / SENSORS_READING_FREQUENCY_HZ)) {
    lastSensorsReadingTime = now;
   
    // read temperature sensors
    const float outletTemperature = outletTemperatureSensor.readTemperature();
    const float returnTemperature = returnTemperatureSensor.readTemperature();
    // TODO: check for temperature errors
    const float tempDiff = outletTemperature - returnTemperature;
    
    // service flow sensor
    const byte pulses = flowSensor.service(timeSinceLastReading);

    const unsigned long timeSinceLastPumpToggle = now - pumpPowerRelay.getLastToggleTimestamp();

    if (
      flowSensor.isFlowing() &&                                 // if the water is flowing AND
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

    // if the water if flowing and it's warm enough
    if (flowSensor.isFlowing() && tempDiff <= MIN_TEMP_DIFF) {
      totalWarmPulses += pulses;
    }

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
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

    display.print("F:");
    display.print(flowSensor.isFlowing() ? "yes" : "no");
    display.print(' ');
    display.print(pulses);
    display.print("p ");
    display.print(flowSensor.pulsesToGallons(pulses * (60000.0f / float(timeSinceLastReading))));
    display.print("gpm");
    display.println();

    display.print("Fc:");
    display.print(flowSensor.getCurrentFlowPulses());
    display.print("p ");
    display.print(flowSensor.getCurrentFlowTimeMillis() / 60000);
    display.print("m ");
    display.print(flowSensor.pulsesToGallons(flowSensor.getCurrentFlowPulses()));
    display.print('g');
    display.println();

    display.print("Ft:");
    display.print(flowSensor.getTotalFlowCount());
    display.print("c ");
    display.print(flowSensor.getTotalFlowMillis() / 60000);
    display.print("m ");
    display.print(flowSensor.pulsesToGallons(flowSensor.getTotalFlowPulses()));
    display.print('g');
    display.println();

    display.print("P:");
    display.print(pumpPowerRelay.isOn()? "on" : "off");
    display.print(' ');
    display.print(timeSinceLastPumpToggle / 1000);
    display.print("s ");
    display.print(tempDiff);
    display.print('C');
    display.println();

    display.print("Pt:");
    display.print(pumpPowerRelay.getTotalOnCount());
    display.print("c ");
    display.print(pumpPowerRelay.getTotalOnDuration() / 60000);
    display.print("m ");
    display.print(pumpPowerRelay.getTotalOnDuration() * 100.00f / now);
    display.print('%');
    display.println();
    
    display.print("Ps:");
    display.print(pumpPowerRelay.getTotalOnCount() == 0? 0 : pumpPowerRelay.getTotalOnDuration() / 60000 / float(pumpPowerRelay.getTotalOnCount()));
    display.print("m~ ");
    display.print(missedTempTargetCount);
    display.print("c ");
    display.print(flowSensor.getTotalFlowPulses() == 0? 0 : totalWarmPulses * 100.00f / flowSensor.getTotalFlowPulses());
    display.print('%');

    display.display();
  }
  delay(10);
}
