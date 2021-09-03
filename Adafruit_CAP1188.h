/*!
 *  @file Adafruit_CAP1188.h
 *
 *  This is a library for the CAP1188 8-Channel Capacitive Sensor
 *
 *  Designed specifically to work with the CAP1188 I2C/SPI 8-chan Capacitive
 *  Sensor from Adafruit
 *
 *  Pick one up today in the adafruit shop!
 *  ------> https://www.adafruit.com/product/1602
 *
 *  These sensors use I2C/SPI to communicate, 2+ pins are required to interface
 *
 *  Adafruit invests time and resources providing this open source code,
 *  please support Adafruit andopen-source hardware by purchasing products
 *  from Adafruit!
 *
 *  Limor Fried/Ladyada (Adafruit Industries).
 *
 *  BSD license, all text above must be included in any redistribution
 */

#include "Arduino.h"
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SPIDevice.h>

#define CAP1188_I2CADDR 0x29 ///< The default I2C address

// Some registers we use
#define CAP1188_SENINPUTSTATUS                                                 \
  0x3 ///< The Sensor Input Status Register stores status bits that indicate a
      ///< touch has been detected. A value of ‘0’ in any bit indicates that no
      ///< touch has been detected. A value of ‘1’ in any bit indicates that a
      ///< touch has been detected.
#define CAP1188_MTBLK                                                          \
  0x2A ///< Multiple Touch Configuration register controls the settings for the
       ///< multiple touch detection circuitry. These settings determine the
       ///< number of simultaneous buttons that may be pressed before additional
       ///< buttons are blocked and the MULT status bit is set. [0/1]
#define CAP1188_LEDLINK                                                        \
  0x72 ///< Sensor Input LED Linking. Controls linking of sensor inputs to LED
       ///< channels
#define CAP1188_PRODID                                                         \
  0xFD ///< Product ID. Stores a fixed value that identifies each product.
#define CAP1188_MANUID                                                         \
  0xFE ///< Manufacturer ID. Stores a fixed value that identifies SMSC
#define CAP1188_STANDBYCFG                                                     \
  0x41 ///< Standby Configuration. Controls averaging and cycle time while in
       ///< standby.
#define CAP1188_REV                                                            \
  0xFF ///< Revision register. Stores an 8-bit value that represents the part
       ///< revision.
#define CAP1188_MAIN                                                           \
  0x00 ///< Main Control register. Controls the primary power state of the
       ///< device.
#define CAP1188_MAIN_INT                                                       \
  0x01 ///< Main Control Int register. Indicates that there is an interrupt.
#define CAP1188_LEDPOL                                                         \
  0x73 ///< LED Polarity. Controls the output polarity of LEDs.

/*!
 *    @brief  Class that stores state and functions for interacting with
 *            CAP1188 Sensor
 */
class Adafruit_CAP1188 {
public:
  // Software SPI
  Adafruit_CAP1188(uint8_t clkpin, uint8_t misopin, uint8_t mosipin,
                   uint8_t cspin, int8_t resetpin);
  // Hardware SPI
  Adafruit_CAP1188(uint8_t cspin, int8_t resetpin, SPIClass *theSPI = &SPI);
  // Hardware I2C
  Adafruit_CAP1188(int8_t resetpin = -1);

  boolean begin(uint8_t i2caddr = CAP1188_I2CADDR, TwoWire *theWire = &Wire);
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);
  uint8_t touched();
  void LEDpolarity(uint8_t x);

private:
  Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
  Adafruit_SPIDevice *spi_dev = NULL; ///< Pointer to SPI bus interface
  int8_t _resetpin;
};
