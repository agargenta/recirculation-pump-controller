#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H
#define FLOW_SENSOR_PULSES_PER_GALLON 1380
#define FLOW_SENSOR_MIN_PULSES_FOR_FLOW_DETECTION 20

#include <Arduino.h>

class FlowSensor {
  
  private:
    byte pin;
    volatile byte pulses = 0;
    boolean currentFlowDetected = false;
    unsigned long currentFlowPulses = 0; // gets reset any time the flow stops
    unsigned long currentFlowTimeMillis = 0; // gets reset any time the flow stops
    unsigned long totalPulses = 0;
    unsigned long totalFlowTimeMillis = 0;
    unsigned long totalFlowCount = 0;

  public:
    
    /**
     * Constructor. Requires an interrupt hanlder to be configured separately. See onPulse() below.
     * 
     * @param pin the pin this sensor will read from.
     */
    FlowSensor(byte pin);

    /**
     * Setup initializer.
     */
    void begin();

    /**
     * To be called by an interrupt handler routine whenever this sensor is FALLING.
     */
    void onPulse();

    /**
     * Call to service this sensor. This method needs to be invoked periodically in order for the sensor to work accurately.
     * At 1HZ, this sensor will be able to support flows up to ~11 gpm.
     * 
     * @param  period time in millis since the this sensor was last serviced.
     * @return the number of pulses detected during this period, possibly zero. 
     *         This number may include sensor noise, so values > 0 do not reliably indicate if the flow has been detected.
     */
    byte service(unsigned long period);

    /**
     * Check if this sensor is flowing. Only significant flow is reported (>= 1s) in order to ignore sensor noise.
     * 
     * @return true if a flow is detected at this sensor and false otherwise.
     */
    boolean isFlowing();

    /**
     * Get the number of pulses since the last detected flow started.
     * 
     * @return the number of pulses since the last detected flow started.
     */
    unsigned long getCurrentFlowPulses();

    /**  
     * Get the time duration since the last detected flow started.
     * 
     * @return the time duration since the last detected flow started, in millis.
     */
    unsigned long getCurrentFlowTimeMillis();

    /**
     * Get the total number of pulses detected (excluding noise) since this sensor was initialized.
     * 
     * @return the total number of pulses detected (excluding noise) since this sensor was initialized.
     */
    unsigned long getTotalFlowPulses();

    /**  
     * Get the total time duration when flow was detected at this sensor (excluding noise).
     * 
     * @return the total time duration when flow was detected at this sensor (excluding noise).
     */
    unsigned long getTotalFlowMillis();

    /**
     * Get the total number of times any continuous flow (excluding noise) was detected at this sensor.
     * 
     * @return the total number of times any continuous flow (excluding noise) was detected at this sensor. Each continuous flow is counted only once.
     */
    unsigned long getTotalFlowCount();

    /**
     * Convert the number of pulses to gallons.
     * @param pulses the pulses to convert.
     * @return gallons-equivalent.
     */
    float pulsesToGallons(unsigned long pulses);
};

#endif
