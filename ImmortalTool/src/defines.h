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
/// @file defines.h
/// @brief Includes some often used header files and names from the std:: namespace.
// -------------------------------------------------------------------------------------------

#ifndef DEFINES_H__
#define DEFINES_H__

// This is no good programming practice, but it is convenient ...
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <limits>
#include <exception>

using namespace std;

typedef vector<vector<int> > TestCase;

#define AIG_FALSE			 0
#define AIG_TRUE 			 1
#define CNF_FALSE 		 1
#define CNF_TRUE			-1

// -------------------------------------------------------------------------------------------
///
/// @def MASSERT(condition, message)
/// @brief A macro for an assertion associated with an error message.
///
/// If the condition evaluates to false, the error message is printed and the program
/// terminates.
///
/// @param condition The condition of the assertion. If this condition evaluates to false,
///        then the error message is printed.
/// @param message The error message to be printed upon assertion violation.
#define MASSERT(condition, message)                                          \
{                                                                            \
  if(!(condition))                                                           \
  {                                                                          \
    std::cerr << __FILE__ << " (" << __LINE__ << ") : " << message << endl;  \
    abort();                                                                 \
  }                                                                          \
}

// -------------------------------------------------------------------------------------------
///
/// @def DASSERT(condition, message)
/// @brief A macro for an assertion that is only enabled in debug mode.
///
/// In debug mode, if the condition evaluates to false, the error message is printed and the
/// program terminates. If not in debug mode, the statement is ignored (the condition is not
/// even evaluated).
///
/// @param condition The condition of the assertion. If we are in debug mode, and if this
///        condition evaluates to false, then the error message is printed.
/// @param message The error message to be printed upon assertion violation.
#ifndef NDEBUG
#define DASSERT(condition, message)                                          \
{                                                                            \
  if(!(condition))                                                           \
  {                                                                          \
    std::cerr << __FILE__ << " (" << __LINE__ << ") : " << message << endl;  \
    abort();                                                                 \
  }                                                                          \
}
#else
#define DASSERT(condition, message) {}
#endif

#endif // CNF_H__
