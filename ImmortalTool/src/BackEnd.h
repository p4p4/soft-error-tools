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
/// synthesis algorithm. This class is abstract, i.e., objects of this class cannot be
/// instantiated. Instantiate one of the derived classes instead.
///
/// Many back-ends can be parameterized with a circuit extraction engine that extracts a
/// circuit from the winning region once the winning region has been computed.
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.2.0
class BackEnd
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	BackEnd(aiger* circuit, int num_err_latches, int mode);

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

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases
///
/// tries to find vulnerabilities using the provided TestCases
///
/// @param testcases a vector of TestCases.
/// @return TRUE if vulnerabilities were found.
	virtual bool findVulnerabilities(vector<TestCase> &testcases) = 0;

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases
///
/// tries to find vulnerabilities using the provided TestCases
///
/// @param paths_to_TC_files a vector of paths to TestCase files.
/// @return TRUE if vulnerabilities were found.
	virtual bool findVulnerabilities(vector<string> paths_to_TC_files) = 0;

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using random input vectors
///
/// tries to find vulnerabilities using num_of_timesteps random input vectors
///
/// @param num_of_TCs the number of random testcases to use
/// @param num_of_timesteps the number random input vectors to test
/// @return TRUE if vulnerabilities were found.
	bool findVulnerabilities(unsigned num_of_TCs,
		unsigned num_of_timesteps);

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
	aiger* circuit_;
	int mode_;
	set<unsigned> vulnerable_elements_;
	TestCase &current_TC_;
	TestCase empty_; // DUMMY TC
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
