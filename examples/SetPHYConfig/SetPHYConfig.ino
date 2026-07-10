/*
  Link Status

  This sketch prints the Ethernet link status. When the
  Ethernet cable is connected the link status should go to "ON".
  NOTE: Only WIZnet W5200 and W5500 are capable of reporting
  the link status. W5100 will report "Unknown".
  Hardware:
   - Ethernet shield or equivalent board/shield with WIZnet W5200/W5500
  Written by Cristian Maglie
  This example is public domain.
*/

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network.
// gateway and subnet are optional:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

void setup() {
  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH Shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit FeatherWing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit FeatherWing Ethernet

  Serial.begin(115200);

  Ethernet.begin(mac, ip, myDns, gateway, subnet);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1);  // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  /*
  * The available options are:
  * - H10: 10baseT/Half autoneg off
	* - F10: 10baseT/Full autoneg off
	* - H100: 100baseT/Half autoneg off
	* - F100: 100baseT/Full autoneg off
	* - H100Auto: 100baseT/Half autoneg on
	* - Off: Power Down mode
	* - All: All capable
	* - PMode: Use PMODE hardware bits instead of OPMD
  */
  Serial.println("Setting PHY configuration...");
  // Set Ethernet to 100 Mbps / Half duplex without auto-negotiation
  if (Ethernet.setPHYConfig(EthernetPHYConfig::H100) == 0) {
    Serial.println("PHY configuration set!");
  } else {
    Serial.println("Failed to set PHY configuration");
    while (true)
      ;
  }
}

void loop() {
}
