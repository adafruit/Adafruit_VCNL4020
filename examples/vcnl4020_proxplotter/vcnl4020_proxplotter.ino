#include <Wire.h>
#include "Adafruit_VCNL4020.h"

// This example is pared down specifically to make it easier to test using the serial plotter

Adafruit_VCNL4020 vcnl4020;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10); // wait for serial port to start.
  
  Serial.println("Adafruit VCNL4020 Library Test Sketch");

  // Initialize sensor
  if (!vcnl4020.begin(&Wire)) {
    Serial.println("Failed to initialize VCNL4020!");
    while (1);
  }
  Serial.println("VCNL4020 initialized.");

  // To set up all the configuration, first disable everything
  vcnl4020.enable(false /* ALS Enable */, false /* Proximity Enable */, false /* Self-Timed Enable */);
  vcnl4020.setOnDemand(false /* ALS on demand read */, false /* Prox on demand read */);  

  // Try different proximity rates, faster measurements use more power of course!
  vcnl4020.setProxRate(PROX_RATE_250_PER_S);
  
  // Set LED Current (Valid range: 0-200 mA), higher currents require
  // more power but will let you detect farther.
  vcnl4020.setProxLEDmA(200);

  // Setup IRQ pin output when proximity data is ready
  vcnl4020.setInterruptConfig(
    true /* Proximity Ready */, 
    false /* ALS Ready */, 
    false /* Threshold */, 
    false /* true = Threshold ALS, false = Threshold Proximity */, 
    INT_COUNT_1 /* how many values before the INT fires */
  );
  
  // finally, we can set up all the sensors we want to use.
  vcnl4020.enable(false /* ALS Enable */, true /* Proximity Enable */, true /* Self-Timed Enable */);
  // dont use on-demand, we will have continuous reads.
  vcnl4020.setOnDemand(false /* ALS on demand read */, false /* Prox on demand read */);
}

void loop() {
  if (vcnl4020.isProxReady()) {
    uint16_t p = vcnl4020.readProximity();
    if (p != 0xFFFF) {  // ignore spurious readings
      Serial.print("Prox: ");
      Serial.println(p);
      
    }
    vcnl4020.clearInterrupts( // Clear Interrupts
      true /* Proximity Ready */, 
      false /* ALS Ready */, 
      false /* Threshold Low */, 
      false /* Threshold High */
    );
  }
}
