#include "src/KWP.h";
#include "src/millisDelay.h";
#include "src/Button.h";
#include "src/MFALib/MFA.h";


#include "boot-image.h";

#define ADR_Engine 0x01
#define ADR_Gears  0x02
#define ADR_ABS_Brakes 0x03
#define ADR_Airbag 0x15
#define ADR_Dashboard 0x17
#define ADR_Immobilizer 0x25
#define ADR_Central_locking 0x35

#define REFRESH_RATE 1000

bool connected = false;
bool startup = true;

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


#define pinCLOCK 4
#define pinDATA 5
#define pinENABLE 6
MFA mfa(pinCLOCK, pinDATA, pinENABLE);

#define pinKLineRX 2
#define pinKLineTX 3
KWP kwp(pinKLineRX, pinKLineTX);

#define pinEnterBtn 7
#define pinDownBtn 8
#define pinUpBtn 9
Button enterBtn(pinEnterBtn);
Button downBtn(pinDownBtn);
Button upBtn(pinUpBtn);

millisDelay imageTimer;
millisDelay refreshTimer;

void setup() {
  pinMode(pinKLineTX, OUTPUT);
  digitalWrite(pinKLineTX, HIGH);

  Serial.begin(19200);
  Serial.println("SETUP: DONE");
  mfa.init();
}

void loop() {
	if (startup) {
		mfa.init_graphic();
		drawBootImage(mfa);
		imageTimer.start(3000);
		startup = false;
	}

	if (imageTimer.isFinished()) {
		mfa.remove_graphic();
	}

	if (!connected) {
		connected = kwp.connect(ADR_Dashboard, 10400);
		refreshTimer.start(REFRESH_RATE);
	}

	if (refreshTimer.isFinished()) {
		updateScreen();
		refreshTimer.repeat();
	}


	if (upBtn.pressed()) {
		Serial.println("UP PRESS");
	}

	if (downBtn.pressed()) {
		Serial.println("DOWN PRESS");
	}

	if (enterBtn.pressed()) {
		Serial.println("ENTER PRESS");
	}
}

void updateScreen() {
	if (!connected) {
		return;
	}

	Block result[4];
	kwp.readGroup(1, result);
	mfa.setRadioText("BOOST", String(random(1, 100))); //result[3].value
	Serial.println("REFRESH");
}
