#include <Wire.h>
#include "Adafruit_VCNL4020.h"

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

  // Verify product ID & revision
  Serial.print("VCNL4020 Product ID & Revision (should be 0x21): ");
  Serial.println(vcnl4020.getProdRevision(), HEX);

  // To set up all the configuration, first disable everything
  vcnl4020.enable(false /* ALS Enable */, false /* Proximity Enable */, false /* Self-Timed Enable */);
  vcnl4020.setOnDemand(false /* ALS on demand read */, false /* Prox on demand read */);  

  // Try different proximity rates, faster measurements use more power of course!
  // Can also try: PROX_RATE_1_95_PER_S, PROX_RATE_3_9_PER_S, PROX_RATE_7_8_PER_S, 
  // PROX_RATE_16_6_PER_S, PROX_RATE_31_2_PER_S, PROX_RATE_62_5_PER_S, 
  // PROX_RATE_125_PER_S, PROX_RATE_250_PER_S
  vcnl4020.setProxRate(PROX_RATE_16_6_PER_S);
  Serial.print("Proximity Rate: ");
  switch (vcnl4020.getProxRate()) {
    case PROX_RATE_1_95_PER_S: Serial.println("1.95 measurements/s"); break;
    case PROX_RATE_3_9_PER_S: Serial.println("3.9 measurements/s"); break;
    case PROX_RATE_7_8_PER_S: Serial.println("7.8 measurements/s"); break;
    case PROX_RATE_16_6_PER_S: Serial.println("16.6 measurements/s"); break;
    case PROX_RATE_31_2_PER_S: Serial.println("31.2 measurements/s"); break;
    case PROX_RATE_62_5_PER_S: Serial.println("62.5 measurements/s"); break;
    case PROX_RATE_125_PER_S: Serial.println("125 measurements/s"); break;
    case PROX_RATE_250_PER_S: Serial.println("250 measurements/s"); break;
  }
  
  // Test LED Current (Valid range: 0-200 mA), higher currents require
  // more power but will let you detect farther.
  vcnl4020.setProxLEDmA(200);
  Serial.print("Proximity LED Current (mA): ");
  Serial.println(vcnl4020.getProxLEDmA());


  // Test Ambient Rate
  // Can also try: AMBIENT_RATE_1_SPS, AMBIENT_RATE_2_SPS, AMBIENT_RATE_3_SPS, 
  // AMBIENT_RATE_4_SPS, AMBIENT_RATE_5_SPS, AMBIENT_RATE_6_SPS, AMBIENT_RATE_8_SPS, 
  // AMBIENT_RATE_10_SPS
  vcnl4020.setAmbientRate(AMBIENT_RATE_10_SPS); 
  Serial.print("Ambient Rate: ");
  switch (vcnl4020.getAmbientRate()) {
    case AMBIENT_RATE_1_SPS: Serial.println("1 samples/s"); break;
    case AMBIENT_RATE_2_SPS: Serial.println("2 samples/s"); break;
    case AMBIENT_RATE_3_SPS: Serial.println("3 samples/s"); break;
    case AMBIENT_RATE_4_SPS: Serial.println("4 samples/s"); break;
    case AMBIENT_RATE_5_SPS: Serial.println("5 samples/s"); break;
    case AMBIENT_RATE_6_SPS: Serial.println("6 samples/s"); break;
    case AMBIENT_RATE_8_SPS: Serial.println("8 samples/s"); break;
    case AMBIENT_RATE_10_SPS: Serial.println("10 samples/s"); break;
  }

  // Test Ambient Averaging
  // Can also try: AVG_1_SAMPLES, AVG_2_SAMPLES, AVG_4_SAMPLES, 
  // AVG_8_SAMPLES, AVG_16_SAMPLES, AVG_32_SAMPLES, AVG_64_SAMPLES, 
  // AVG_128_SAMPLES

  vcnl4020.setAmbientAveraging(AVG_1_SAMPLES); 
  Serial.print("Ambient Averaging: ");
  switch (vcnl4020.getAmbientAveraging()) {
    case AVG_1_SAMPLES: Serial.println("1 sample"); break;
    case AVG_2_SAMPLES: Serial.println("2 samples"); break;
    case AVG_4_SAMPLES: Serial.println("4 samples"); break;
    case AVG_8_SAMPLES: Serial.println("8 samples"); break;
    case AVG_16_SAMPLES: Serial.println("16 samples"); break;
    case AVG_32_SAMPLES: Serial.println("32 samples"); break;
    case AVG_64_SAMPLES: Serial.println("64 samples"); break;
    case AVG_128_SAMPLES: Serial.println("128 samples"); break;
  }

  // Setup IRQ pin output when proximity data is ready
  vcnl4020.setInterruptConfig(
    true /* Proximity Ready */, 
    false /* ALS Ready */, 
    false /* Threshold */, 
    false /* true = Threshold ALS, false = Threshold Proximity */, 
    INT_COUNT_1 /* how many values before the INT fires */
  );
  
  // The proximity measurement is using a square IR signal as 
  // measurement signal. Four different values are
  // possible: PROX_FREQ_390_625_KHZ, PROX_FREQ_781_25_KHZ, 
  // PROX_FREQ_1_5625_MHZ, PROX_FREQ_3_125_MHZ
  vcnl4020.setProxFrequency(PROX_FREQ_390_625_KHZ); 
  Serial.print("Proximity Frequency: ");
  switch (vcnl4020.getProxFrequency()) {
    case PROX_FREQ_390_625_KHZ: Serial.println("390.625 KHz"); break;
    case PROX_FREQ_781_25_KHZ: Serial.println("781.25 KHz"); break;
    case PROX_FREQ_1_5625_MHZ: Serial.println("1.5625 MHz"); break;
    case PROX_FREQ_3_125_MHZ: Serial.println("3.125 MHz"); break;
  }

  
  // finally, we can set up all the sensors we want to use. for example:
  // enable light sensor and proximity sensor, we will also use 'self timed' mode, which means 
  // measurements are taken repeatedly for us and we dont have to do an 'on demand' measurement
  // request (simpler for getting started)
  vcnl4020.enable(true /* ALS Enable */, true /* Proximity Enable */, true /* Self-Timed Enable */);
  // dont use on-demand, we will have continuous reads.
  vcnl4020.setOnDemand(false /* ALS on demand read */, false /* Prox on demand read */);
}

void loop() {
  if (vcnl4020.isProxReady()) {
    Serial.print("Prox: ");
    Serial.println(vcnl4020.readProximity());
    vcnl4020.clearInterrupts( // Clear Interrupts
      true /* Proximity Ready */, 
      false /* ALS Ready */, 
      false /* Threshold Low */, 
      false /* Threshold High */
    );
  }

  if (vcnl4020.isAmbientReady()) {
    Serial.print("Ambient: ");
    Serial.print(vcnl4020.readAmbient());
    Serial.print(", ");
    vcnl4020.clearInterrupts( // Clear Interrupts
      false /* Proximity Ready */, 
      true /* ALS Ready */, 
      false /* Threshold Low */, 
      false /* Threshold High */
    );
  }
}
