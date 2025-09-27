/**
 * @file Adafruit_SGP41.cpp
 *
 * Arduino library for the Sensirion SGP41 multi-pixel gas sensor.
 *
 * Written by Limor 'ladyada' Fried with assistance from Claude Code for
 * Adafruit Industries. MIT license, all text above must be included in any
 * redistribution.
 */

#include "Adafruit_SGP41.h"

/**
 * @brief Construct a new Adafruit_SGP41 instance.
 */
Adafruit_SGP41::Adafruit_SGP41(void) : _i2c_dev(nullptr) {}

/**
 * @brief Destroy the Adafruit_SGP41 object and release resources.
 */
Adafruit_SGP41::~Adafruit_SGP41(void) {
  if (_i2c_dev) {
    delete _i2c_dev;
    _i2c_dev = nullptr;
  }
}

/**
 * @brief Initialize I2C communication with the SGP41 sensor.
 * @param addr The I2C address to use.
 * @param wire Pointer to the TwoWire instance for I2C communication.
 * @return true if the device is detected and ready.
 * @return false if communication could not be established.
 */
bool Adafruit_SGP41::begin(uint8_t addr, TwoWire* wire) {
  if (_i2c_dev) {
    delete _i2c_dev;
    _i2c_dev = nullptr;
  }

  _i2c_dev = new Adafruit_I2CDevice(addr, wire);
  if (!_i2c_dev) {
    return false;
  }

  if (!_i2c_dev->begin()) {
    delete _i2c_dev;
    _i2c_dev = nullptr;
    return false;
  }

  return true;
}
