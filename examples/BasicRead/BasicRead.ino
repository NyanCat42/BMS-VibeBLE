#include <BMSVibeBLE.h>

BMSVibeBLE bms("a5:c2:37:3b:1f:b3");

void setup() {
  Serial.begin(115200);
  bms.begin();
}

void loop() {
  bms.update();

  if (bms.isConnected()) {
    BMSData d = bms.getData();
    if (d.valid) {
      Serial.printf("Voltage: %.2f V | Current: %.2f A | SoC: %d%%\n", d.voltage, d.current, d.stateOfCharge);
      for (size_t i = 0; i < d.temperatures.size(); ++i) {
        Serial.printf("Temp %d: %.1f °C\n", (int)i + 1, d.temperatures[i]);
      }
    }
  } else {
    Serial.println("Waiting for connection...");
  }

  delay(5000);
}
