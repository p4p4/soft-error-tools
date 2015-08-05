// ----------------------------------------------------------------------------
// Copyright (c) 2013-2014 by Graz University of Technology and
//                            Johannes Kepler University Linz
//
// This is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 3 of the License, or (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, see
// <http://www.gnu.org/licenses/>.
//
// For more information about this software see
//   <http://www.iaik.tugraz.at/content/research/design_verification/demiurge/>
// or email the authors directly.
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file Stopwatch.h
/// @brief Contains the declaration of the class Stopwatch.
// -------------------------------------------------------------------------------------------

#ifndef Stopwatch_H__
#define Stopwatch_H__

#include "defines.h"
#include <time.h>

// -------------------------------------------------------------------------------------------
///
/// @typedef pair<clock_t, time_t> PointInTime
/// @brief A type for a point in time.
typedef pair<clock_t, time_t> PointInTime;


// -------------------------------------------------------------------------------------------
///
/// @class Stopwatch
/// @brief Implements a stopwatch that can be used to measure execution time.
///
/// The stopwatch is able to find out both elapsed CPU time and elapsed realtime.
/// CPU time is meant to be used for precise measurements of short time periods. For longer
/// periods, it may overflow (typically after 36 minutes in a 32 bit machine; on a 64-bit
/// machine this usually does not happen). Use getRealTimeSec() to crosscheck.
///
/// Usage example:
/// @code
///   PointInTime start_time = Stopwatch::start();
///   doFirstPart();
///   double first_part_cpu = Stopwatch::getCPUTimeSec(start_time);
///   L_LOG("doFirstPart() took " << first_part_cpu << " seconds of CPU time.");
///   doSecondPart();
///   double all_real_time = Stopwatch::getRealTimeSec(start_time);
///   L_LOG("The whole computation took " << all_real_time << " seconds (real-time).");
/// @endcode
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.2.0
class Stopwatch
{

public:

// -------------------------------------------------------------------------------------------
///
/// @brief Starts the stopwatch.
///
/// @return Information about the moment in time where the stopwatch was started.
  static PointInTime start();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the elapsed CPU time in seconds.
///
/// CPU time is meant to be used for precise measurements of short time periods. For longer
/// periods, it may overflow (typically after 36 minutes in a 32 bit machine; on a 64-bit
/// machine this usually does not happen). Use getRealTimeSec() to crosscheck.
///
/// @param start_time The start time as produced by the start() method.
/// @return The CPU time elapsed since the corresponding call to start(), in seconds.
  static double getCPUTimeSec(const PointInTime &start_time);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the elapsed CPU time in milliseconds.
///
/// CPU time is meant to be used for precise measurements of short time periods. For longer
/// periods, it may overflow (typically after 36 minutes in a 32 bit machine; on a 64-bit
/// machine this usually does not happen). Use getRealTimeSec() to crosscheck.
///
/// @param start_time The start time as produced by the start() method.
/// @return The CPU time elapsed since the corresponding call to start(), in milliseconds.
  static double getCPUTimeMilliSec(const PointInTime &start_time);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the elapsed real-time in seconds.
///
/// This method returns the elapsed real-time in a rather coarse way. You can use it to check
/// whether the CPU-time counter had an overflow.
///
/// @param start_time The start time as produced by the start() method.
/// @return The real-time elapsed since the corresponding call to start(), in seconds.
  static size_t getRealTimeSec(const PointInTime &start_time);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns both the CPU time and the elapsed time as string.
///
/// This method returns the string 'XX s (YY s)', where XX is the elapsed CPU time and YY
/// is the elapsed real-time.
///
/// @param start_time The start time as produced by the start() method.
/// @return Both the CPU time and the elapsed time as string.
  static string getTimeAsString(const PointInTime &start_time);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~Stopwatch();

protected:

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// Constructor is private. Use the static methods instead.
  Stopwatch();

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  Stopwatch(const Stopwatch &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  Stopwatch& operator= (const Stopwatch &other);

};

#endif // Stopwatch_H__
