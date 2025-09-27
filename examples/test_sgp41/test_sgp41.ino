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
}

void loop(void) {}
