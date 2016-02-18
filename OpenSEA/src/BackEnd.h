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
/// @file BackEnd.h
/// @brief Contains the declaration of the class BackEnd.
// -------------------------------------------------------------------------------------------

#ifndef BackEnd_H__
#define BackEnd_H__

#include "defines.h"

struct aiger;
// -------------------------------------------------------------------------------------------
///
/// @class BackEnd
/// @brief An interface for the back-ends.
///
/// This class provides an interface for all back-ends. Every back-end implements a different
/// Soft-Error-Analysis algorithm. This class is abstract, i.e., objects of this class cannot
/// be instantiated. Instantiate one of the derived classes instead.
///
/// @author Patrick Klampfl
/// @version 1.2.0
class BackEnd
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	BackEnd(aiger* circuit, int num_err_latches, int mode, aiger* environment_model = 0);

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	virtual ~BackEnd();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the set of vulnerable elements of the circuit
///
/// @return The set of vulnerable elements of the circuit
	const set<unsigned>& getVulnerableElements() const;

/// @return The number of vulnerable elements / number of superfluous traces
	unsigned int getNumberOfErrors();

	void printErrorTraces();

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases
///
/// tries to find vulnerabilities using the provided TestCases
///
/// @param testcases a vector of TestCases.
/// @return TRUE if vulnerabilities were found.
	virtual bool analyze(vector<TestCase> &testcases) = 0;

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases
///
/// tries to find vulnerabilities using the provided TestCases
///
/// @param paths_to_TC_files a vector of paths to TestCase files.
/// @return TRUE if vulnerabilities were found.
	bool analyze(vector<string> paths_to_TC_files);

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using random input vectors
///
/// tries to find vulnerabilities using num_of_timesteps random input vectors
///
/// @param num_of_TCs the number of random testcases to use
/// @param num_of_timesteps the number random input vectors to test
/// @return TRUE if vulnerabilities were found.
	bool analyzeWithRandomTestCases(unsigned num_of_TCs,
		unsigned num_of_timesteps);

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using open input vectors (model-checking approach)
///
/// tries to find vulnerabilities within num_of_timesteps open input vectors
/// Attention: This method can only be used with modes which suppurt free inputs!
///
/// @param num_of_timesteps the number of timesteps to search for vulnerabilities
/// @return TRUE if vulnerabilities were found.
	bool analyzeModelChecking(unsigned num_of_timesteps);

// -------------------------------------------------------------------------------------------
///
/// @brief allows to set an environment-model
///
/// an environment-model is an aiger-circuit with the same number of inputs and outputs as the
/// original circuit. These outputs define when an output of the original may be (ir)relevant.
///
/// @param environmentModel the environment-model
	void setEnvironmentModel(aiger* environmentModel);

// -------------------------------------------------------------------------------------------
///
/// @brief randomly generates 0 or 1
///
	struct gen_rand {
	public:
	    gen_rand() {}
	    int operator()() {
	        return rand() % 2;
	    }
	};
protected:
// -------------------------------------------------------------------------------------------
///
/// @brief the circuit to analyze
	aiger* circuit_;

	aiger* environment_model_;

// -------------------------------------------------------------------------------------------
///
/// @brief the mode (if there are more than one version and/or optimizations to enable)
	int mode_;

// -------------------------------------------------------------------------------------------
///
/// @brief the list of vulnerabilities found so for
	set<unsigned> vulnerable_elements_;


// -------------------------------------------------------------------------------------------
///
/// @brief the number of error-latches. Error latches are additional latches used for the
/// protection circuit.
	unsigned num_err_latches_;

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	BackEnd(const BackEnd &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	BackEnd& operator=(const BackEnd &other);

};

#endif // BackEnd_H__
