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
 * "AsyncShiftIn" ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Option) jeder späteren
 * veröffentlichten Version, weiterverbreiten und/oder modifizieren.
 *
 * "AsyncShiftIn" wird in der Hoffnung, dass es nützlich sein wird, aber
 * OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
 * Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Details.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include <AsyncShiftIn.h>

void AsyncShiftIn::init(uint8_t data, uint8_t clock, uint8_t load, uint8_t reset, unsigned int length, unsigned int clkPeriod, unsigned int resetPeriod) {
  // Init the hardware
  this->data = data; 
  this->clock = clock; 
  this->load = load;
  this->reset = reset;
  pinMode(this->clock, OUTPUT);
  pinMode(this->load, OUTPUT); 
  pinMode(this->reset, OUTPUT);
  pinMode(this->data, INPUT);
  digitalWrite(this->clock, LOW);
  digitalWrite(this->load, LOW);
  digitalWrite(this->reset, LOW); 
  
  this->length = length; 
  this->nextBit = length; // act as if we were at the end
  this->state = READ_SAMPLE;
  this->clkPeriod = clkPeriod;
  this->resetPeriod = resetPeriod;
  this->currentSleepDuration = 0;
  this->lastActiveTime = 0;
}

void AsyncShiftIn::loop() {
  // check for activity
  unsigned long sleepDuration(micros() - lastActiveTime); // this avoids the overflow problem
  if (sleepDuration <= currentSleepDuration) {
    return;
  }
  
  // enter new state
  stateTransition();
  
  // Sample
  int readValue = 0;
  if (state == READ_SAMPLE) {
    readValue = digitalRead(data);
  }
  
  // configure output
  setupOutput();
  
  // setup sleep duration
  setupSleepDuration();
  
  // setup lastActiveTime
  lastActiveTime = micros();
  
  // run callbacks
  if (AsyncShiftIn_shift && state == READ_SAMPLE) {
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

void AsyncShiftIn::setupOutput() {
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
      digitalWrite(reset, HIGH);
    break;
    case RES5:
      digitalWrite(reset, LOW);
    break;
    case RES6:
      digitalWrite(load, LOW);
    break;
    case RES6_SAMPLE:
    break;
  }
}