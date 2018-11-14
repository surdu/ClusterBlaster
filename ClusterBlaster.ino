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
#define MAX_KWP_RETRIES 3

bool ignitionON = false;
bool startup = true;

#define pinCLOCK 4
#define pinDATA 5
#define pinENABLE 6
MFA mfa(pinCLOCK, pinDATA, pinENABLE);

#define pinKLineRX 2
#define pinKLineTX 3
KWP kwp(pinKLineRX, pinKLineTX);

#define pinDownBtn 7
#define pinEnterBtn 8
#define pinUpBtn 9
Button downBtn(pinDownBtn);
Button enterBtn(pinEnterBtn);
Button upBtn(pinUpBtn);

millisDelay imageTimer;
millisDelay refreshTimer;

#define RADIO_ENTRIES 3
RadioEntry radioEntries[RADIO_ENTRIES] = {
	{"OIL TEMP", ADR_Dashboard, 3, 2},
	{"FUEL LVL", ADR_Dashboard, 2, 1},
	{"SPEED", ADR_Dashboard, 1, 0}
};
int8_t radioEntryIndex = 0;

void setup() {
  pinMode(pinKLineTX, OUTPUT);
  digitalWrite(pinKLineTX, HIGH);

  Serial.begin(19200);
  Serial.println("SETUP: DONE");

  ignitionON = true;
}

void loop() {
	if (!ignitionON) {
		uint8_t state = digitalRead(pinENABLE);
		if (state > 0) {
			ignitionON = true;
		}
	}

	if (ignitionON && startup) {
		mfa.init();
		mfa.init_graphic();
		drawBootImage(mfa);
		imageTimer.start(3000);
		startup = false;

		kwp.connect(ADR_Dashboard, 10400);
		refreshTimer.start(REFRESH_RATE);
	}

	if (imageTimer.isFinished()) {
		mfa.remove_graphic();
	}

	if (refreshTimer.isFinished()) {
		updateScreen();
		refreshTimer.repeat();
	}


	if (upBtn.pressed()) {
		radioEntryIndex = (radioEntryIndex + 1) % RADIO_ENTRIES;
		updateScreen();
	}

	if (downBtn.pressed()) {
		radioEntryIndex = (radioEntryIndex - 1) % RADIO_ENTRIES;

		if (radioEntryIndex  < 0) {
			radioEntryIndex += RADIO_ENTRIES;
		}
		updateScreen();
	}

	if (enterBtn.pressed()) {
		Serial.println("ENTER PRESS");
	}
}

void updateScreen() {
	if (!kwp.isConnected()) {
		ignitionON = false;
		startup = true;
		return;
	}

	Block result[4];
	kwp.readGroup(radioEntries[radioEntryIndex].group, result);
	mfa.setRadioText(radioEntries[radioEntryIndex].name, result[radioEntries[radioEntryIndex].groupIndex].value + " " + result[radioEntries[radioEntryIndex].groupIndex].unit);
	Serial.println("REFRESH");
}
