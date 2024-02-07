/*!
 * @file Adafruit_VCNL4020.h
 *
 * This is the documentation for Adafruit's VCNL4020 driver for the
 * Arduino platform. It is designed specifically to work with the
 * Adafruit VCNL4020 breakout: https://www.adafruit.com/product/XYZ
 *
 * These sensors use I2C to communicate, 2 pins (SCL+SDA) are required
 * to interface with the board.
 *
 * Adafruit invests time and resources providing this open-source code.
 * Please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by [Your Name Here].
 *
 * MIT license, all text here must be included in any redistribution.
 *
 */

#ifndef ADAFRUIT_VCNL4020_H
#define ADAFRUIT_VCNL4020_H

#include "Arduino.h"
#include <Adafruit_BusIO_Register.h>

#define VCNL4020_I2C_ADDRESS 0x13 ///< The address is fixed

///< VCNL4020 Register Definitions
#define VCNL4020_REG_COMMAND 0x80 ///< Register #0 Command Register
#define VCNL4020_REG_PRODUCT_ID                                                \
  0x81 ///< Register #1 Product ID Revision Register
#define VCNL4020_REG_PROX_RATE                                                 \
  0x82 ///< Register #2 Rate of Proximity Measurement
#define VCNL4020_REG_IR_LED_CURRENT                                            \
  0x83 ///< Register #3 (Not explicitly named in datasheet)
#define VCNL4020_REG_AMBIENT_PARAM                                             \
  0x84 ///< Register #4 Ambient Light Parameter Register
#define VCNL4020_REG_AMBIENT_RESULT_HIGH                                       \
  0x85 ///< Register #5 Ambient Light Result High Byte
#define VCNL4020_REG_AMBIENT_RESULT_LOW                                        \
  0x86 ///< Register #6 Ambient Light Result Low Byte
#define VCNL4020_REG_PROX_RESULT_HIGH                                          \
  0x87 ///< Register #7 Proximity Result High Byte
#define VCNL4020_REG_PROX_RESULT_LOW                                           \
  0x88                             ///< Register #8 Proximity Result Low Byte
#define VCNL4020_REG_INT_CTRL 0x89 ///< Register #9 Interrupt Control Register
#define VCNL4020_REG_LOW_THRES_HIGH                                            \
  0x8A ///< Register #10 Low Threshold High Byte
#define VCNL4020_REG_LOW_THRES_LOW 0x8B ///< Register #11 Low Threshold Low Byte
#define VCNL4020_REG_HIGH_THRES_HIGH                                           \
  0x8C ///< Register #12 High Threshold High Byte
#define VCNL4020_REG_HIGH_THRES_LOW                                            \
  0x8D                               ///< Register #13 High Threshold Low Byte
#define VCNL4020_REG_INT_STATUS 0x8E ///< Register #14 Interrupt status register
#define VCNL4020_REG_PROX_ADJUST                                               \
  0x8F ///< Register #15 Proximity adjustment register

// clang-format off

/** The measurements-per-second for automatic proximity sensing */
typedef enum {
  PROX_RATE_1_95_PER_S = 0x00, ///< 1.95 measurements/s
  PROX_RATE_3_9_PER_S = 0x01,  ///< 3.90625 measurements/s
  PROX_RATE_7_8_PER_S = 0x02,  ///< 7.8125 measurements/s
  PROX_RATE_16_6_PER_S = 0x03, ///< 16.625 measurements/s
  PROX_RATE_31_2_PER_S = 0x04, ///< 31.25 measurements/s
  PROX_RATE_62_5_PER_S = 0x05, ///< 62.5 measurements/s
  PROX_RATE_125_PER_S = 0x06,  ///< 125 measurements/s
  PROX_RATE_250_PER_S = 0x07   ///< 250 measurements/s
} vcnl4020_proxrate;

/** The measurements-per-second for automatic ambient sensing */
typedef enum {
  AMBIENT_RATE_1_SPS = 0x00, ///< 1 samples/s
  AMBIENT_RATE_2_SPS = 0x01, ///< 2 samples/s (DEFAULT)
  AMBIENT_RATE_3_SPS = 0x02, ///< 3 samples/s
  AMBIENT_RATE_4_SPS = 0x03, ///< 4 samples/s
  AMBIENT_RATE_5_SPS = 0x04, ///< 5 samples/s
  AMBIENT_RATE_6_SPS = 0x05, ///< 6 samples/s
  AMBIENT_RATE_8_SPS = 0x06, ///< 8 samples/s
  AMBIENT_RATE_10_SPS = 0x07 ///< 10 samples/s
} vcnl4020_ambientrate;

/** How many samples to average together per reading */
typedef enum {
  AVG_1_SAMPLES = 0x00,  ///< 2^0 = 1 sample
  AVG_2_SAMPLES = 0x01,  ///< 2^1 = 2 samples
  AVG_4_SAMPLES = 0x02,  ///< 2^2 = 4 samples
  AVG_8_SAMPLES = 0x03,  ///< 2^3 = 8 samples
  AVG_16_SAMPLES = 0x04, ///< 2^4 = 16 samples
  AVG_32_SAMPLES = 0x05, ///< 2^5 = 32 samples
  AVG_64_SAMPLES = 0x06, ///< 2^6 = 64 samples
  AVG_128_SAMPLES = 0x07 ///< 2^7 = 128 samples
} vcnl4020_averaging;

/** How many out-of-bounds measurements before we trigger an IRQ */
typedef enum {
  INT_COUNT_1 = 0x00,  ///< 1 count (DEFAULT)
  INT_COUNT_2 = 0x01,  ///< 2 count
  INT_COUNT_4 = 0x02,  ///< 4 count
  INT_COUNT_8 = 0x03,  ///< 8 count
  INT_COUNT_16 = 0x04, ///< 16 count
  INT_COUNT_32 = 0x05, ///< 32 count
  INT_COUNT_64 = 0x06, ///< 64 count
  INT_COUNT_128 = 0x07 ///< 128 count
} vcnl4020_int_count;

/** Advanced usage adjustable proximity squarewave carrier */
typedef enum {
  PROX_FREQ_390_625_KHZ = 0x00, ///< 390.625 kHz (DEFAULT)
  PROX_FREQ_781_25_KHZ = 0x01,  ///< 781.25 kHz
  PROX_FREQ_1_5625_MHZ = 0x02,  ///< 1.5625 MHz
  PROX_FREQ_3_125_MHZ = 0x03    ///< 3.125 MHz
} vcnl4020_proxfreq;

#define VCNL4020_INT_TH_HI      0x01 ///< High threshold exceed
#define VCNL4020_INT_TH_LOW     0x02 ///< Low threshold exceed
#define VCNL4020_INT_ALS_READY  0x04 ///< ALS ready
#define VCNL4020_INT_PROX_READY 0x08 ///< Proximity ready

// clang-format on

/*!
 * @brief Class that stores state and functions for interacting with VCNL4020
 * sensor.
 */
class Adafruit_VCNL4020 {
public:
  Adafruit_VCNL4020();
  bool begin(TwoWire *theWire = &Wire, uint8_t addr = VCNL4020_I2C_ADDRESS);

  // Command Register Functions
  void setOnDemand(bool als, bool prox);
  void enable(bool als, bool prox, bool selftimed);

  // Product ID Revision Register Function
  uint8_t getProdRevision();

  // Proximity Measurement Rate Functions
  void setProxRate(vcnl4020_proxrate rate);
  vcnl4020_proxrate getProxRate();
  void setProxFrequency(vcnl4020_proxfreq freq);
  vcnl4020_proxfreq getProxFrequency();

  // LED Current Setting for Proximity Mode Functions
  void setProxLEDmA(uint8_t LEDmA);
  uint8_t getProxLEDmA();

  // Ambient Light Parameter Register Functions
  void setContinuousConversion(bool continuous);
  void setAutoOffsetComp(bool autoOffset);

  // Ambient Light Result Register Function
  bool isAmbientReady();
  uint16_t readAmbient();
  void setAmbientRate(vcnl4020_ambientrate rate);
  vcnl4020_ambientrate getAmbientRate();
  void setAmbientAveraging(vcnl4020_averaging avg);
  vcnl4020_averaging getAmbientAveraging();

  // Proximity Measurement Result Register Function
  uint16_t readProximity();
  bool isProxReady();

  // Low and High Threshold Functions
  void setLowThreshold(uint16_t threshold);
  uint16_t getLowThreshold();
  void setHighThreshold(uint16_t threshold);
  uint16_t getHighThreshold();

  // Interrupt Control Register Function
  void setInterruptConfig(bool proxReady, bool alsReady, bool thresh,
                          bool threshALS, vcnl4020_int_count intCount);
  uint8_t getInterruptStatus();
  void clearInterrupts(bool proxready, bool alsready, bool th_low,
                       bool th_high);

private:
  Adafruit_I2CDevice *_i2c = NULL;
};

#endif // ADAFRUIT_VCNL4020_H
