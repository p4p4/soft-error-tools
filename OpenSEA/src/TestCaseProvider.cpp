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
/// @file TestCaseProvider.cpp
/// @brief Contains the definition of the class TestCaseProvider.
// -------------------------------------------------------------------------------------------

#include "TestCaseProvider.h"
#include "Options.h"
#include "Utils.h"

TestCaseProvider *TestCaseProvider::instance_ = NULL;

TestCaseProvider& TestCaseProvider::instance()
{
	if (instance_ == NULL)
		instance_ = new TestCaseProvider;
	MASSERT(instance_ != NULL, "Could not create TestCaseProvider instance.");
	return *instance_;
}

// -------------------------------------------------------------------------------------------
TestCaseProvider::TestCaseProvider() : circuit_(Options::instance().getCircuit())
{
}

vector<TestCase> TestCaseProvider::getTestcases()
{
	int tc_mode = Options::instance().getTestcaseMode();
	switch (tc_mode)
	{
	case Options::TC_RANDOM:
	{
		return generateRandomTestCases(Options::instance().getNumTestcases(),
				Options::instance().getLenRandTestcases());
	}
	case Options::TC_FILES:
	{
		return readTestcasesFromFiles(Options::instance().getPathsToTestcases());
	}
	case Options::TC_MC:
	{
		return generateMcTestCase(Options::instance().getLenRandTestcases());
	}
	default:
		MASSERT(false, "No test-case provided.")
	}
}

vector<TestCase> TestCaseProvider::readTestcasesFromFiles(vector<string> paths_to_TC_files)
{
	vector<TestCase> testcases;
	//for each test case t[][]
	for (unsigned tc_index_ = 0; tc_index_ < paths_to_TC_files.size(); tc_index_++)
	{
		TestCase testcase;
		Utils::parseAigSimFile(paths_to_TC_files[tc_index_], testcase, circuit_->num_inputs);
		testcases.push_back(testcase);
	}

	return testcases;
}

vector<TestCase> TestCaseProvider::generateMcTestCase(unsigned num_of_timesteps)
{
	vector<TestCase> testcases;
	testcases.reserve(1);

	TestCase tc;
	tc.reserve(num_of_timesteps);

	vector<int> inputs;
	inputs.reserve(circuit_->num_inputs);
	for (unsigned input = 0; input < circuit_->num_inputs; input++)
		inputs.push_back(LIT_FREE);

	for (unsigned timestep = 0; timestep < num_of_timesteps; timestep++)
	{
		tc.push_back(inputs);
	}
	testcases.push_back(tc);

	return testcases;
}

vector<TestCase> TestCaseProvider::generateRandomTestCases(unsigned num_of_TCs,
		unsigned num_of_timesteps)
{
	vector<TestCase> testcases;
	if (Options::instance().num_open_inputs_ == 0)
		Utils::generateRandomTestCases(testcases, num_of_TCs, num_of_timesteps,
				circuit_->num_inputs);
	else
		Utils::generateRandomTestCases(testcases, num_of_TCs, num_of_timesteps, circuit_->num_inputs,
				Options::instance().num_open_inputs_);

	return testcases;
}

void TestCaseProvider::setCircuit(aiger* circuit)
{
	circuit_ = circuit;
}

// -------------------------------------------------------------------------------------------
TestCaseProvider::~TestCaseProvider()
{
}

