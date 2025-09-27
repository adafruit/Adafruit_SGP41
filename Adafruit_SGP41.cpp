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
Adafruit_SGP41::Adafruit_SGP41(void) : _i2c_dev(nullptr), _wire(nullptr) {}

/**
 * @brief Destroy the Adafruit_SGP41 object and release resources.
 */
Adafruit_SGP41::~Adafruit_SGP41(void) {
  if (_i2c_dev) {
    delete _i2c_dev;
    _i2c_dev = nullptr;
  }
  _wire = nullptr;
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
  _wire = wire;

  _i2c_dev = new Adafruit_I2CDevice(addr, wire);
  if (!_i2c_dev) {
    return false;
  }

  if (!_i2c_dev->begin()) {
    delete _i2c_dev;
    _i2c_dev = nullptr;
    _wire = nullptr;
    return false;
  }

  return true;
}

/**
 * @brief Execute the SGP41 conditioning command.
 * @param sraw_voc Pointer that will receive the VOC raw ticks.
 * @param default_rh Humidity compensation word to transmit.
 * @param default_t Temperature compensation word to transmit.
 * @return true on success, false otherwise.
 */
bool Adafruit_SGP41::executeConditioning(uint16_t* sraw_voc,
                                         uint16_t default_rh,
                                         uint16_t default_t) {
  if (!_i2c_dev || !sraw_voc) {
    return false;
  }

  uint16_t payload[2] = {default_rh, default_t};
  if (!_writeCommand(SGP41_CMD_EXECUTE_CONDITIONING, payload, 2)) {
    return false;
  }

  delay(SGP41_CONDITIONING_DELAY_MS);

  return _readWords(sraw_voc, 1);
}

/**
 * @brief Measure raw VOC and NOx signals from the sensor.
 * @param sraw_voc Pointer that will receive the VOC raw ticks.
 * @param sraw_nox Pointer that will receive the NOx raw ticks.
 * @param relative_humidity Humidity compensation word.
 * @param temperature Temperature compensation word.
 * @return true on success, false otherwise.
 */
bool Adafruit_SGP41::measureRawSignals(uint16_t* sraw_voc, uint16_t* sraw_nox,
                                       uint16_t relative_humidity,
                                       uint16_t temperature) {
  if (!_i2c_dev || !sraw_voc || !sraw_nox) {
    return false;
  }

  uint16_t payload[2] = {relative_humidity, temperature};
  if (!_writeCommand(SGP41_CMD_MEASURE_RAW_SIGNALS, payload, 2)) {
    return false;
  }

  delay(SGP41_MEASUREMENT_DELAY_MS);

  uint16_t results[2];
  if (!_readWords(results, 2)) {
    return false;
  }

  *sraw_voc = results[0];
  *sraw_nox = results[1];
  return true;
}

/**
 * @brief Run the built-in self-test sequence.
 * @param test_result Pointer that will receive the 16-bit test result code.
 * @return true on success, false otherwise.
 */
bool Adafruit_SGP41::executeSelfTest(uint16_t* test_result) {
  if (!_i2c_dev || !test_result) {
    return false;
  }

  if (!_writeCommand(SGP41_CMD_EXECUTE_SELF_TEST)) {
    return false;
  }

  delay(SGP41_SELF_TEST_DELAY_MS);

  return _readWords(test_result, 1);
}

/**
 * @brief Turn off the integrated heater and enter idle mode.
 * @return true on success, false otherwise.
 */
bool Adafruit_SGP41::turnHeaterOff(void) {
  if (!_i2c_dev) {
    return false;
  }

  if (!_writeCommand(SGP41_CMD_TURN_HEATER_OFF)) {
    return false;
  }

  delay(1);
  return true;
}

/**
 * @brief Retrieve the 48-bit serial number from the sensor.
 * @param serial_number Array of three 16-bit words to store the serial number.
 * @return true on success, false otherwise.
 */
bool Adafruit_SGP41::getSerialNumber(uint16_t serial_number[3]) {
  if (!_i2c_dev || !serial_number) {
    return false;
  }

  if (!_writeCommand(SGP41_CMD_GET_SERIAL_NUMBER)) {
    return false;
  }

  delay(1);

  return _readWords(serial_number, 3);
}

/**
 * @brief Issue an I2C general call soft reset.
 * @return true on success, false otherwise.
 */
bool Adafruit_SGP41::softReset(void) {
  if (!_wire) {
    return false;
  }

  Adafruit_I2CDevice general_call(SGP41_GENERAL_CALL_ADDR, _wire);
  if (!general_call.begin(false)) {
    return false;
  }

  uint8_t payload[2] = {0x00, 0x06};
  if (!general_call.write(payload, sizeof(payload))) {
    return false;
  }

  delay(1);
  return true;
}

/**************************************************************************/
/*!
    @brief Write a command and optional payload words to the sensor.
    @param command 16-bit command code.
    @param data_words Pointer to 16-bit payload words (can be nullptr).
    @param num_words Number of payload words.
    @returns True on success.
*/
/**************************************************************************/
bool Adafruit_SGP41::_writeCommand(uint16_t command, const uint16_t* data_words,
                                   uint8_t num_words) {
  if (!_i2c_dev) {
    return false;
  }

  uint8_t buffer[2 + 3 * 3];
  uint8_t index = 0;
  buffer[index++] = command >> 8;
  buffer[index++] = command & 0xFF;

  for (uint8_t i = 0; i < num_words; i++) {
    uint16_t word = data_words[i];
    buffer[index++] = word >> 8;
    buffer[index++] = word & 0xFF;
    buffer[index++] = _crc8(word);
  }

  return _i2c_dev->write(buffer, index);
}

/**************************************************************************/
/*!
    @brief Read 16-bit data words from the sensor, verifying CRC.
    @param data Pointer to store the received words.
    @param num_words Number of 16-bit words expected.
    @returns True on success.
*/
/**************************************************************************/
bool Adafruit_SGP41::_readWords(uint16_t* data, uint8_t num_words) {
  if (!_i2c_dev) {
    return false;
  }

  if (num_words == 0) {
    return true;
  }

  uint8_t buffer[3 * 3];
  uint8_t read_len = num_words * 3;

  if (!_i2c_dev->read(buffer, read_len)) {
    return false;
  }

  for (uint8_t i = 0, j = 0; i < num_words; i++, j += 3) {
    uint16_t word = ((uint16_t)buffer[j] << 8) | buffer[j + 1];
    uint8_t crc = buffer[j + 2];
    if (_crc8(word) != crc) {
      return false;
    }
    data[i] = word;
  }

  return true;
}

/**************************************************************************/
/*!
    @brief Compute the CRC-8 checksum used by the SGP41.
    @param word 16-bit data word to protect.
    @returns The CRC-8 checksum value.
*/
/**************************************************************************/
uint8_t Adafruit_SGP41::_crc8(uint16_t word) {
  uint8_t crc = 0xFF;
  uint8_t bytes[2] = {static_cast<uint8_t>(word >> 8),
                      static_cast<uint8_t>(word & 0xFF)};

  for (uint8_t i = 0; i < 2; i++) {
    crc ^= bytes[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {
        crc = (crc << 1) ^ 0x31;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}
