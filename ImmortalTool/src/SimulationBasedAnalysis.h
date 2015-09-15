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
/// @file SimulationBasedAnalysis.h
/// @brief Contains the declaration of the class SimulationBasedAnalysis.
// -------------------------------------------------------------------------------------------

#ifndef SimulationBasedAnalysis_H__
#define SimulationBasedAnalysis_H__

#include "defines.h"
#include "AigSimulator.h"
#include "BackEnd.h"

// -------------------------------------------------------------------------------------------
///
/// @class SimulationBasedAnalysis
/// @brief Simulates a circuit using provided TestCases, searches fur vulnerable Elements.
///
/// @author Patrick Klampfl
/// @version 1.2.0

struct aiger;

class SimulationBasedAnalysis : public BackEnd
{
public:

	// for the linker: also use the inherited method(s) with the name findVulnerabilities
	using BackEnd::findVulnerabilities;

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
  SimulationBasedAnalysis(aiger* circuit, int num_err_latches, int mode=0);


// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~SimulationBasedAnalysis();



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
	bool findVulnerabilities(vector<string> paths_to_TC_files);


protected:

// -------------------------------------------------------------------------------------------
///
/// @brief the Aiger Simulator instance
  AigSimulator* sim_;

// -------------------------------------------------------------------------------------------
///
/// @brief the index of the currnt TestCase
  unsigned tc_index_;


// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities for the current TestCase.
///
/// gets called by the public findVulnerabilities() functions.
///
  void findVulnerabilitiesForCurrentTC();

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities for a random TestCase.
///
/// gets called by the public findVulnerabilities(num_of_TCs,num_of_timesteps) function.
///
	void findVulnerabilitiesForRandomTC(unsigned num_of_timesteps);
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
