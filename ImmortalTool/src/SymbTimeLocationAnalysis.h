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
/// @file SymbTimeLocationAnalysis.h
/// @brief Contains the declaration of the class SymbTimeLocationAnalysis.
// -------------------------------------------------------------------------------------------

#ifndef SymbTimeLocationAnalysis_H__
#define SymbTimeLocationAnalysis_H__

#include "defines.h"
#include "BackEnd.h"
#include "SatSolver.h"
#include "AigSimulator.h"

// -------------------------------------------------------------------------------------------
///
/// @class SymbTimeLocationAnalysis
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class SymbTimeLocationAnalysis: public BackEnd
{
	public:

	using BackEnd::findVulnerabilities;

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	SymbTimeLocationAnalysis(aiger* circuit, int num_err_latches, int mode = 0);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
	virtual ~SymbTimeLocationAnalysis();

enum AnalysisMode {
	STANDARD = 0,
	FREE_INPUTS = 1
};

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
/// @brief tries to find vulnerabilities using the provided TestCases.
///	Point in time as well as location is symbolic
///
/// STANDARD mode.
///
/// Algorithm ANALYZE2: point in time and location is symbolic
///
/// @param testcase a vector of input vectors
	void Analyze2(vector<TestCase> &testcases);


// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases.
///	Point in time as well as location is symbolic. Some inputs might
/// as well be symbolic, i.e. have undefined values ('?')
///
/// STANDARD mode.
///
/// Algorithm ANALYZE2: point in time and location is symbolic
///
/// @param testcase a vector of input vectors
	void Analyze2_free_inputs(vector<TestCase> &testcases);

// -------------------------------------------------------------------------------------------
///
/// @brief the Aiger Simulator instance
	AigSimulator* sim_;

// -------------------------------------------------------------------------------------------
///
/// @brief the Sat-Solver instance
	SatSolver* solver_;

// 0 = disabled, 1 = every iteration, 2 = every 2nd iteration, ...
unsigned unsat_core_interval_;
	private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	SymbTimeLocationAnalysis(const SymbTimeLocationAnalysis &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	SymbTimeLocationAnalysis& operator=(const SymbTimeLocationAnalysis &other);

};

#endif // SymbTimeLocationAnalysis_H__
