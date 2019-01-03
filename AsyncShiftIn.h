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

using AsyncShiftInPinType = uint8_t;

/**
 * \brief Class that implements an asynchronous Shift Register input reader.
 *
 * The class is setup to support a Marklin S88 bus. For a "regular" shift
 * register, you can leave the "load" pin unused.
 */
class AsyncShiftIn
{
public:
  using PinIndexType = AsyncShiftInPinType;
  using ClockTimeType = unsigned int;
  using SleepTimeType = unsigned long;
  using LengthType = unsigned int;

  typedef struct
  {
    /// Input Pin for data from the shift register
    AsyncShiftInPinType data;
    /// Output Pin for generating the clock signal
    AsyncShiftInPinType clock;
    /// Output Pin for generating the load signal
    AsyncShiftInPinType load;
    /// Output Pin for generating the reset signal
    AsyncShiftInPinType reset;
  } PinConfiguration;

  typedef struct
  {
    /// The duration of the clock period in the read phase (in MICROseconds!)
    ClockTimeType clkPeriod;
    /// The duration of the clock period in the reset phase (in MICROseconds!)
    ClockTimeType resetPeriod;
  } TimeConfiguration;

protected:
  // Internal variables
  LengthType length;
  LengthType nextBit;
  enum State
  {
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

  const PinConfiguration *const pinConfiguration = nullptr;
  const TimeConfiguration *const timeConfiguration = nullptr;

  void setupOutput();
  void setupSleepDuration();
  void stateTransition();

public:
  /**
   * \brief Initializes this shift register.
   *
   * \param length The number of bits to shift in before resetting the shift
   */
  void init(const PinConfiguration *const pinConfiguration, const TimeConfiguration *const timeConfiguration, LengthType length);

  /**
   * \brief Update the length of the shift register.
   * 
   * The update is applied immediately and the object is set back to the state right after calling init.
   * The register will be re-read from the beginning.
   */
  void setLength(LengthType length);

  /**
   * \brief Main Loop to update pins and trigger callbacks.
   *
   * Call loop() in your main loop. When the time has come, this will update the
   * pins and call the appropriate callbacks.
   */
  void loop();
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