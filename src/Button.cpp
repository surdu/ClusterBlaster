#include "Button.h";

Button::Button(uint8_t pin) {
  PIN = pin;

  pinMode(PIN, INPUT);
}

uint8_t Button::readBtnState() {
	uint8_t state = digitalRead(PIN);

	if (state != lastState) {
		btnTimer.start(BTN_DEBOUNCE_DELAY);
  }

  return state;
}

bool Button::pressed() {
	uint8_t state = readBtnState();

	if (btnTimer.isFinished()) {
		lastState = state;
		return state == LOW;
	}

	return false;
}
