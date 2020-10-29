#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "TemperatureSensor.h"
#include "FlowSensor.h"
#include "PowerRelay.h"


#define OUTLET_TEMPERATURE_SENSOR_PIN   11
#define RETURN_TEMPERATURE_SENSOR_PIN   12
#define FLOW_SENSOR_PIN                  2
#define SENSORS_READING_FREQUENCY_HZ     1
#define OLED_SCREEN_WIDTH              128
#define OLED_SCREEN_HEIGHT              64
#define OLED_RESET                       4

TemperatureSensor outletTemperatureSensor(OUTLET_TEMPERATURE_SENSOR_PIN);
TemperatureSensor returnTemperatureSensor(RETURN_TEMPERATURE_SENSOR_PIN);
FlowSensor flowSensor(FLOW_SENSOR_PIN);
Adafruit_SSD1306 display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, OLED_RESET);
unsigned long lastSensorsReadingTime;

void flowSensorIsr() {
  flowSensor.onPulse();
}

void setup() {
//  Serial.begin(9600);

  // set up temperature sensors
  outletTemperatureSensor.begin();
  returnTemperatureSensor.begin();

  // set up flow sensor
  flowSensor.begin();
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), flowSensorIsr, FALLING);

  // set up display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed. Can't set up the display. Aborting setup."));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
}

void loop() {
  unsigned long currentTime = millis();
  unsigned long timeSinceLastReading = currentTime - lastSensorsReadingTime;
  if (timeSinceLastReading >= (1000 / SENSORS_READING_FREQUENCY_HZ)) {
    lastSensorsReadingTime = currentTime;
   
    // read temperature sensors
    float outletTemperature = outletTemperatureSensor.readTemperature();
    float returnTemperature = returnTemperatureSensor.readTemperature();

    // service flow sensor
    byte pulses = flowSensor.service(timeSinceLastReading);

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

    display.display();
  }
  delay(10);
}
