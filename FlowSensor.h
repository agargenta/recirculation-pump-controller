#ifndef FLOW_SENSOR_H
#define FLOW_SENSOR_H
#define FLOW_SENSOR_PULSES_PER_GALLON 1380
#define FLOW_SENSOR_MIN_SIGNIFICANT_PULSES 20

#include <Arduino.h>

class FlowSensor {
  
  private:
    byte pin;
    volatile byte pulses = 0;
    boolean currentFlowSignificant = false;
    unsigned long currentFlowPulses = 0; // gets reset any time the flow stops
    unsigned long currentFlowTimeMillis = 0; // gets reset any time the flow stops
    unsigned long totalPulses = 0;
    unsigned long totalFlowTimeMillis = 0;
    unsigned long totalFlowCount = 0;
    unsigned long totalSignificantFlowCount = 0;

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
     * At 1HZ, this sensor will be able to support flow up to ~11 gpm.
     * 
     * @param period millis since the last time this sensor was serviced.
     * @return the number of pulses detected during this period, possibly zero.
     */
    byte service(unsigned long period);

    /**
     * Check if this sensor is flowing. Only significant flow is reported (>= 1s) in order to ignore sensor noise.
     * 
     * @return true if a flow is detected at this sensor and false otherwise.
     */
    boolean isFlowing();

    /**
     * Convert the number of pulses to gallons.
     * @param pulses the pulses to convert.
     * @return gallons-equivalent.
     */
    float pulsesToGallons(unsigned long pulses);

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
     * Get the total number of pulses detected (including noise) since this sensor was initialized.
     * 
     * @return the total number of pulses detected (including noise) since this sensor was initialized.
     */
    unsigned long getTotalFlowPulses();

    /**  
     * Get the total time duration when flow was detected at this sensor (including noise).
     * 
     * @return the total time duration when flow was detected at this sensor (including noise).
     */
    unsigned long getTotalFlowMillis();

    /**
     * Get the total number of times any flow (including noise) was detected at this sensor.
     * 
     * @return the total number of times any flow (including noise) was detected at this sensor. Continuous flow is counted only once.
     */
    unsigned long getTotalFlowCount();

    /**
     * Get the total number of times any significant flow was detected at this sensor.
     * 
     * @return the total number of times any significant flow was detected at this sensor. Continuous flow is counted only once.
     */    
    unsigned long getTotalSignificantFlowCount();
};

#endif
