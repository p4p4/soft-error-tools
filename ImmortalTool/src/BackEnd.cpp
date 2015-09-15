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


extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
BackEnd::BackEnd(aiger* circuit, int num_err_latches, int mode) :
		circuit_(circuit), mode_(mode), current_TC_(empty_), num_err_latches_(num_err_latches)
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
bool BackEnd::findVulnerabilities(unsigned num_of_TCs,
		unsigned num_of_timesteps)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();

	// 1. generate random testcases
	vector<TestCase> testcases;
	testcases.reserve(num_of_TCs);
	for(unsigned tc_i=0; tc_i < num_of_TCs; tc_i++)
	{
		TestCase tc;
		tc.reserve(num_of_timesteps);
		for(unsigned timestep=0;timestep< num_of_timesteps;timestep++)
		{
			vector<int> inputs;
			inputs.reserve(circuit_->num_inputs);
			generate_n(back_inserter(inputs), circuit_->num_inputs, gen_rand());
			tc.push_back(inputs);
		}
		testcases.push_back(tc);
	}


	// 2. run testcases:
	return findVulnerabilities(testcases);

}
