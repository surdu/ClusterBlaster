#include "KWP.h"

KWP::KWP(uint8_t receivePin, uint8_t transmitPin){
  _OBD_RX_PIN = receivePin;
  _OBD_TX_PIN = transmitPin;

  pinMode(transmitPin, OUTPUT);
  digitalWrite(transmitPin, HIGH);

  obd = new NewSoftwareSerial(receivePin, transmitPin, false); // RX, TX, inverse logic

  Serial.println(F("KWP created"));
}

KWP::~KWP(){
  delete obd;
  obd = NULL;
}

bool KWP::connect(uint8_t addr, int baudrate) {
  Serial.print(F("------connect addr="));
  Serial.print(addr);
  Serial.print(F(" baud="));
  Serial.println(baudrate);
  blockCounter = 0;
  currAddr = 0;
  obd->begin(baudrate);
  // delay(3000);
  KWP5BaudInit(addr);

  char s[3];
  int size = 3;
  if (!KWPReceiveBlock(s, 3, size)) return false;
  if (    (((uint8_t)s[0]) != 0x55)
     ||   (((uint8_t)s[1]) != 0x01)
     ||   (((uint8_t)s[2]) != 0x8A)   ){
    Serial.println(F("ERROR: invalid magic"));
    disconnect();
    errorData++;
    return false;
  }
  currAddr = addr;
  connected = true;
  if (!readConnectBlocks()) return false;
  return true;
}

void KWP::disconnect() {
  connected = false;
}

void KWP::readGroup(int group, Block result[]) {
  Serial.print(F("------readMeasGroup "));
  Serial.println(group);

  char s[64];
  sprintf(s, "\x04%c\x29%c\x03", blockCounter, group);
  if (!KWPSendBlock(s, 5)) {
		return false;
	}

  int size = 0;
  KWPReceiveBlock(s, 64, size);
  if (s[2] != '\xe7') {
    Serial.println(F("ERROR: invalid answer"));
    disconnect();
    errorData++;
    return false;
  }

  int count = (size-4) / 3;
  Serial.print(F("count="));
  Serial.println(count);
  for (int idx=0; idx < count; idx++){
    byte k=s[3 + idx*3];
    byte a=s[3 + idx*3+1];
    byte b=s[3 + idx*3+2];
    String n;

    result[idx] = buildBlock(k, a, b);
  }
}

Block KWP::buildBlock(byte k, byte a, byte b) {
  Serial.print(F("type="));
  Serial.print(k);
  Serial.print(F("  a="));
  Serial.print(a);
  Serial.print(F("  b="));
  Serial.println(b);

	char buf[32];

  float value = 0;

  Block block;
  block.unit = "";

	switch (k) {
		case 1:  value=0.2*a*b;             block.unit=F("rpm"); break;
		case 2:  value=a*0.002*b;           block.unit=F("%%"); break;
		case 3:  value=0.002*a*b;           block.unit=F("Deg"); break;
		case 4:  value=abs(b-127)*0.01*a;   block.unit=F("ATDC"); break;
		case 5:  value=a*(b-100)*0.1;       block.unit=F("°C");break;
		case 6:  value=0.001*a*b;           block.unit=F("V");break;
		case 7:  value=0.01*a*b;            block.unit=F("km/h");break;
		case 8:  value=0.1*a*b;             block.unit=F(" ");break;
		case 9:  value=(b-127)*0.02*a;      block.unit=F("Deg");break;
		case 10: if (b == 0) block.value=F("COLD"); else block.value=F("WARM");break;
		case 11: value=0.0001*a*(b-128)+1;  block.unit = F(" ");break;
		case 12: value=0.001*a*b;           block.unit =F("Ohm");break;
		case 13: value=(b-127)*0.001*a;     block.unit =F("mm");break;
		case 14: value=0.005*a*b;           block.unit=F("bar");break;
		case 15: value=0.01*a*b;            block.unit=F("ms");break;
		case 18: value=0.04*a*b;            block.unit=F("mbar");break;
		case 19: value=a*b*0.01;            block.unit=F("l");break;
		case 20: value=a*(b-128)/128;       block.unit=F("%%");break;
		case 21: value=0.001*a*b;           block.unit=F("V");break;
		case 22: value=0.001*a*b;           block.unit=F("ms");break;
		case 23: value=b/256*a;             block.unit=F("%%");break;
		case 24: value=0.001*a*b;           block.unit=F("A");break;
		case 25: value=(b*1.421)+(a/182);   block.unit=F("g/s");break;
		case 26: value=float(b-a);          block.unit=F("C");break;
		case 27: value=abs(b-128)*0.01*a;   block.unit=F("°");break;
		case 28: value=float(b-a);          block.unit=F(" ");break;
		case 30: value=b/12*a;              block.unit=F("Deg k/w");break;
		case 31: value=b/2560*a;            block.unit=F("°C");break;
		case 33: value=100*b/a;             block.unit=F("%%");break;
		case 34: value=(b-128)*0.01*a;      block.unit=F("kW");break;
		case 35: value=0.01*a*b;            block.unit=F("l/h");break;
		case 36: value=((unsigned long)a)*2560+((unsigned long)b)*10;  block.unit=F("km");break;
		case 37: value=b; break; // oil pressure ?!
		// ADP: FIXME!
		/*case 37: switch(b){
					 case 0: sprintf(buf, F("ADP OK (%d,%d)"), a,b); t=String(buf); break;
					 case 1: sprintf(buf, F("ADP RUN (%d,%d)"), a,b); t=String(buf); break;
					 case 0x10: sprintf(buf, F("ADP ERR (%d,%d)"), a,b); t=String(buf); break;
					 default: sprintf(buf, F("ADP (%d,%d)"), a,b); t=String(buf); break;
				}*/
		case 38: value=(b-128)*0.001*a;        block.unit=F("Deg k/w"); break;
		case 39: value=b/256*a;                block.unit=F("mg/h"); break;
		case 40: value=b*0.1+(25.5*a)-400;     block.unit=F("A"); break;
		case 41: value=b+a*255;                block.unit=F("Ah"); break;
		case 42: value=b*0.1+(25.5*a)-400;     block.unit=F("Kw"); break;
		case 43: value=b*0.1+(25.5*a);         block.unit=F("V"); break;
		case 44: sprintf(buf, "%02d:%02d", a,b); block.value=String(buf); break;
		case 45: value=0.1*a*b/100;            block.unit=F(" "); break;
		case 46: value=(a*b-3200)*0.0027;      block.unit=F("Deg k/w"); break;
		case 47: value=(b-128)*a;              block.unit=F("ms"); break;
		case 48: value=b+a*255;                block.unit=F(" "); break;
		case 49: value=(b/4)*a*0.1;            block.unit=F("mg/h"); break;
		case 50: value=(b-128)/(0.01*a);       block.unit=F("mbar"); break;
		case 51: value=((b-128)/255)*a;        block.unit=F("mg/h"); break;
		case 52: value=b*0.02*a-a;             block.unit=F("Nm"); break;
		case 53: value=(b-128)*1.4222+0.006*a;  block.unit=F("g/s"); break;
		case 54: value=a*256+b;                block.unit=F("count"); break;
		case 55: value=a*b/200;                block.unit=F("s"); break;
		case 56: value=a*256+b;                block.unit=F("WSC"); break;
		case 57: value=a*256+b+65536;          block.unit=F("WSC"); break;
		case 59: value=(a*256+b)/32768;        block.unit=F("g/s"); break;
		case 60: value=(a*256+b)*0.01;         block.unit=F("sec"); break;
		case 62: value=0.256*a*b;              block.unit=F("S"); break;
		case 64: value=float(a+b);             block.unit=F("Ohm"); break;
		case 65: value=0.01*a*(b-127);         block.unit=F("mm"); break;
		case 66: value=(a*b)/511.12;          block.unit=F("V"); break;
		case 67: value=(640*a)+b*2.5;         block.unit=F("Deg"); break;
		case 68: value=(256*a+b)/7.365;       block.unit=F("deg/s");break;
		case 69: value=(256*a +b)*0.3254;     block.unit=F("Bar");break;
		case 70: value=(256*a +b)*0.192;      block.unit=F("m/s^2");break;
		default: sprintf(buf, "%2x, %2x      ", a, b); block.value=String(buf); break;
	}

	if (block.unit.length() != 0) {
	  dtostrf(value, 4, 2, buf);
	  block.value = String(buf);
	}

	return block;
}

bool KWP::isConnected() {
  return connected;
}

void KWP::obdWrite(uint8_t data) {
  obd->write(data);
}

uint8_t KWP::obdRead() {
  unsigned long timeout = millis() + 1000;
  while (!obd->available()){
    if (millis() >= timeout) {
      Serial.println(F("ERROR: obdRead timeout"));
      disconnect();
      errorTimeout++;
      return 0;
    }
  }
  uint8_t data = obd->read();
  return data;
}

bool KWP::KWP5BaudInit(uint8_t addr){
  Serial.println(F("---KWP 5 baud init"));

  #define bitcount 10
  byte bits[bitcount];
  byte even=1;
  byte bit;
  for (int i=0; i < bitcount; i++){
    bit=0;
    if (i == 0)  bit = 0;
      else if (i == 8) bit = even; // computes parity bit
      else if (i == 9) bit = 1;
      else {
        bit = (byte) ((addr & (1 << (i-1))) != 0);
        even = even ^ bit;
      }
    Serial.print(F("bit"));
    Serial.print(i);
    Serial.print(F("="));
    Serial.print(bit);
    if (i == 0) Serial.print(F(" startbit"));
      else if (i == 8) Serial.print(F(" parity"));
      else if (i == 9) Serial.print(F(" stopbit"));
    Serial.println();
    bits[i]=bit;
  }
  for (int i=0; i < bitcount+1; i++){
    if (i != 0){
      delay(200);
      if (i == bitcount) break;
    }
    if (bits[i] == 1){
      digitalWrite(_OBD_TX_PIN, HIGH);
    } else {
      digitalWrite(_OBD_TX_PIN, LOW);
    }
  }
  obd->flush();
  return true;
}

bool KWP::KWPSendBlock(char *s, int size) {
  Serial.print(F("---KWPSend sz="));
  Serial.print(size);
  Serial.print(F(" blockCounter="));
  Serial.println(blockCounter);
  Serial.print(F("OUT:"));
  for (int i=0; i < size; i++){
    uint8_t data = s[i];
    Serial.print(data, HEX);
    Serial.print(" ");
  }
  Serial.println();
  for (int i=0; i < size; i++){
    uint8_t data = s[i];
    obdWrite(data);
    if (i < size-1){
      uint8_t complement = obdRead();
      if (complement != (data ^ 0xFF)){
        Serial.println(F("ERROR: invalid complement"));
        disconnect();
        errorData++;
        return false;
      }
    }
  }
  blockCounter++;
  return true;
}

bool KWP::KWPReceiveBlock(char s[], int maxsize, int &size, bool init_delay) {
  bool ackeachbyte = false;
  uint8_t data = 0;
  int recvcount = 0;
  if (size == 0) ackeachbyte = true;
  Serial.print(F("---KWPReceive sz="));
  Serial.print(size);
  Serial.print(F(" blockCounter="));
  Serial.println(blockCounter);
  if (size > maxsize) {
    Serial.println("ERROR: invalid maxsize");
    return false;
  }
  unsigned long timeout = millis() + 2000;  // TODO: This allows connect to different Modules
  //unsigned long timeout = millis() + 1000;
  while ((recvcount == 0) || (recvcount != size)) {
    while (obd->available()){
      data = obdRead();
      s[recvcount] = data;
      recvcount++;
      if ((size == 0) && (recvcount == 1)) {
        size = data + 1;
        if (size > maxsize) {
          Serial.println("ERROR: invalid maxsize");
          return false;
        }
      }
      if ((ackeachbyte) && (recvcount == 2)) {
        if (data != blockCounter){
          Serial.println(F("ERROR: invalid blockCounter"));
          disconnect();
          errorData++;
          return false;
        }
      }
      if ( ((!ackeachbyte) && (recvcount == size)) ||  ((ackeachbyte) && (recvcount < size)) ){
        obdWrite(data ^ 0xFF);
      }
      timeout = millis() + 1000;
    }
    if (millis() >= timeout){
      Serial.println(F("ERROR: timeout"));
      disconnect();
      errorTimeout++;
      return false;
    }
  }
  Serial.print(F("IN: sz="));
  Serial.print(size);
  Serial.print(F(" data="));
  for (int i=0; i < size; i++){
    uint8_t data = s[i];
    Serial.print(data, HEX);
    Serial.print(F(" "));
  }
  Serial.println();
  blockCounter++;
  return true;
}

bool KWP::KWPSendAckBlock() {
  Serial.print(F("---KWPSendAckBlock blockCounter="));
  Serial.println(blockCounter);
  char buf[32];
  sprintf(buf, "\x03%c\x09\x03", blockCounter);
  return (KWPSendBlock(buf, 4));
}

bool KWP::readConnectBlocks() {
  Serial.println(F("------readconnectblocks"));
  String info;
  while (true){
    int size = 0;
    char s[64];
    if (!(KWPReceiveBlock(s, 64, size))) return false;
    if (size == 0) return false;
    if (s[2] == '\x09') break;
    if (s[2] != '\xF6') {
      Serial.println(F("ERROR: unexpected answer"));
      disconnect();
      errorData++;
      return false;
    }
    String text = String(s);
    info += text.substring(3, size-2);
    if (!KWPSendAckBlock()) return false;
  }
  Serial.print("label=");
  Serial.println(info);
  return true;
}
