#ifndef Button_h
#define Button_h

#include <Arduino.h>;
#include "millisDelay.h";

#define BTN_DEBOUNCE_DELAY 50

class Button {
	public:
		Button(uint8_t pin);
		bool pressed();
	private:
		uint8_t PIN;
		millisDelay btnTimer;
		uint8_t lastState = LOW;

		uint8_t Button::readBtnState();
};

#endif
