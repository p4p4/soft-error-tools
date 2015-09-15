// ----------------------------------------------------------------------------
// Copyright (c) 2013-2015 by Graz University of Technology and
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
/// @file Logger.cpp
/// @brief Contains the definition of the class Logger.
// -------------------------------------------------------------------------------------------

#include "Logger.h"

// -------------------------------------------------------------------------------------------
Logger *Logger::instance_ = NULL;

// -------------------------------------------------------------------------------------------
Logger &Logger::instance()
{
  if(instance_ == NULL)
    instance_ = new Logger;
  MASSERT(instance_ != NULL, "could not create Logger instance");
  return *instance_;
}

// -------------------------------------------------------------------------------------------
Logger::Logger ()
{
  streams_.push_back(&cerr); // ERR
  streams_.push_back(&cerr); // WRN
  streams_.push_back(&cout); // RES
  streams_.push_back(&cout); // INF
  streams_.push_back(&cout); // DBG
  streams_.push_back(&cout); // LOG
  enabled_.push_back(true);  // ERR
  enabled_.push_back(true);  // WRN
  enabled_.push_back(true);  // RES
  enabled_.push_back(true);  // INF
  enabled_.push_back(false); // DBG
  enabled_.push_back(true);  // LOG
}

// -------------------------------------------------------------------------------------------
Logger::~Logger ()
{
  streams_[0]->flush();
  streams_[1]->flush();
  streams_[2]->flush();
  streams_[3]->flush();
  streams_[4]->flush();
  streams_[5]->flush();
}

// -------------------------------------------------------------------------------------------
void Logger::disable(Logtype lt)
{
  enabled_[lt] = false;
}

// -------------------------------------------------------------------------------------------
void Logger::enable(Logtype lt)
{
  enabled_[lt] = true;
}

// -------------------------------------------------------------------------------------------
bool Logger::isEnabled(Logtype lt) const
{
  return enabled_[lt];
}

// -------------------------------------------------------------------------------------------
ostream &Logger::getOstream(Logtype lt)
{
  MASSERT(streams_[lt]->good(), "Cannot write to ostream");
  return *streams_[lt];
}

