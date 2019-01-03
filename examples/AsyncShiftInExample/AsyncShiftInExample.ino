#include <AsyncShiftIn.h>

#define SHIFT_REGISTER_LENGTH 16

boolean initialized;

AsyncShiftIn shiftRegister0;

void setup() {
  // Setup serial for output
  Serial.begin(115200);
  Serial.println(F("AsyncShiftInExample Starting..."));

  initialized = false;

  // Initializing the shift register
  shiftRegister0.init(2, 3, 6, 4, SHIFT_REGISTER_LENGTH, 90, 50);

  Serial.println(F("Initialized Shift Register ingerface."));

  // Optional: Perform a full sweep of the shift register
  while (!initialized) {
    shiftRegister0.loop();
  }

  Serial.println(F("Initial full sweep done."));
}

void loop() {
  // put your main code here, to run repeatedly:
  shiftRegister0.loop();
}

void AsyncShiftIn_reset(const AsyncShiftIn *asyncShiftIn) {
  // Any reset, especially the first one, will cause initialized to be set to
  // TRUE
  initialized = true;

  Serial.println(F("RESET!"));
}

void AsyncShiftIn_shift(const AsyncShiftIn *asyncShiftIn,
                        unsigned int bitNumber, uint8_t state) {
  Serial.print(bitNumber);
  Serial.print((state == HIGH ? ":HIGH " : ":LOW  "));
}
