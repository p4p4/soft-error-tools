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
/// @file BackEnd.cpp
/// @brief Contains the definition of the class BackEnd.
// -------------------------------------------------------------------------------------------
#include <algorithm>

#include "BackEnd.h"
#include "Utils.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
BackEnd::BackEnd(aiger* circuit, int num_err_latches, int mode, aiger* environment_model) :
		circuit_(circuit), environment_model_(environment_model), mode_(mode),
		current_TC_(empty_TC_), num_err_latches_(num_err_latches)
{
	// nothing to be done
}

// -------------------------------------------------------------------------------------------
BackEnd::~BackEnd()
{
	// nothing to be done
}

// -------------------------------------------------------------------------------------------
const set<unsigned>& BackEnd::getVulnerableElements() const
{
	return vulnerable_elements_;
}

// -------------------------------------------------------------------------------------------
bool BackEnd::analyzeWithRandomTestCases(unsigned num_of_TCs, unsigned num_of_timesteps)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();

	// 1. generate random testcases
	vector<TestCase> testcases;
	Utils::generateRandomTestCases(testcases, num_of_TCs,num_of_timesteps,circuit_->num_inputs);

	// 2. run testcases:
	return analyze(testcases);

}

bool BackEnd::analyzeModelChecking(unsigned num_of_timesteps)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();

	// 1. generate testcase
	vector<TestCase> testcases;
	testcases.reserve(1);

	TestCase tc;
	tc.reserve(num_of_timesteps);

	vector<int> inputs;
	inputs.reserve(circuit_->num_inputs);
	for(unsigned input=0; input < circuit_->num_inputs; input++)
		inputs.push_back(LIT_FREE);

	for (unsigned timestep = 0; timestep < num_of_timesteps; timestep++)
	{
		tc.push_back(inputs);
	}
	testcases.push_back(tc);

	// 2. run testcases:
	return analyze(testcases);
}

bool BackEnd::analyze(vector<string> paths_to_TC_files)
{
	vector<TestCase> testcases;
	//for each test case t[][]
	for (unsigned tc_index_ = 0; tc_index_ < paths_to_TC_files.size(); tc_index_++)
	{
		TestCase testcase;
		Utils::parseAigSimFile(paths_to_TC_files[tc_index_], testcase, circuit_->num_inputs);
		testcases.push_back(testcase);
	}

	return analyze(testcases);
}

void BackEnd::setEnvironmentModel(aiger* environmentModel)
{
	environment_model_ = environmentModel;
}
