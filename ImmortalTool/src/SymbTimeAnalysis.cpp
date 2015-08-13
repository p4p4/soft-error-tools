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
//   <http://www.iaik.tugraz.at/content/research/design_verification/others/>
// or email the authors directly.
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file SymbTimeAnalysis.cpp
/// @brief Contains the definition of the class SymbTimeAnalysis.
// -------------------------------------------------------------------------------------------

#include "SymbTimeAnalysis.h"

// -------------------------------------------------------------------------------------------
SymbTimeAnalysis::SymbTimeAnalysis(aiger* circuit, int num_err_latches,
		int mode) :
		BackEnd(circuit, num_err_latches, mode)
{
	MASSERT(false, "Implement me");
}

// -------------------------------------------------------------------------------------------
SymbTimeAnalysis::~SymbTimeAnalysis()
{
	MASSERT(false, "Implement me");
}

// -------------------------------------------------------------------------------------------
bool SymbTimeAnalysis::findVulnerabilities(vector<TestCase> &testcases)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();
	//for each test case t[][]
	for (unsigned tc_index_ = 0; tc_index_ < testcases.size(); tc_index_++)
	{
		MASSERT(false, "Implement me");
	}

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
bool SymbTimeAnalysis::findVulnerabilities(
		vector<string> paths_to_TC_files)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();
	//for each test case t[][]
	for (unsigned tc_index_ = 0; tc_index_ < paths_to_TC_files.size(); tc_index_++)
	{
		MASSERT(false, "Implement me");
	}

	return (vulnerable_elements_.size() != 0);
}

