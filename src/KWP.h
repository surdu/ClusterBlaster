#ifndef KWP_h
#define KWP_h

#include <inttypes.h>
#include <Arduino.h>
#include "NewSoftwareSerial.h"

#define ADR_Engine 0x01 // Engine
#define ADR_Gears  0x02 // Auto Trans
#define ADR_ABS_Brakes 0x03
#define ADR_Airbag 0x15
#define ADR_Dashboard 0x17 // Instruments
#define ADR_Immobilizer 0x25
#define ADR_Central_locking 0x35
#define ADR_Navigation 0x37

struct RadioEntry {
	String name;
	uint8_t addr;
	uint8_t group;
	uint8_t groupIndex;
};

struct Block {
  String name;
  String value;
  String unit;
};

class KWP {
  public:
    KWP(uint8_t receivePin, uint8_t transmitPin);
    ~KWP();
    bool connect(uint8_t addr, int baudrate);
    void disconnect();
    void readGroup(int group, Block result[]);
    Block buildBlock(byte k, byte a, byte b);
    bool isConnected();
    uint8_t getCurrAddr();
  private:
    uint8_t _OBD_RX_PIN;
    uint8_t _OBD_TX_PIN;

    bool connected = false;
    uint8_t blockCounter = 0;
    uint8_t errorTimeout = 0;
    uint8_t errorData = 0;
    uint8_t currAddr = 0;

		int8_t coolantTemp = 0;
		int8_t oilTemp = 0;
		int8_t intakeAirTemp = 0;
		int8_t oilPressure = 0;
		float engineLoad = 0;
		int   engineSpeed = 0;
		float throttleValve = 0;
		float supplyVoltage = 0;
		uint8_t vehicleSpeed = 0;
		uint8_t fuelConsumption = 0;
		uint8_t fuelLevel = 0;
		unsigned long odometer = 0;


    NewSoftwareSerial *obd;

    void obdWrite(uint8_t data);
    uint8_t obdRead();
    bool KWP5BaudInit(uint8_t addr);
    bool KWPSendBlock(char *s, int size);
    bool KWPReceiveBlock(char s[], int maxsize, int &size, bool init_delay = false);
    bool KWPSendAckBlock();
    bool readConnectBlocks();

};

#endif
