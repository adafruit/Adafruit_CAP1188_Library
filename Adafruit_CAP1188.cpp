/*!
 *  @file Adafruit_CAP1188.cpp
 *
 *  @mainpage Adafruit CAP1188 I2C/SPI 8-chan Capacitive Sensor
 *
 *  @section intro_sec Introduction
 *
 * 	This is a library for the Adafruit CAP1188 I2C/SPI 8-chan Capacitive
 * Sensor http://www.adafruit.com/products/1602
 *
 *  These sensors use I2C/SPI to communicate, 2+ pins are required to
 *  interface
 *
 * 	Adafruit invests time and resources providing this open source code,
 *  please support Adafruit and open-source hardware by purchasing products from
 * 	Adafruit!
 *
 *  @section author Author
 *
 *  Limor Fried/Ladyada (Adafruit Industries).
 *
 * 	@section license License
 *
 * 	BSD (see license.txt)
 *
 * 	@section  HISTORY
 *
 *     v1.0 - First release
 */

#include "Adafruit_CAP1188.h"

/*!
 *    @brief  Instantiates a new CAP1188 class using hardware I2C
 *    @param  resetpin
 *            number of pin where reset is connected
 *
 */
Adafruit_CAP1188::Adafruit_CAP1188(int8_t resetpin) {
  // I2C
  _resetpin = resetpin;
}

/*!
 *    @brief  Instantiates a new CAP1188 class using hardware SPI
 *    @param  cspin
 *            number of CSPIN (Chip Select)
 *    @param  *theSPI
 *            optional parameter contains spi object
 *    @param  resetpin
 *            number of pin where reset is connected
 *
 */
Adafruit_CAP1188::Adafruit_CAP1188(uint8_t cspin, int8_t resetpin,
                                   SPIClass *theSPI) {
  // Hardware SPI
  spi_dev = new Adafruit_SPIDevice(cspin, 2000000, SPI_BITORDER_MSBFIRST,
                                   SPI_MODE0, theSPI);
  _resetpin = resetpin;
}

/*!
 *    @brief  Instantiates a new CAP1188 class using software SPI
 *    @param  clkpin
 *            number of pin used for CLK (clock pin)
 *    @param  misopin
 *            number of pin used for MISO (Master In Slave Out)
 *    @param  mosipin
 *            number of pin used for MOSI (Master Out Slave In))
 *    @param  cspin
 *            number of CSPIN (Chip Select)
 *    @param  resetpin
 *            number of pin where reset is connected
 *
 */
Adafruit_CAP1188::Adafruit_CAP1188(uint8_t clkpin, uint8_t misopin,
                                   uint8_t mosipin, uint8_t cspin,
                                   int8_t resetpin) {
  // Software SPI
  spi_dev = new Adafruit_SPIDevice(cspin, clkpin, misopin, mosipin, 2000000);
  _resetpin = resetpin;
}

/*!
 *    @brief  Setups the i2c depending on selected mode (I2C / SPI, Software /
 * Hardware). Displays useful debug info, as well as allow multiple touches
 * (CAP1188_MTBLK), links leds to touches (CAP1188_LEDLINK), and increase the
 * cycle time value (CAP1188_STANDBYCFG)
 *    @param  i2caddr
 *            optional i2caddres (default to 0x29)
 *    @param  theWire
 *            optional wire object
 *    @return True if initialization was successful, otherwise false.
 */
boolean Adafruit_CAP1188::begin(uint8_t i2caddr, TwoWire *theWire) {
  if (spi_dev) {
    // Hardware or Software SPI
    if (!spi_dev->begin())
      return false;
  } else {
    // I2C
    i2c_dev = new Adafruit_I2CDevice(i2caddr, theWire);
    if (!i2c_dev->begin())
      return false;
  }

  if (_resetpin != -1) {
    pinMode(_resetpin, OUTPUT);
    digitalWrite(_resetpin, LOW);
    delay(100);
    digitalWrite(_resetpin, HIGH);
    delay(100);
    digitalWrite(_resetpin, LOW);
    delay(100);
  }

  readRegister(CAP1188_PRODID);

  // Useful debugging info

  Serial.print("Product ID: 0x");
  Serial.println(readRegister(CAP1188_PRODID), HEX);
  Serial.print("Manuf. ID: 0x");
  Serial.println(readRegister(CAP1188_MANUID), HEX);
  Serial.print("Revision: 0x");
  Serial.println(readRegister(CAP1188_REV), HEX);

  if ((readRegister(CAP1188_PRODID) != 0x50) ||
      (readRegister(CAP1188_MANUID) != 0x5D) ||
      (readRegister(CAP1188_REV) != 0x83)) {
    return false;
  }
  // allow multiple touches
  writeRegister(CAP1188_MTBLK, 0);
  // Have LEDs follow touches
  writeRegister(CAP1188_LEDLINK, 0xFF);
  // speed up a bit
  writeRegister(CAP1188_STANDBYCFG, 0x30);
  return true;
}

/*!
 *   @brief  Reads the touched status (CAP1188_SENINPUTSTATUS)
 *   @return Returns read from CAP1188_SENINPUTSTATUS where 1 is touched, 0 not
 * touched.
 */
uint8_t Adafruit_CAP1188::touched() {
  uint8_t t = readRegister(CAP1188_SENINPUTSTATUS);
  if (t) {
    writeRegister(CAP1188_MAIN, readRegister(CAP1188_MAIN) & ~CAP1188_MAIN_INT);
  }
  return t;
}

/*!
 *   @brief  Controls the output polarity of LEDs.
 *   @param  inverted
 *           0 (default) - The LED8 output is inverted.
 *           1 - The LED8 output is non-inverted.
 */
void Adafruit_CAP1188::LEDpolarity(uint8_t inverted) {
  writeRegister(CAP1188_LEDPOL, inverted);
}

/*!
 *    @brief  Reads from selected register
 *    @param  reg
 *            register address
 *    @return
 */
uint8_t Adafruit_CAP1188::readRegister(uint8_t reg) {
  uint8_t buffer[3] = {reg, 0, 0};
  if (i2c_dev) {
    i2c_dev->write_then_read(buffer, 1, buffer, 1);
  } else {
    buffer[0] = 0x7D;
    buffer[1] = reg;
    buffer[2] = 0x7F;
    spi_dev->write_then_read(buffer, 3, buffer, 1);
  }
  return buffer[0];
}

/*!
 *   @brief  Writes 8-bits to the specified destination register
 *   @param  reg
 *           register address
 *   @param  value
 *           value that will be written at selected register
 */
void Adafruit_CAP1188::writeRegister(uint8_t reg, uint8_t value) {
  uint8_t buffer[4] = {reg, value, 0, 0};
  if (i2c_dev) {
    i2c_dev->write(buffer, 2);
  } else {
    buffer[0] = 0x7D;
    buffer[1] = reg;
    buffer[2] = 0x7E;
    buffer[3] = value;
    spi_dev->write(buffer, 4);
  }
}
