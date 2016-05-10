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
/// @file FalsePositives.h
/// @brief Contains the declaration of the class FalsePositives.
// -------------------------------------------------------------------------------------------

#ifndef FalsePositives_H__
#define FalsePositives_H__

#include "defines.h"
#include "BackEnd.h"
#include "SuperFluousTrace.h"

struct aiger;

// -------------------------------------------------------------------------------------------
///
/// @class FalsePositives
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class FalsePositives : public BackEnd
{
public:
	using BackEnd::analyze;


	enum AnalysisMode
	{
		SYMB_TIME = 0,
		SYMB_TIME_LOCATION = 1,
		SYMB_TIME_INPUTS = 2,
		SYMB_TIME_LOCATION_INPUTS = 3
	};
// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	FalsePositives(aiger* circuit, int num_err_latches, int mode = 0);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
	virtual ~FalsePositives();


	// -------------------------------------------------------------------------------------------
	///
	/// @brief tries to find vulnerabilities using the provided TestCases
	///
	/// tries to find vulnerabilities using the provided TestCases
	///
	/// @param testcases a vector of TestCases.
	/// @return TRUE if vulnerabilities were found.
		bool analyze(vector<TestCase> &testcases);
		void analyze();


	// -------------------------------------------------------------------------------------------
	///
	/// @brief tries to find false positives using the provided TestCases
	///
	/// @param testcases a vector of TestCases.
	/// @return TRUE if vulnerabilities were found.
	bool findFalsePositives_1b(vector<TestCase> &testcases);

	// -------------------------------------------------------------------------------------------
	///
	/// @brief tries to find false positives using the provided TestCases, free inputs allowed
	///
	/// @param testcases a vector of TestCases.
	/// @return TRUE if vulnerabilities were found.
	bool findFalsePositives_1b_free_inputs(vector<TestCase> &testcases);

	// -------------------------------------------------------------------------------------------
	///
	/// @brief tries to find false positives using the provided TestCases
	///
	/// @param testcases a vector of TestCases.
	/// @return TRUE if vulnerabilities were found.
	bool findFalsePositives_2b(vector<TestCase> &testcases);

	// -------------------------------------------------------------------------------------------
	///
	/// @brief tries to find false positives using the provided TestCases, free inputs allowed
	///
	/// @param testcases a vector of TestCases.
	/// @return TRUE if vulnerabilities were found.
	bool findFalsePositives_2b_free_inputs(vector<TestCase> &testcases);


/// @return The number of vulnerable elements / number of superfluous traces
	unsigned int getNumberOfErrors();

	void printErrorTraces();


	vector<SuperfluousTrace*> getSuperfluous()
	{
		return superfluous;
	}

protected:
	// -------------------------------------------------------------------------------------------
	///
	/// @brief the circuit to analyze
	aiger* circuit_;

	// -------------------------------------------------------------------------------------------
	///
	/// @brief the mode (if there are more than one version and/or optimizations to enable)
	int mode_;

	// -------------------------------------------------------------------------------------------
	///
	/// @brief the number of error-latches. Error latches are additional latches used for the
	/// protection circuit.
	unsigned num_err_latches_;

	void addSuperfluousTrace(int component, TestCase& testcase,  unsigned flip_timestep, unsigned alarm_timestep, unsigned error_gone_ts);


	vector<SuperfluousTrace*> superfluous;

	bool isEqualN(vector<int> a, vector<int> b, int elements_to_skip);

	void clearSuperfluousList();


private:


// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	FalsePositives(const FalsePositives &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	FalsePositives& operator=(const FalsePositives &other);

};

#endif // FalsePositives_H__
