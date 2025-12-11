// Basic test for the Adafruit SGP41 sensor

#include <Adafruit_SGP41.h>

Adafruit_SGP41 sgp41;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println(F("Adafruit SGP41 test"));

  if (!sgp41.begin()) {
    Serial.println(F("Could not find SGP41"));
    while (1) {
      delay(10);
    }
  }

  Serial.println(F("SGP41 found"));

  // Perform a soft reset
  Serial.println(F("Performing soft reset..."));
  if (!sgp41.softReset()) {
    Serial.println(F("Soft reset failed!"));
    while (1) {
      delay(10);
    }
  }
  Serial.println(F("Soft reset successful."));
  delay(10); // Give it a moment to reset

  // Re-initialize and read serial number
  if (!sgp41.begin()) {
    Serial.println(F("Could not find SGP41 after reset"));
    while (1) {
      delay(10);
    }
  }

  uint16_t serial_number[3];
  if (sgp41.getSerialNumber(serial_number)) {
    Serial.print(F("Serial Number: 0x"));
    for (int i = 0; i < 3; i++) {
      if (serial_number[i] < 0x1000) Serial.print(F("0"));
      if (serial_number[i] < 0x100) Serial.print(F("0"));
      if (serial_number[i] < 0x10) Serial.print(F("0"));
      Serial.print(serial_number[i], HEX);
    }
    Serial.println();
  } else {
    Serial.println(F("Failed to read serial number!"));
  }

  // Self Test
  Serial.println(F("Executing self test..."));
  uint16_t self_test_result = sgp41.executeSelfTest();
  if (self_test_result == SGP41_SELF_TEST_OK) {
    Serial.println(F("Self test passed!"));
  } else {
    Serial.print(F("Self test failed (0x"));
    Serial.print(self_test_result, HEX);
    Serial.print(F("): "));
    if (self_test_result & SGP41_SELF_TEST_VOC_FAIL_MASK) {
      Serial.print(F("VOC pixel failed! "));
    }
    if (self_test_result & SGP41_SELF_TEST_NOX_FAIL_MASK) {
      Serial.print(F("NOx pixel failed! "));
    }
    if (!((self_test_result & SGP41_SELF_TEST_VOC_FAIL_MASK) || (self_test_result & SGP41_SELF_TEST_NOX_FAIL_MASK))) {
        Serial.print(F("Unknown failure. ")); // Should not happen if bits 0 or 1 indicate failure
    }
    Serial.println();
  }

  // Execute conditioning
  Serial.println(F("Executing conditioning..."));
  uint16_t sraw_voc;
  if (sgp41.executeConditioning(&sraw_voc)) {
    Serial.print(F("Conditioning successful, SRAW_VOC: 0x"));
    Serial.println(sraw_voc, HEX);
  } else {
    Serial.println(F("Conditioning failed!"));
  }
}

void loop(void) {
  uint16_t sraw_voc, sraw_nox;

  // Measure with default 50% RH and 25 degrees C compensation
  if (sgp41.measureRawSignals(&sraw_voc, &sraw_nox)) {
    Serial.print(F("SRAW_VOC: 0x"));
    Serial.print(sraw_voc, HEX);
    Serial.print(F(", SRAW_NOX: 0x"));
    Serial.println(sraw_nox, HEX);
  } else {
    Serial.println(F("Measurement failed!"));
  }
  delay(1000);
}
