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
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file BddAnalysis.h
/// @brief Contains the declaration of the class BddAnalysis.
// -------------------------------------------------------------------------------------------

#ifndef BddAnalysis_H__
#define BddAnalysis_H__

#include "defines.h"
#include "BackEnd.h"
#include "Stopwatch.h"


extern "C" {
#include "aiger.h"
#include "cudd.h"
};

#include "cuddObj.hh"

// -------------------------------------------------------------------------------------------
///
/// @class BddAnalysis
/// @brief TODO
///
/// @author Patrick Klampfl
/// @version 1.2.0
class BddAnalysis : public BackEnd
{
public:

// for the linker: also use the inherited method(s) with the name findVulnerabilities
using BackEnd::analyze;

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
  BddAnalysis(aiger* circuit, int num_err_latches, int mode=0);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~BddAnalysis();

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases
///
/// tries to find vulnerabilities using the provided TestCases
///
/// @param testcases a vector of TestCases.
/// @return TRUE if vulnerabilities were found.
  bool analyze(vector<TestCase> &testcases);


protected:
  bool useStatistics_ = true;
  enum Statistic { CREATE_C_SIGNALS, SIM_ANDs, SWITCH_NXT_ST, OUT_IS_DIFF, SATISFIABILITY, STORE_MODEL, INIT_Latches, SIDE_CONSTRAINTS, MODIFY_LATCHES };
  map<Statistic, double> accumulated_durations_;
  PointInTime start_time_;
  void stopWatchStart();
  void stopWatchStore(Statistic statistic);
  void printStatistics(PointInTime begin);

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  BddAnalysis(const BddAnalysis &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  BddAnalysis& operator=(const BddAnalysis &other);

};

#endif // BddAnalysis_H__
