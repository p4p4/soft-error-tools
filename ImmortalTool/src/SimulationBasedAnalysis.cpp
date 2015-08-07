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
/// @file SimulationBasedAnalysis.cpp
/// @brief Contains the definition of the class SimulationBasedAnalysis.
// -------------------------------------------------------------------------------------------

#include "SimulationBasedAnalysis.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SimulationBasedAnalysis::SimulationBasedAnalysis(aiger* circuit, int mode) :
		circuit_(circuit), mode_(mode)
{
}

// -------------------------------------------------------------------------------------------
SimulationBasedAnalysis::~SimulationBasedAnalysis()
{
}

// -------------------------------------------------------------------------------------------
const set<int>& SimulationBasedAnalysis::getVulnerableElements() const
{
	return vulnerable_elements_;
}

// -------------------------------------------------------------------------------------------
bool SimulationBasedAnalysis::findVulnerabilities(vector<TestCase> &testcases)
{
	// TODO implement
	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
bool SimulationBasedAnalysis::findVulnerabilities(
		vector<const char*> paths_to_TC_files)
{
	// TODO implement
	return (vulnerable_elements_.size() != 0);
}
