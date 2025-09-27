// Basic raw signal example for the Adafruit SGP41 sensor

#include <Adafruit_SGP41.h>
#include <Wire.h>

Adafruit_SGP41 sgp41;

static uint16_t conditioning_seconds = 10;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println(F("Adafruit SGP41 raw signal example"));

  Wire.begin();

  if (!sgp41.begin()) {
    Serial.println(F("Failed to find SGP41 sensor"));
    while (1) {
      delay(10);
    }
  }

  uint16_t serial_number[3];
  if (sgp41.getSerialNumber(serial_number)) {
    Serial.print(F("Serial number: 0x"));
    for (uint8_t i = 0; i < 3; i++) {
      if (serial_number[i] < 0x1000) {
        Serial.print(F("0"));
      }
      if (serial_number[i] < 0x0100) {
        Serial.print(F("0"));
      }
      if (serial_number[i] < 0x0010) {
        Serial.print(F("0"));
      }
      Serial.print(serial_number[i], HEX);
    }
    Serial.println();
  } else {
    Serial.println(F("Failed to read serial number"));
  }

  uint16_t self_test = sgp41.executeSelfTest();
  if (self_test == SGP41_SELF_TEST_OK) {
    Serial.println(F("Self-test passed"));
  } else {
    Serial.print(F("Self-test result: 0x"));
    Serial.println(self_test, HEX);
  }
}

void loop(void) {
  delay(1000);

  uint16_t sraw_voc = 0;
  uint16_t sraw_nox = 0;

  bool ok = false;
  if (conditioning_seconds > 0) {
    ok = sgp41.executeConditioning(&sraw_voc);
    conditioning_seconds--;
  } else {
    ok = sgp41.measureRawSignals(&sraw_voc, &sraw_nox);
  }

  if (!ok) {
    Serial.println(F("Measurement failed"));
    return;
  }

  Serial.print(F("SRAW_VOC: "));
  Serial.print(sraw_voc);
  Serial.print(F("\t"));
  Serial.print(F("SRAW_NOx: "));
  Serial.println(sraw_nox);
}
