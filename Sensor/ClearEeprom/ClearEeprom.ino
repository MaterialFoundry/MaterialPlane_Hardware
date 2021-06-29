#include <EEPROM.h>

void setup() {
  Serial.begin(250000);
  EEPROM.begin(512);
  delay(100);
  for (int i=0; i<512; i++) EEPROM.write(i,255);
  EEPROM.commit(); 
  Serial.println("\nEEPROM Cleared");
}

void loop() {}
