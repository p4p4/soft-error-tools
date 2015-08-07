// ----------------------------------------------------------------------------
// Copyright (c) 2013-2014 by Graz University of Technology
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
/// @file SimulationBasedAnalysis.h
/// @brief Contains the declaration of the class SimulationBasedAnalysis.
// -------------------------------------------------------------------------------------------

#ifndef SimulationBasedAnalysis_H__
#define SimulationBasedAnalysis_H__

#include "defines.h"
#include "AigSimulator.h"

// -------------------------------------------------------------------------------------------
///
/// @class SimulationBasedAnalysis
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0

struct aiger;

class SimulationBasedAnalysis
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
  SimulationBasedAnalysis(aiger* circuit, int mode=0);


// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~SimulationBasedAnalysis();


// -------------------------------------------------------------------------------------------
///
/// @brief Returns the set of vulnerable elements of the circuit
///
/// @return The set of vulnerable elements of the circuit
	const set<int>& getVulnerableElements() const;


// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases
///
/// tries to find vulnerabilities using the provided TestCases
///
/// @param testcases a vector of TestCases.
/// @return TRUE if vulnerabilities were found.
	bool findVulnerabilities(vector<TestCase> &testcases);


// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases
///
/// tries to find vulnerabilities using the provided TestCases
///
/// @param paths_to_TC_files a vector of paths to TestCase files.
/// @return TRUE if vulnerabilities were found.
	bool findVulnerabilities(vector<char*> paths_to_TC_files);


protected:
  aiger* circuit_;
  AigSimulator* sim_;
  int mode_;
  set<int> vulnerable_elements_;

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities for the current TestCase.
///
/// gets called by the public findVulnerabilities() functions.
///
  void findVulnerabilitiesForCurrentTC();

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  SimulationBasedAnalysis(const SimulationBasedAnalysis &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  SimulationBasedAnalysis& operator=(const SimulationBasedAnalysis &other);

};

#endif // SimulationBasedAnalysis_H__
