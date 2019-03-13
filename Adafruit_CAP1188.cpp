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
#include <Wire.h>

// If the SPI library has transaction support, these functions
// establish settings and protect from interference from other
// libraries.  Otherwise, they simply do nothing.
void Adafruit_CAP1188::spi_begin() {
  // max speed!
  _spi->beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
}
void Adafruit_CAP1188::spi_end() { _spi->endTransaction(); }

/*!
 *    @brief  Instantiates a new CAP1188 class using hardware I2C
 *    @param  resetpin
 *            number of pin where reset is connected
 *
 */
Adafruit_CAP1188::Adafruit_CAP1188(int8_t resetpin) {
  // I2C
  _resetpin = resetpin;
  _i2c = true;
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
Adafruit_CAP1188::Adafruit_CAP1188(int8_t cspin, int8_t resetpin,
                                   SPIClass *theSPI) {
  // Hardware SPI
  _cs = cspin;
  _resetpin = resetpin;
  _clk = -1;
  _i2c = false;
  _spi = theSPI;
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
Adafruit_CAP1188::Adafruit_CAP1188(int8_t clkpin, int8_t misopin,
                                   int8_t mosipin, int8_t cspin,
                                   int8_t resetpin) {
  // Software SPI
  _cs = cspin;
  _resetpin = resetpin;
  _clk = clkpin;
  _miso = misopin;
  _mosi = mosipin;
  _i2c = false;
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
  if (_i2c) {
    _wire = theWire;
    _i2caddr = i2caddr;
    _wire->begin();
  } else if (_clk == -1) {
    // Hardware SPI
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    _spi->begin();
  } else {
    // Sofware SPI
    pinMode(_clk, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    digitalWrite(_clk, HIGH);
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
    @brief  Abstract away platform differences in Arduino wire library
    @param  x
 */
void Adafruit_CAP1188::i2cwrite(uint8_t x) {
  _wire->write((uint8_t)x);
}

/*!
 *   @brief  Reads 8-bits from the specified register
 */
uint8_t Adafruit_CAP1188::spixfer(uint8_t data) {
  if (_clk == -1) {
    // Serial.println("Hardware SPI");
    return _spi->transfer(data);
  } else {
    // Serial.println("Software SPI");
    uint8_t reply = 0;
    for (int i = 7; i >= 0; i--) {
      reply <<= 1;
      digitalWrite(_clk, LOW);
      digitalWrite(_mosi, data & (1 << i));
      digitalWrite(_clk, HIGH);
      if (digitalRead(_miso))
        reply |= 1;
    }
    return reply;
  }
}

/*!
 *    @brief  Reads from selected register
 *    @param  reg
 *            register address
 *    @return
 */
uint8_t Adafruit_CAP1188::readRegister(uint8_t reg) {
  if (_i2c) {
    _wire->beginTransmission(_i2caddr);
    i2cwrite(reg);
    _wire->endTransmission();
    _wire->requestFrom(_i2caddr, 1);
    return (_wire->read());
  } else {
    spi_begin();
    digitalWrite(_cs, LOW);
    // set address
    spixfer(0x7D);
    spixfer(reg);
    digitalWrite(_cs, HIGH);
    digitalWrite(_cs, LOW);
    spixfer(0x7F);
    uint8_t reply = spixfer(0);
    digitalWrite(_cs, HIGH);
    spi_end();
    return reply;
  }
}

/*!
 *   @brief  Writes 8-bits to the specified destination register
 *   @param  reg
 *           register address
 *   @param  value
 *           value that will be written at selected register
 */
void Adafruit_CAP1188::writeRegister(uint8_t reg, uint8_t value) {
  if (_i2c) {
    _wire->beginTransmission(_i2caddr);
    i2cwrite((uint8_t)reg);
    i2cwrite((uint8_t)(value));
    _wire->endTransmission();
  } else {
    spi_begin();
    digitalWrite(_cs, LOW);
    // set address
    spixfer(0x7D);
    spixfer(reg);
    digitalWrite(_cs, HIGH);
    digitalWrite(_cs, LOW);
    spixfer(0x7E);
    spixfer(value);
    digitalWrite(_cs, HIGH);
    spi_end();
  }
}
