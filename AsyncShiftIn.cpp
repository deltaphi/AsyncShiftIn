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

#include <Arduino.h>
#include <AsyncShiftIn.h>

void AsyncShiftIn::init(const PinConfiguration* const pinConfiguration, const TimeConfiguration* const timeConfiguration, unsigned int length) {
  this->pinConfiguration = pinConfiguration;
  this->timeConfiguration = timeConfiguration;
  
  // Init the hardware
  pinMode(this->pinConfiguration->clock, OUTPUT);
  pinMode(this->pinConfiguration->load, OUTPUT);
  pinMode(this->pinConfiguration->reset, OUTPUT);
  pinMode(this->pinConfiguration->data, INPUT);
  digitalWrite(this->pinConfiguration->clock, LOW);
  digitalWrite(this->pinConfiguration->load, LOW);
  digitalWrite(this->pinConfiguration->reset, LOW);

  setLength(length);
}

void AsyncShiftIn::setLength(LengthType length) {
  this->length = length;
  this->nextBit = length;  // act as if we were at the end
  this->state = READ_SAMPLE;
  this->currentSleepDuration = 0;
  this->lastActiveTime = 0;
}

void AsyncShiftIn::loop() {
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

void AsyncShiftIn::stateTransition() {
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

void AsyncShiftIn::setupSleepDuration() {
  switch (state) {
    case READ_HIGH:
      currentSleepDuration = this->timeConfiguration->clkPeriod;
      break;
    case READ_LOW:
      currentSleepDuration = this->timeConfiguration->clkPeriod / 2;
      break;
    case READ_SAMPLE:
      currentSleepDuration = this->timeConfiguration->clkPeriod / 2;
      break;
    case RES1:
      currentSleepDuration = this->timeConfiguration->resetPeriod;
      break;
    case RES2:
      currentSleepDuration = this->timeConfiguration->resetPeriod;
      break;
    case RES3:
      currentSleepDuration = this->timeConfiguration->resetPeriod;
      break;
    case RES4:
      currentSleepDuration = this->timeConfiguration->resetPeriod;
      break;
    case RES5:
      currentSleepDuration = this->timeConfiguration->resetPeriod;
      break;
    case RES6:
      currentSleepDuration = this->timeConfiguration->resetPeriod / 2;
      break;
    case RES6_SAMPLE:
      currentSleepDuration = this->timeConfiguration->resetPeriod / 2;
      break;
  }
}

void AsyncShiftIn::setupOutput() {
  switch (state) {
    case READ_HIGH:
      digitalWrite(this->pinConfiguration->clock, HIGH);
      break;
    case READ_LOW:
      digitalWrite(this->pinConfiguration->clock, LOW);
      break;
    case READ_SAMPLE:
      break;
    case RES1:
      digitalWrite(this->pinConfiguration->clock, LOW);
      digitalWrite(this->pinConfiguration->load, HIGH);
      break;
    case RES2:
      digitalWrite(this->pinConfiguration->clock, HIGH);
      break;
    case RES3:
      digitalWrite(this->pinConfiguration->clock, LOW);
      break;
    case RES4:
      digitalWrite(this->pinConfiguration->reset, HIGH);
      break;
    case RES5:
      digitalWrite(this->pinConfiguration->reset, LOW);
      break;
    case RES6:
      digitalWrite(this->pinConfiguration->load, LOW);
      break;
    case RES6_SAMPLE:
      break;
  }
}