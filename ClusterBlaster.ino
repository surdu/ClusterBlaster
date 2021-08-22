#include <StateMachine.h>;

#include "src/KWP.h";
#include "src/millisDelay.h";
#include "src/Button.h";
#include "src/MFALib/MFA.h";

// #include "boot-image.h";

#define REFRESH_RATE 500
#define MAX_KWP_RETRIES 3

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

#define ledPin 13

millisDelay imageTimer;
millisDelay refreshTimer;
millisDelay ledTimer;

Module Dash = {
	ADR_Dashboard,
	10400
};

Module ECU = {
	ADR_Engine,
	9600
};


#define RADIO_ENTRIES 5
RadioEntry radioEntries[RADIO_ENTRIES] = {
	{"VOLTAGE", ECU, 16, 3},
	{"BOOST", ECU, 11, 2},
	{"OIL TEMP", Dash, 3, 2},
	{"FUEL LVL", Dash, 2, 1},
	{"SPEED", Dash, 1, 0}
};
int8_t radioEntryIndex = 0;

StateMachine machine = StateMachine();

State* S1 = machine.addState(&S1idle);
State* S2 = machine.addState(&S2showLogo);
State* S3 = machine.addState(&S3showVoltage);
State* S4 = machine.addState(&S4showOilTemp);

void setup() {
  pinMode(pinKLineTX, OUTPUT);
  digitalWrite(pinKLineTX, HIGH);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  Serial.begin(9600);

	S1->addTransition(&transitionS1S2,S2);
	S2->addTransition(&transitionS2S3,S3);
	S3->addTransition(&transitionS3S4,S4);
  Serial.println("SETUP: DONE");

	mfa.init();
}

void loop() {
	machine.run();
  delay(REFRESH_RATE);
}

void debug(String a, ...) {
	va_list l_Arg;
	va_start(l_Arg, a);

	while(a != NULL) {
		Serial.print(a);
		Serial.print(" ");
		a = va_arg(l_Arg, String);
	}
	Serial.println();
}

void S1idle() {
	Serial.println("Idle");
}

bool transitionS1S2(){
	uint8_t state = digitalRead(pinKLineRX);
	debug("State:", state);
	return state > 0;
}

void S2showLogo() {
	if(machine.executeOnce){
		Serial.println("Logo");
		// mfa.init_graphic();
		// drawBootImage(mfa);
		imageTimer.start(3000);
	}
}

bool transitionS2S3() {
	bool isFinished = imageTimer.isFinished();
	Serial.print("isFinished:");
	Serial.println(isFinished);

	if (isFinished) {
		// mfa.remove_graphic();
	}
	return isFinished;
}

void S3showVoltage() {
	RadioEntry voltageEntry = {"VOLTAGE", ECU, 16, 3};
	displayRadioEntry(voltageEntry);
}

bool transitionS3S4() {
	RadioEntry rpmEntry = {"RPM", ECU, 8, 0};
	String rpm = readKWP(rpmEntry);
	Serial.print("RPM: ");
	Serial.println(rpm.toFloat());
	return rpm.toFloat() > 0 ;
}

void S4showOilTemp() {
	RadioEntry oilTempEntry = {"OIL TEMP", Dash, 3, 2};
	displayRadioEntry(oilTempEntry);
}

// ----------------------------------------------------------------

void connect(RadioEntry entry) {
	if (!kwp.isConnected() || kwp.getCurrAddr() != entry.module.addr) {
		Serial.print("Connecting to module: ");
		Serial.println(entry.module.addr);
		kwp.connect(entry.module.addr, entry.module.baud);
	}
}

String readKWP(RadioEntry entry) {
	connect(entry);
	Block result[4];
	kwp.readGroup(entry.group, result);
	return result[entry.groupIndex].value;
}

void displayRadioEntry(RadioEntry entry) {
	String value = readKWP(entry);
	Serial.print(entry.name);
	Serial.print(": ");
	Serial.println(value);
	mfa.setRadioText(entry.name, value);
}
