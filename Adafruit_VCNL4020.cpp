/*!
 * @file Adafruit_VCNL4020.cpp
 *
 * @mainpage Adafruit VCNL4020 Proximity/Ambient Light sensor driver
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's VCNL4020 driver for the
 * Arduino platform. It is designed specifically to work with the
 * Adafruit VCNL4020 breakout: https://www.adafruit.com/product/XXXX
 *
 * @section dependencies Dependencies
 *
 * This library depends on the Adafruit BusIO library.
 *
 * @section author Author
 *
 * Written by ladyada.
 *
 * @section license License
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Adafruit_VCNL4020.h>

/*!
 * @brief  Constructs an Adafruit_VCNL4020 object.
 */
Adafruit_VCNL4020::Adafruit_VCNL4020() {
  // Empty constructor; initialization will be done in the begin() method
}

/*!
 * @brief  Initializes the VCNL4020 sensor and checks for a valid Product ID
 * Revision.
 * @param  theWire  The I2C interface to use, defaults to Wire.
 * @param  addr     The I2C address of the VCNL4020, defaults to
 * VCNL4020_I2C_ADDRESS.
 * @return True if initialization was successful and Product ID Revision is
 * correct, otherwise False.
 */
bool Adafruit_VCNL4020::begin(TwoWire *theWire, uint8_t addr) {
  // Initialize the I2C interface
  if (_i2c)
    delete _i2c;

  _i2c = new Adafruit_I2CDevice(addr, theWire);

  // Try to initialize I2C
  bool found = false;
  for (uint8_t retries = 0; retries < 5; retries++) {
    if (_i2c->begin()) {
      found = true;
      break;
    }
    delay(10);
  }
  if (!found)
    return false;

  // Check the Product ID Revision
  uint8_t prodRev = getProdRevision();
  if (prodRev != 0x21) {
    return false;
  }

  // To set up all the configuration, first disable everything
  enable(false /* ALS Enable */, false /* Proximity Enable */,
         false /* Self-Timed Enable */);
  setOnDemand(false /* ALS on demand read */, false /* Prox on demand read */);
  // set fastest rate so folks see stuff, can always config lower power later
  setProxRate(PROX_RATE_250_PER_S);
  setProxLEDmA(200);
  setAmbientRate(AMBIENT_RATE_10_SPS);
  setAmbientAveraging(AVG_1_SAMPLES);

  // default IRQ on data ready
  setInterruptConfig(
      true /* Proximity Ready */, true /* ALS Ready */, false /* Threshold */,
      false /* true = Threshold ALS, false = Threshold Proximity */,
      INT_COUNT_1 /* how many values before the INT fires */
  );

  // default freq
  setProxFrequency(PROX_FREQ_390_625_KHZ);

  enable(true /* ALS Enable */, true /* Proximity Enable */,
         true /* Self-Timed Enable */);

  return true;
}

/*!
 * @brief  Checks if the Ambient Light Sensor data is ready.
 * @return True if ALS data is ready, otherwise false.
 */
bool Adafruit_VCNL4020::isAmbientReady() {
  // Create an I2C Register object for COMMAND REGISTER #0
  Adafruit_BusIO_Register command_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_COMMAND);

  // Create a bit accessor for bit #6 (als_data_rdy)
  Adafruit_BusIO_RegisterBits als_data_rdy =
      Adafruit_BusIO_RegisterBits(&command_reg, 1, 6); // 1 bit at position 6

  // Read the bit and return its value
  return als_data_rdy.read();
}

/*!
 * @brief  Checks if the Proximity data is ready.
 * @return True if Proximity data is ready, otherwise false.
 */
bool Adafruit_VCNL4020::isProxReady() {
  // Create an I2C Register object for COMMAND REGISTER #0
  Adafruit_BusIO_Register command_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_COMMAND);

  // Create a bit accessor for bit #5 (prox_data_rdy)
  Adafruit_BusIO_RegisterBits prox_data_rdy =
      Adafruit_BusIO_RegisterBits(&command_reg, 1, 5); // 1 bit at position 5

  // Read the bit and return its value
  return prox_data_rdy.read();
}

/*!
 * @brief  Sets the on-demand bits for ALS and Proximity measurements.
 * @param  als  True to set the ALS on-demand bit, otherwise false.
 * @param  prox True to set the Proximity on-demand bit, otherwise false.
 */
void Adafruit_VCNL4020::setOnDemand(bool als, bool prox) {
  // Create an I2C Register object for COMMAND REGISTER #0
  Adafruit_BusIO_Register command_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_COMMAND);

  // Create bit accessors for bit #4 (als_od) and bit #3 (prox_od)
  Adafruit_BusIO_RegisterBits als_od =
      Adafruit_BusIO_RegisterBits(&command_reg, 1, 4); // 1 bit at position 4
  Adafruit_BusIO_RegisterBits prox_od =
      Adafruit_BusIO_RegisterBits(&command_reg, 1, 3); // 1 bit at position 3

  // Set the bits based on the arguments
  als_od.write(als);
  prox_od.write(prox);
}

/*!
 * @brief  Enables or disables the ALS, Proximity, and Self-Timed measurements.
 * @param  als        True to enable the ALS, otherwise false.
 * @param  prox       True to enable the Proximity, otherwise false.
 * @param  selftimed  True to enable the Self-Timed measurements, otherwise
 * false.
 */
void Adafruit_VCNL4020::enable(bool als, bool prox, bool selftimed) {
  // Create an I2C Register object for COMMAND REGISTER #0
  Adafruit_BusIO_Register command_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_COMMAND);

  // Create bit accessors for bit #2 (als_en), bit #1 (prox_en), and bit #0
  // (selftimed_en)
  Adafruit_BusIO_RegisterBits als_en =
      Adafruit_BusIO_RegisterBits(&command_reg, 1, 2); // 1 bit at position 2
  Adafruit_BusIO_RegisterBits prox_en =
      Adafruit_BusIO_RegisterBits(&command_reg, 1, 1); // 1 bit at position 1
  Adafruit_BusIO_RegisterBits selftimed_en =
      Adafruit_BusIO_RegisterBits(&command_reg, 1, 0); // 1 bit at position 0

  // Set the bits based on the arguments
  als_en.write(als);
  prox_en.write(prox);
  selftimed_en.write(selftimed);
}

/*!
 * @brief  Gets the Product ID Revision from Register #1.
 * @return 8-bit value representing the Product ID Revision.
 */
uint8_t Adafruit_VCNL4020::getProdRevision() {
  // Create an I2C Register object for Product ID Revision Register (Register
  // #1)
  Adafruit_BusIO_Register prod_id_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_PRODUCT_ID);

  // Read the 8-bit value from the register and return it
  return prod_id_reg.read();
}

/*!
 * @brief  Sets the Proximity Rate.
 * @param  rate  The rate to set, as defined in the vcnl4020_proxrate enum.
 */
void Adafruit_VCNL4020::setProxRate(vcnl4020_proxrate rate) {
  // Create an I2C Register object for Register #2 (Rate of Proximity
  // Measurement)
  Adafruit_BusIO_Register prox_rate_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_PROX_RATE);

  // Create a bit accessor for the 3-bit Proximity Rate field
  Adafruit_BusIO_RegisterBits prox_rate_bits = Adafruit_BusIO_RegisterBits(
      &prox_rate_reg, 3, 0); // 3 bits starting at position 0

  // Write the 3-bit value to the register
  prox_rate_bits.write(rate);
}

/*!
 * @brief  Gets the current Proximity Rate.
 * @return The current rate, as defined in the vcnl4020_proxrate enum.
 */
vcnl4020_proxrate Adafruit_VCNL4020::getProxRate() {
  // Create an I2C Register object for Register #2 (Rate of Proximity
  // Measurement)
  Adafruit_BusIO_Register prox_rate_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_PROX_RATE);

  // Create a bit accessor for the 3-bit Proximity Rate field
  Adafruit_BusIO_RegisterBits prox_rate_bits = Adafruit_BusIO_RegisterBits(
      &prox_rate_reg, 3, 0); // 3 bits starting at position 0

  // Read the 3-bit value from the register
  return (vcnl4020_proxrate)prox_rate_bits.read();
}

/*!
 * @brief  Sets the LED current for Proximity Mode in mA.
 * @param  LEDmA  The LED current in mA.
 */
void Adafruit_VCNL4020::setProxLEDmA(uint8_t LEDmA) {
  // Create an I2C Register object for Register #3 (LED Current Setting for
  // Proximity Mode)
  Adafruit_BusIO_Register led_current_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_IR_LED_CURRENT);

  // Create a bit accessor for the 6-bit LED current field
  Adafruit_BusIO_RegisterBits led_current_bits = Adafruit_BusIO_RegisterBits(
      &led_current_reg, 6, 0); // 6 bits starting at position 0

  // Divide the LED current by 10 and write the value to the register
  led_current_bits.write(LEDmA / 10);
}

/*!
 * @brief  Gets the LED current for Proximity Mode in mA.
 * @return The LED current in mA.
 */
uint8_t Adafruit_VCNL4020::getProxLEDmA() {
  // Create an I2C Register object for Register #3 (LED Current Setting for
  // Proximity Mode)
  Adafruit_BusIO_Register led_current_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_IR_LED_CURRENT);

  // Create a bit accessor for the 6-bit LED current field
  Adafruit_BusIO_RegisterBits led_current_bits = Adafruit_BusIO_RegisterBits(
      &led_current_reg, 6, 0); // 6 bits starting at position 0

  // Read the value from the register and multiply by 10 to get the LED current
  // in mA
  return led_current_bits.read() * 10;
}

/*!
 * @brief  Sets the Continuous Conversion mode for Ambient Light Measurement.
 * This function can be used for performing faster ambient light measurements.
 * This mode should only be used with ambient light on-demand measurements. Do
 * not use with self-timed mode. Please refer to the application information
 * chapter 3.3 for details about this function.
 * @param  enable  True to enable, False to disable.
 */
void Adafruit_VCNL4020::setContinuousConversion(bool enable) {
  // Create an I2C Register object for Register #4 (Ambient Light Parameter)
  Adafruit_BusIO_Register ambient_light_param_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_AMBIENT_PARAM);

  // Create a bit accessor for the Continuous Conversion mode bit (Bit 7)
  Adafruit_BusIO_RegisterBits continuous_conversion_bit =
      Adafruit_BusIO_RegisterBits(&ambient_light_param_reg, 1,
                                  7); // 1 bit at position 7

  // Write the value to the register
  continuous_conversion_bit.write(enable ? 1 : 0);
}

/*!
 * @brief  Sets the Auto Offset Compensation for Ambient Light Measurement.
 * In order to compensate a technology, package or temperature related drift of
 * the ambient light values there is a built in automatic offset compensation
 * function. With active auto offset compensation the offset value is measured
 * before each ambient light measurement and subtracted automatically from
 * actual reading.
 * @param  enable  True to enable, False to disable.
 */
void Adafruit_VCNL4020::setAutoOffsetComp(bool enable) {
  // Create an I2C Register object for Register #4 (Ambient Light Parameter)
  Adafruit_BusIO_Register ambient_light_param_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_AMBIENT_PARAM);

  // Create a bit accessor for the Auto Offset Compensation bit (Bit 3)
  Adafruit_BusIO_RegisterBits auto_offset_comp_bit =
      Adafruit_BusIO_RegisterBits(&ambient_light_param_reg, 1,
                                  3); // 1 bit at position 3

  // Write the value to the register
  auto_offset_comp_bit.write(enable ? 1 : 0);
}

/*!
 * @brief  Sets the Ambient Light Measurement Rate.
 * @param  rate  The rate to set, as defined in the vcnl4020_ambientrate enum.
 */
void Adafruit_VCNL4020::setAmbientRate(vcnl4020_ambientrate rate) {
  // Create an I2C Register object for Register #4 (Ambient Light Parameter)
  Adafruit_BusIO_Register ambient_light_param_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_AMBIENT_PARAM);

  // Create a bit accessor for the 3-bit Ambient Light Measurement Rate field
  // (Bits 6-4)
  Adafruit_BusIO_RegisterBits ambient_rate_bits = Adafruit_BusIO_RegisterBits(
      &ambient_light_param_reg, 3, 4); // 3 bits starting at position 4

  // Write the 3-bit value to the register
  ambient_rate_bits.write(rate);
}

/*!
 * @brief  Gets the current Ambient Light Measurement Rate.
 * @return The current rate, as defined in the vcnl4020_ambientrate enum.
 */
vcnl4020_ambientrate Adafruit_VCNL4020::getAmbientRate() {
  // Create an I2C Register object for Register #4 (Ambient Light Parameter)
  Adafruit_BusIO_Register ambient_light_param_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_AMBIENT_PARAM);

  // Create a bit accessor for the 3-bit Ambient Light Measurement Rate field
  // (Bits 6-4)
  Adafruit_BusIO_RegisterBits ambient_rate_bits = Adafruit_BusIO_RegisterBits(
      &ambient_light_param_reg, 3, 4); // 3 bits starting at position 3

  // Read the 3-bit value from the register
  return (vcnl4020_ambientrate)ambient_rate_bits.read();
}

/*!
 * @brief  Sets the Averaging function for Ambient Light Measurement.
 * @param  avg  The averaging setting to use, as defined in the
 * vcnl4020_averaging enum.
 */
void Adafruit_VCNL4020::setAmbientAveraging(vcnl4020_averaging avg) {
  // Create an I2C Register object for Register #4 (Ambient Light Parameter)
  Adafruit_BusIO_Register ambient_light_param_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_AMBIENT_PARAM);

  // Create a bit accessor for the 3-bit Averaging function field (Bits 2-0)
  Adafruit_BusIO_RegisterBits averaging_bits = Adafruit_BusIO_RegisterBits(
      &ambient_light_param_reg, 3, 0); // 3 bits starting at position 0

  // Write the 3-bit value to the register
  averaging_bits.write(avg);
}

/*!
 * @brief  Gets the current Averaging function for Ambient Light Measurement.
 * Bit values sets the number of single conversions done during one measurement
 * cycle. Result is the average value of all conversions.
 * @return The current averaging setting, as defined in the vcnl4020_averaging
 * enum.
 */
vcnl4020_averaging Adafruit_VCNL4020::getAmbientAveraging() {
  // Create an I2C Register object for Register #4 (Ambient Light Parameter)
  Adafruit_BusIO_Register ambient_light_param_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_AMBIENT_PARAM);

  // Create a bit accessor for the 3-bit Averaging function field (Bits 2-0)
  Adafruit_BusIO_RegisterBits averaging_bits = Adafruit_BusIO_RegisterBits(
      &ambient_light_param_reg, 3, 0); // 3 bits starting at position 0

  // Read the 3-bit value from the register
  return (vcnl4020_averaging)averaging_bits.read();
}

/*!
 * @brief  Reads the Ambient Light Sensor (ALS) measurement result.
 * @return The 16-bit ALS measurement result.
 */
uint16_t Adafruit_VCNL4020::readAmbient() {
  // Create an I2C Register object for Register #5 and #6 (Ambient Light Result
  // Register) The register is 2 bytes long and MSB-first
  Adafruit_BusIO_Register als_result_reg = Adafruit_BusIO_Register(
      _i2c, VCNL4020_REG_AMBIENT_RESULT_HIGH, 2, MSBFIRST);

  // Read the 16-bit value from the register
  return als_result_reg.read();
}

/*!
 * @brief  Reads the Proximity Measurement Result.
 * @return The 16-bit Proximity Measurement Result.
 */
uint16_t Adafruit_VCNL4020::readProximity() {
  // Create an I2C Register object for Register #7 and #8 (Proximity Measurement
  // Result Register) The register is 2 bytes long and MSB-first
  Adafruit_BusIO_Register proximity_result_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_PROX_RESULT_HIGH, 2, MSBFIRST);

  // Read the 16-bit value from the register
  return proximity_result_reg.read();
}

/*!
 * @brief  Sets the Low Threshold for Proximity Measurement.
 * @param  threshold  The 16-bit Low Threshold value.
 */
void Adafruit_VCNL4020::setLowThreshold(uint16_t threshold) {
  // Create an I2C Register object for Register #10 and #11 (Low Threshold)
  Adafruit_BusIO_Register low_threshold_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_LOW_THRES_HIGH, 2, MSBFIRST);

  // Write the 16-bit value to the register
  low_threshold_reg.write(threshold);
}

/*!
 * @brief  Gets the Low Threshold for Proximity Measurement.
 * @return The 16-bit Low Threshold value.
 */
uint16_t Adafruit_VCNL4020::getLowThreshold() {
  // Create an I2C Register object for Register #10 and #11 (Low Threshold)
  Adafruit_BusIO_Register low_threshold_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_LOW_THRES_HIGH, 2, MSBFIRST);

  // Read the 16-bit value from the register
  return low_threshold_reg.read();
}

/*!
 * @brief  Sets the High Threshold for Proximity Measurement.
 * @param  threshold  The 16-bit High Threshold value.
 */
void Adafruit_VCNL4020::setHighThreshold(uint16_t threshold) {
  // Create an I2C Register object for Register #12 and #13 (High Threshold)
  Adafruit_BusIO_Register high_threshold_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_HIGH_THRES_HIGH, 2, MSBFIRST);

  // Write the 16-bit value to the register
  high_threshold_reg.write(threshold);
}

/*!
 * @brief  Gets the High Threshold for Proximity Measurement.
 * @return The 16-bit High Threshold value.
 */
uint16_t Adafruit_VCNL4020::getHighThreshold() {
  // Create an I2C Register object for Register #12 and #13 (High Threshold)
  Adafruit_BusIO_Register high_threshold_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_HIGH_THRES_HIGH, 2, MSBFIRST);

  // Read the 16-bit value from the register
  return high_threshold_reg.read();
}

/*!
 * @brief  Sets the Interrupt Configuration for INTERRUPT CONTROL REGISTER #9.
 * @param  proxReady  True to enable Proximity Ready interrupt, False to
 * disable.
 * @param  alsReady   True to enable Ambient Light Sensor Ready interrupt, False
 * to disable.
 * @param  thresh     True to enable Threshold interrupt, False to disable.
 * @param  threshALS  True to enable Threshold ALS interrupt, False to disable.
 * @param  intCount   The interrupt count setting, as defined in the
 * vcnl4020_int_count enum.
 */
void Adafruit_VCNL4020::setInterruptConfig(bool proxReady, bool alsReady,
                                           bool thresh, bool threshALS,
                                           vcnl4020_int_count intCount) {
  // Create an I2C Register object for Register #9 (INTERRUPT CONTROL REGISTER)
  Adafruit_BusIO_Register interrupt_control_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_INT_CTRL);

  // Create bit accessors for each field
  Adafruit_BusIO_RegisterBits int_count_bits =
      Adafruit_BusIO_RegisterBits(&interrupt_control_reg, 3, 5); // Bits 5, 6, 7
  Adafruit_BusIO_RegisterBits prox_ready_bit =
      Adafruit_BusIO_RegisterBits(&interrupt_control_reg, 1, 3); // Bit 3
  Adafruit_BusIO_RegisterBits als_ready_bit =
      Adafruit_BusIO_RegisterBits(&interrupt_control_reg, 1, 2); // Bit 2
  Adafruit_BusIO_RegisterBits thresh_bit =
      Adafruit_BusIO_RegisterBits(&interrupt_control_reg, 1, 1); // Bit 1
  Adafruit_BusIO_RegisterBits thresh_als_bit =
      Adafruit_BusIO_RegisterBits(&interrupt_control_reg, 1, 0); // Bit 0

  // Write the values to the register bits
  int_count_bits.write(intCount);
  prox_ready_bit.write(proxReady ? 1 : 0);
  als_ready_bit.write(alsReady ? 1 : 0);
  thresh_bit.write(thresh ? 1 : 0);
  thresh_als_bit.write(threshALS ? 1 : 0);
}

/*!
 * @brief  Gets the status of the interrupts from INTERRUPT STATUS REGISTER #14.
 * @return uint8_t containing the lower 4 bits of the INTERRUPT STATUS REGISTER.
 */
uint8_t Adafruit_VCNL4020::getInterruptStatus() {
  // Create an I2C Register object for Register #14 (INTERRUPT STATUS REGISTER)
  Adafruit_BusIO_Register int_status_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_INT_STATUS);

  // Read the value of the register
  uint8_t int_status = int_status_reg.read();

  // Mask the lower 4 bits to get the interrupt status
  return (int_status & 0x0F);
}

/*!
 * @brief  Clears the specified interrupt flags in INTERRUPT STATUS REGISTER
 * #14.
 * @param  proxready  True to clear the Proximity Ready interrupt flag, False to
 * leave it.
 * @param  alsready   True to clear the ALS Ready interrupt flag, False to leave
 * it.
 * @param  th_low     True to clear the Low Threshold interrupt flag, False to
 * leave it.
 * @param  th_high    True to clear the High Threshold interrupt flag, False to
 * leave it.
 */
void Adafruit_VCNL4020::clearInterrupts(bool proxready, bool alsready,
                                        bool th_low, bool th_high) {
  // Create an I2C Register object for Register #14 (INTERRUPT STATUS REGISTER)
  Adafruit_BusIO_Register int_status_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_INT_STATUS);

  // Read the current value of the register
  uint8_t int_status = int_status_reg.read();

  // Prepare the bits to be cleared
  uint8_t clear_bits = 0;
  if (proxready)
    clear_bits |= VCNL4020_INT_PROX_READY;
  if (alsready)
    clear_bits |= VCNL4020_INT_ALS_READY;
  if (th_low)
    clear_bits |= VCNL4020_INT_TH_LOW;
  if (th_high)
    clear_bits |= VCNL4020_INT_TH_HI;

  // Clear the specified bits by writing '1' to them
  int_status_reg.write(int_status | clear_bits);
}

/*!
 * @brief  Sets the Proximity Frequency in Register #15 Proximity Modulator
 * Timing Adjustment.
 * @param  freq  The proximity frequency setting, as defined in the
 * vcnl4020_proxfreq enum.
 */
void Adafruit_VCNL4020::setProxFrequency(vcnl4020_proxfreq freq) {
  // Create an I2C Register object for Register #15 (Proximity Modulator Timing
  // Adjustment)
  Adafruit_BusIO_Register prox_mod_timing_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_PROX_ADJUST);

  // Create bit accessors for Proximity Frequency (Bits 2 and 3)
  Adafruit_BusIO_RegisterBits prox_freq_bits = Adafruit_BusIO_RegisterBits(
      &prox_mod_timing_reg, 2, 3); // 2 bits starting at bit 3

  // Write the value to the register bits
  prox_freq_bits.write(freq);
}

/*!
 * @brief  Gets the proximity frequency setting.
 * @return  The current proximity frequency setting.
 */
vcnl4020_proxfreq Adafruit_VCNL4020::getProxFrequency() {
  Adafruit_BusIO_Register prox_timing_reg =
      Adafruit_BusIO_Register(_i2c, VCNL4020_REG_PROX_ADJUST);
  Adafruit_BusIO_RegisterBits proxfreq_bits = Adafruit_BusIO_RegisterBits(
      &prox_timing_reg, 2, 3); // 2 bits starting at bit 3
  return (vcnl4020_proxfreq)proxfreq_bits.read();
}
