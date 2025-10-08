/* Copyright 2018 Paul Stoffregen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Arduino.h>
#include "Ethernet.h"
#include "utility/w5100.h"
#include "Dhcp.h"

#if ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
// Add the warning in this file to only emit one warning
#warning "This library is deprecated in ESP32. Use the Ethernet library (ETH.h)!"
#endif

IPAddress EthernetClass::_dnsServerAddress;
DhcpClass* EthernetClass::_dhcp = NULL;

int EthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout)
{
	static DhcpClass s_dhcp;
	_dhcp = &s_dhcp;

	// Initialise the basic info
	if (W5100.init() == 0) return 0;

#if defined(MDUINO_PLUS) || defined(ESP32PLC)
	// Ignore PMODE inputs
	int result = setPHYConfig(EthernetPHYConfig::All);
	if (result != 0) {
		return result;
	}
#endif // defined(MDUINO_PLUS) || defined(ESP32PLC)

	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setMACAddress(mac);
	W5100.setIPAddress(IPAddress(0,0,0,0).raw_address());
	SPI.endTransaction();

	// Now try to get our config info from a DHCP server
	int ret = _dhcp->beginWithDHCP(mac, timeout, responseTimeout);
	if (ret == 1) {
		// We've successfully found a DHCP server and got our configuration
		// info, so set things accordingly
		SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
		W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
		W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
		W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
		SPI.endTransaction();
		_dnsServerAddress = _dhcp->getDnsServerIp();
		socketPortRand(micros());
	}
	return ret;
}

void EthernetClass::begin(uint8_t *mac, IPAddress ip)
{
	// Assume the DNS server will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress dns = ip;
	dns[3] = 1;
	begin(mac, ip, dns);
}

void EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns)
{
	// Assume the gateway will be the machine on the same network as the local IP
	// but with last octet being '1'
	IPAddress gateway = ip;
	gateway[3] = 1;
	begin(mac, ip, dns, gateway);
}

void EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway)
{
	IPAddress subnet(255, 255, 255, 0);
	begin(mac, ip, dns, gateway, subnet);
}

void EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet)
{
	if (W5100.init() == 0) return;

#if defined(MDUINO_PLUS) || defined(ESP32PLC)
	// Ignore PMODE inputs
	setPHYConfig(EthernetPHYConfig::All);
#endif // defined(MDUINO_PLUS) || defined(ESP32PLC)

	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setMACAddress(mac);
	W5100.setIPAddress(ip.raw_address());
	W5100.setGatewayIp(gateway.raw_address());
	W5100.setSubnetMask(subnet.raw_address());
	SPI.endTransaction();
	_dnsServerAddress = dns;
}

void EthernetClass::init(uint8_t sspin)
{
	W5100.setSS(sspin);
}

EthernetLinkStatus EthernetClass::linkStatus()
{
	switch (W5100.getLinkStatus()) {
		case UNKNOWN:  return Unknown;
		case LINK_ON:  return LinkON;
		case LINK_OFF: return LinkOFF;
		default:       return Unknown;
	}
}

EthernetHardwareStatus EthernetClass::hardwareStatus()
{
	switch (W5100.getChip()) {
		case 51: return EthernetW5100;
		case 52: return EthernetW5200;
		case 55: return EthernetW5500;
		default: return EthernetNoHardware;
	}
}

int EthernetClass::maintain()
{
	int rc = DHCP_CHECK_NONE;
	if (_dhcp != NULL) {
		// we have a pointer to dhcp, use it
		rc = _dhcp->checkLease();
		switch (rc) {
		case DHCP_CHECK_NONE:
			//nothing done
			break;
		case DHCP_CHECK_RENEW_OK:
		case DHCP_CHECK_REBIND_OK:
			//we might have got a new IP.
			SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
			W5100.setIPAddress(_dhcp->getLocalIp().raw_address());
			W5100.setGatewayIp(_dhcp->getGatewayIp().raw_address());
			W5100.setSubnetMask(_dhcp->getSubnetMask().raw_address());
			SPI.endTransaction();
			_dnsServerAddress = _dhcp->getDnsServerIp();
			break;
		default:
			//this is actually an error, it will retry though
			break;
		}
	}
	return rc;
}


void EthernetClass::MACAddress(uint8_t *mac_address)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getMACAddress(mac_address);
	SPI.endTransaction();
}

IPAddress EthernetClass::localIP()
{
	IPAddress ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getIPAddress(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

IPAddress EthernetClass::subnetMask()
{
	IPAddress ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getSubnetMask(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

IPAddress EthernetClass::gatewayIP()
{
	IPAddress ret;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.getGatewayIp(ret.raw_address());
	SPI.endTransaction();
	return ret;
}

void EthernetClass::setMACAddress(const uint8_t *mac_address)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setMACAddress(mac_address);
	SPI.endTransaction();
}

void EthernetClass::setLocalIP(const IPAddress local_ip)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = local_ip;
	W5100.setIPAddress(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClass::setSubnetMask(const IPAddress subnet)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = subnet;
	W5100.setSubnetMask(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClass::setGatewayIP(const IPAddress gateway)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	IPAddress ip = gateway;
	W5100.setGatewayIp(ip.raw_address());
	SPI.endTransaction();
}

void EthernetClass::setRetransmissionTimeout(uint16_t milliseconds)
{
	if (milliseconds > 6553) milliseconds = 6553;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setRetransmissionTime(milliseconds * 10);
	SPI.endTransaction();
}

void EthernetClass::setRetransmissionCount(uint8_t num)
{
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setRetransmissionCount(num);
	SPI.endTransaction();
}


int EthernetClass::setPHYConfig(EthernetPHYConfig cfg) {
	if (W5100.getChip() != 55) {
		// Register PHYCFGR it's only for W5500
		return -1;
	}

	bool result;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setW5500PHYConfig(static_cast<uint8_t>(cfg));
	SPI.endTransaction();
	delay(100); // Add some delay to let other PHYs detect that the link is down
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	W5100.setW5500PHYConfig(static_cast<uint8_t>(cfg) | (1 << 7));
	result = W5100.getW5500PHYConfig() == static_cast<uint8_t>(cfg);
	SPI.endTransaction();
	return result ? 0 : 1;
}

EthernetPHYConfig EthernetClass::getPHYConfig(void) {
	if (W5100.getChip() != 55) {
		// Register PHYCFGR it's only for W5500
		return EthernetPHYConfig::PMode;
	}

	EthernetPHYConfig phyRead;
	SPI.beginTransaction(SPI_ETHERNET_SETTINGS);
	phyRead = static_cast<EthernetPHYConfig>(W5100.getW5500PHYConfig());
	SPI.endTransaction();
	return phyRead;
}



EthernetClass Ethernet;
