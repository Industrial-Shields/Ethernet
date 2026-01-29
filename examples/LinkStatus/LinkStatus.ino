/*
  Link Status

  This sketch prints the Ethernet link status. When the
  Ethernet cable is connected the link status should go to "ON".
  Written by Cristian Maglie
  This example is public domain.
*/

#include <SPI.h>
#include <Ethernet.h>

void setup() {
  Serial.begin(115200);
}

void loop() {
  auto link = Ethernet.linkStatus();
  Serial.print("Link status: ");
  switch (link) {
    case Unknown:
      Serial.println("Unknown");
      break;
    case LinkON:
      Serial.println("ON");
      break;
    case LinkOFF:
      Serial.println("OFF");
      break;
  }
  delay(1000);
}
