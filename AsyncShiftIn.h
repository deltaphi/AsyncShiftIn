/**
 * Copyright 2013 Damian Philipp
 *
 * This file is part of "AsyncShiftIn".
 *
 * "AsyncShiftIn" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "AsyncShiftIn" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Diese Datei ist Teil von "AsyncShiftIn".
 *
 * "AsyncShiftIn" ist Freie Software: Sie k�nnen es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Option) jeder sp�teren
 * ver�ffentlichten Version, weiterverbreiten und/oder modifizieren.
 *
 * "AsyncShiftIn" wird in der Hoffnung, dass es n�tzlich sein wird, aber
 * OHNE JEDE GEW�HRLEISTUNG, bereitgestellt; sogar ohne die implizite
 * Gew�hrleistung der MARKTF�HIGKEIT oder EIGNUNG F�R EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License f�r weitere Details.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
 */

#ifndef __ASYNC_SHIFT_IN__H__
#define __ASYNC_SHIFT_IN__H__

#include <Arduino.h>

using AsyncShiftInPinIndexType = uint8_t;

constexpr AsyncShiftInPinIndexType kPinUnused{255};

/**
 *
 * \tparam data Input Pin for data from the shift register
 * \tparam clock Output Pin for generating the clock signal
 * \tparam load Output Pin for generating the load signal
 * \tparam reset Output Pin for generating the reset signal
 */
template <AsyncShiftInPinIndexType data, AsyncShiftInPinIndexType clock,
          AsyncShiftInPinIndexType load, AsyncShiftInPinIndexType reset,
          typename TLengthType = uint8_t>
class AsyncShiftIn {
 public:
  using PinIndexType = AsyncShiftInPinIndexType;
  using ClockTimeType = unsigned int;
  using SleepTimeType = unsigned long;
  using LengthType = TLengthType;

 protected:
  // Internal variables
  LengthType length;
  LengthType nextBit;
  enum State {
    RES1,
    RES2,
    RES3,
    RES4,
    RES5,
    RES6,
    RES6_SAMPLE,
    READ_HIGH,
    READ_LOW,
    READ_SAMPLE
  };
  State state;

  ClockTimeType clkPeriod;
  ClockTimeType resetPeriod;
  SleepTimeType currentSleepDuration;
  SleepTimeType lastActiveTime;

  /**
   * \brief Set up the output pins according to the current state of the state
   * machine.
   */
  void setupOutput() {
    switch (state) {
      case READ_HIGH:
        digitalWrite(clock, HIGH);
        break;
      case READ_LOW:
        digitalWrite(clock, LOW);
        break;
      case READ_SAMPLE:
        break;
      case RES1:
        digitalWrite(clock, LOW);
        digitalWrite(load, HIGH);
        break;
      case RES2:
        digitalWrite(clock, HIGH);
        break;
      case RES3:
        digitalWrite(clock, LOW);
        break;
      case RES4:
        if (reset != kPinUnused) {
          digitalWrite(reset, HIGH);
        }
        break;
      case RES5:
        if (reset != kPinUnused) {
          digitalWrite(reset, LOW);
        }
        break;
      case RES6:
        digitalWrite(load, LOW);
        break;
      case RES6_SAMPLE:
        break;
    }
  }

  /**
   * \brief Setup the sleep duration to stay inactive until the next action.
   *
   * Note that the next action is always a state transition but not always a
   * change to the output pins.
   */
  void setupSleepDuration() {
    switch (state) {
      case READ_HIGH:
        currentSleepDuration = clkPeriod;
        break;
      case READ_LOW:
        currentSleepDuration = clkPeriod / 2;
        break;
      case READ_SAMPLE:
        currentSleepDuration = clkPeriod / 2;
        break;
      case RES1:
        currentSleepDuration = resetPeriod;
        break;
      case RES2:
        currentSleepDuration = resetPeriod;
        break;
      case RES3:
        currentSleepDuration = resetPeriod;
        break;
      case RES4:
        currentSleepDuration = resetPeriod;
        break;
      case RES5:
        currentSleepDuration = resetPeriod;
        break;
      case RES6:
        currentSleepDuration = resetPeriod / 2;
        break;
      case RES6_SAMPLE:
        currentSleepDuration = resetPeriod / 2;
        break;
    }
  }

  /**
   * \brief Advance to the next state.
   */
  void stateTransition() {
    switch (state) {
      case READ_HIGH:
        state = READ_LOW;
        break;
      case READ_LOW:
        state = READ_SAMPLE;
        break;
      case READ_SAMPLE:
        if (nextBit < length) {
          state = READ_HIGH;
        } else {
          nextBit = 0;
          state = RES1;
        }
        break;
      case RES1:
        state = RES2;
        break;
      case RES2:
        state = RES3;
        break;
      case RES3:
        state = RES4;
        break;
      case RES4:
        state = RES5;
        break;
      case RES5:
        state = RES6;
        break;
      case RES6:
        state = RES6_SAMPLE;
        break;
      case RES6_SAMPLE:
        state = READ_HIGH;
        break;
    }
  }

 public:
  /**
   * \brief Initialize the class with a given register length and timing.
   * 
   * \param length The number of bits to shift in before resetting the shift
   *               register
   * \param clkPeriod The duration of the clock period in the read phase (in
   *                  MICROseconds!)
   * \param resetPeriod The duration of the clock period in the reset phase (in
   *                    MICROseconds!)
   */
  AsyncShiftIn(LengthType length, ClockTimeType clkPeriod,
               ClockTimeType resetPeriod)
      : clkPeriod(clkPeriod), resetPeriod(resetPeriod) {
    setLength(length);
  }

  /**
   * \brief Initializes the hardware for this shift register.
   */
  void init() {
    // Init the hardware
    pinMode(clock, OUTPUT);
    pinMode(load, OUTPUT);
    if (reset != kPinUnused) {
      pinMode(reset, OUTPUT);
    }
    pinMode(data, INPUT);

    digitalWrite(clock, LOW);
    digitalWrite(load, LOW);
    if (reset != kPinUnused) {
      digitalWrite(reset, LOW);
    }

    // Reset to current length
    setLength(length);
  }

  void setLength(LengthType length) {
    this->length = length;
    this->nextBit = length;  // act as if we were at the end
    this->state = READ_SAMPLE;
    this->clkPeriod = clkPeriod;
    this->resetPeriod = resetPeriod;
    this->currentSleepDuration = 0;
    this->lastActiveTime = 0;
  }

  /**
   * \brief Main Loop to update pins and trigger callbacks.
   *
   * Call loop() in your main loop. When the time has come, this will update the
   * pins and call the appropriate callbacks.
   */
  void loop() {
    // check for activity
    SleepTimeType sleepDuration(
        micros() - lastActiveTime);  // this avoids the overflow problem
    if (sleepDuration <= currentSleepDuration) {
      return;
    }

    // enter new state
    stateTransition();

    // Sample
    uint8_t readValue = 0;
    if (state == READ_SAMPLE || state == RES6_SAMPLE) {
      readValue = digitalRead(data);
    }

    // configure output
    setupOutput();

    // setup sleep duration
    setupSleepDuration();

    // setup lastActiveTime
    lastActiveTime = micros();

    // run callbacks
    if (AsyncShiftIn_shift && (state == READ_SAMPLE || state == RES6_SAMPLE)) {
      AsyncShiftIn_shift(this, nextBit, readValue);
      ++nextBit;
    }

    if (AsyncShiftIn_reset && state == RES1) {
      AsyncShiftIn_reset(this);
    }
  }
};

/**
 * \brief This callback is called whenever a new bit has been shifted in.
 *
 * Please make sure to keep the processing in this method fast, so that the
 * timing won't suffer too much. If you have no processing to do for individual
 * bits (unlikely...), you can omit this callback.
 *
 * \param asyncShiftIn A pointer to the calling AsyncShiftIn object to
 * distinguish between multiple shift registers
 * \param bitNumer The index of the bit that was just shifted in (0-based)
 * \param state The state of the bit that was just shifted in (LOW/HIGH)
 */
extern void AsyncShiftIn_shift(const AsyncShiftIn *asyncShiftIn,
                               LengthType bitNumber, uint8_t state)
    __attribute__((weak));

/**
 * \brief This callback is called whenever the shift register has been reset.
 *
 * This callback indicates that a read cycle was completed. Please make sure to
 * keep the processing in this method fast, so that the timing won't suffer too
 * much.
 * If you have no processing to do per cycle, you can omit this callback.
 *
 * \param asyncShiftIn A pointer to the calling AsyncShiftIn object to
 * distinguish between multiple shift registers
 */
extern void AsyncShiftIn_reset(const AsyncShiftIn *asyncShiftIn)
    __attribute__((weak));

#endif