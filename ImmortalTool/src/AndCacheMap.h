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
//   <http://www.iaik.tugraz.at/content/research/design_verification/others/>
// or email the authors directly.
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file AndCacheMap.h
/// @brief Contains the declaration of the class AndCacheMap.
// -------------------------------------------------------------------------------------------

#ifndef AndCacheMap_H__
#define AndCacheMap_H__

#include <stdint.h>
#include "defines.h"
#include "SatSolver.h"


// -------------------------------------------------------------------------------------------
///
/// @class AndCacheMap
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class AndCacheMap
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
  AndCacheMap(SatSolver* solver);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~AndCacheMap();

  int addAndGate(int left, int right, int& next_free_cnf_var);

  void clearCache();

protected:
  map<uint64_t, int> cache_;
  SatSolver* solver_;

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  AndCacheMap(const AndCacheMap &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  AndCacheMap& operator=(const AndCacheMap &other);

};

#endif // AndCacheMap_H__
