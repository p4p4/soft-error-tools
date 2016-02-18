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
/// @file Stopwatch.cpp
/// @brief Contains the definition of the class Stopwatch.
// -------------------------------------------------------------------------------------------

#include "Stopwatch.h"

// -------------------------------------------------------------------------------------------
PointInTime Stopwatch::start()
{
  time_t start_time = time(NULL);
  clock_t start_clock = clock();
  return std::make_pair(start_clock, start_time);
}

// -------------------------------------------------------------------------------------------
double Stopwatch::getCPUTimeSec(const PointInTime &start_time)
{
  clock_t now = clock();
  return static_cast<double>(now - start_time.first) / CLOCKS_PER_SEC;
}

// -------------------------------------------------------------------------------------------
double Stopwatch::getCPUTimeMilliSec(const PointInTime &start_time)
{
  clock_t now = clock();
  return static_cast<double>(now - start_time.first) / (CLOCKS_PER_SEC/1000);
}

// -------------------------------------------------------------------------------------------
size_t Stopwatch::getRealTimeSec(const PointInTime &start_time)
{
  time_t now = time(NULL);
  return (now - start_time.second);
}

// -------------------------------------------------------------------------------------------
string Stopwatch::getTimeAsString(const PointInTime &start_time)
{
  ostringstream res;
  res << getCPUTimeSec(start_time) << " s (" << getRealTimeSec(start_time) << " s)";
  return res.str();
}

// -------------------------------------------------------------------------------------------
Stopwatch::~Stopwatch ()
{
  // Nothing to be done. Constructor is private.
}

