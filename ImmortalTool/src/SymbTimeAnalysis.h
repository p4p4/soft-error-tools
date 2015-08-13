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
/// @file SymbTimeAnalysis.h
/// @brief Contains the declaration of the class SymbTimeAnalysis.
// -------------------------------------------------------------------------------------------

#ifndef SymbTimeAnalysis_H__
#define SymbTimeAnalysis_H__

#include "defines.h"
#include "BackEnd.h"

// -------------------------------------------------------------------------------------------
///
/// @class SymbTimeAnalysis
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class SymbTimeAnalysis : public BackEnd
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
  SymbTimeAnalysis(aiger* circuit, int num_err_latches, int mode=0);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~SymbTimeAnalysis();

protected:

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  SymbTimeAnalysis(const SymbTimeAnalysis &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  SymbTimeAnalysis& operator=(const SymbTimeAnalysis &other);

};

#endif // SymbTimeAnalysis_H__
