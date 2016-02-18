// ----------------------------------------------------------------------------
// Copyright (c) 2015 by Graz University of Technology
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
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file AndCacheFor2Simulators.h
/// @brief Contains the declaration of the class AndCacheFor2Simulators.
// -------------------------------------------------------------------------------------------

#ifndef AndCacheFor2Simulators_H__
#define AndCacheFor2Simulators_H__

#include "defines.h"
#include "SatSolver.h"

// -------------------------------------------------------------------------------------------
///
/// @class AndCacheFor2Simulators
/// @brief A simple cache for two simulators running (almost) the same circuit.
///
/// @author Patrick Klampfl
/// @version 1.2.0
class AndCacheFor2Simulators
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
  AndCacheFor2Simulators(vector<int> &results1, vector<int> &results2, SatSolver* solver, int& next_free_cnf_var);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~AndCacheFor2Simulators();

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new AND-Gate, if it is not already in the cache.
  void addAndGate(int lhs_aig_lit, int rhs0_aig_lit, int rhs1_aig_lit);

protected:

  vector<int> &results1_;
  vector<int> &results2_;
  SatSolver* solver_;
  int &next_free_cnf_var_;

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  AndCacheFor2Simulators(const AndCacheFor2Simulators &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  AndCacheFor2Simulators& operator=(const AndCacheFor2Simulators &other);

};

#endif // AndCacheFor2Simulators_H__
