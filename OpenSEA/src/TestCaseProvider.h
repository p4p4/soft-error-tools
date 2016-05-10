// ----------------------------------------------------------------------------
// Copyright (c) 2016 by Graz University of Technology
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
/// @file TestCaseProvider.h
/// @brief Contains the declaration of the class TestCaseProvider.
// -------------------------------------------------------------------------------------------

#ifndef TestCaseProvider_H__
#define TestCaseProvider_H__

#include "defines.h"
extern "C"
{
#include "aiger.h"
}

class TestCaseProvider
{
public:

	static TestCaseProvider& instance();

	vector<TestCase> getTestcases();
	vector<TestCase> readTestcasesFromFiles(vector<string>);
	vector<TestCase> generateMcTestCase(unsigned num_of_timesteps);
	vector<TestCase> generateRandomTestCases(unsigned num_testcases, unsigned num_timesteps);

	void setCircuit(aiger* circuit);

protected:
	aiger* circuit_;
	static TestCaseProvider *instance_;
	TestCaseProvider();
	virtual ~TestCaseProvider();
private:




// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  TestCaseProvider(const TestCaseProvider &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  TestCaseProvider& operator=(const TestCaseProvider &other);

};

#endif // TestCaseProvider_H__
