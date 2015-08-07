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
		circuit_(circuit), sim_(0), mode_(mode)
{
	sim_ = new AigSimulator(circuit_);
}

// -------------------------------------------------------------------------------------------
SimulationBasedAnalysis::~SimulationBasedAnalysis()
{
	delete sim_;
}

// -------------------------------------------------------------------------------------------
const set<unsigned>& SimulationBasedAnalysis::getVulnerableElements() const
{
	return vulnerable_elements_;
}

// -------------------------------------------------------------------------------------------
bool SimulationBasedAnalysis::findVulnerabilities(vector<TestCase> &testcases)
{
	for (unsigned tc_idx = 0; tc_idx < testcases.size(); tc_idx++)
	{
		sim_->setTestcase(testcases[tc_idx]);
		findVulnerabilitiesForCurrentTC();
	}

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
bool SimulationBasedAnalysis::findVulnerabilities(
		vector<char*> paths_to_TC_files)
{
	for (unsigned tc_idx = 0; tc_idx < paths_to_TC_files.size(); tc_idx++)
	{
		sim_->setTestcase(paths_to_TC_files[tc_idx]);
		findVulnerabilitiesForCurrentTC();
	}

	return (vulnerable_elements_.size() != 0);
}

void SimulationBasedAnalysis::findVulnerabilitiesForCurrentTC()
{
//	vulnerable_latches = empty_set/list
//for each test case t[][]

	vector<vector<int> > outputs;
	vector<vector<int> > states;

//  states[][], outputs[][] = simulate(t)
	while (sim_->simulateOneTimeStep() == true)
	{
		// TODO: this could be done more efficient if we directly read from sim_->res_[]
		outputs.push_back(sim_->getOutputs());
		states.push_back(sim_->getLatchValues());

		sim_->switchToNextState();
	}


//  for each latch l without vulnerable_latches:
	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		// skip latches where we already know that they are vulnerable
		if(vulnerable_elements_.find(circuit_->latches[cnt].lit) != vulnerable_elements_.end())
		{
			continue;
		}
		//    l_is_vulnerable = false
		bool l_is_vulnerable = false;
		//    for all time steps i von t:
		for (unsigned timestep=0; timestep < states.size(); timestep++)
		{
			//       if(l_is_vulnerable)
			//          break;
			if(l_is_vulnerable)
				break;
			//       state[] = states[i][]
			vector<int> &state = states[timestep];
			//       state[l] = !state[l]
			//       for all j >= i:
			//         next_state[], out[], alarm = simulate1step(state[], t[j])
			//         if(alarm)
			//            break;
			//         if(out[] != outputs[j][])
			//           vulnerable_latches.add(l)
			//           l_is_vulnerable = true
			//           break; // continue with next l
			//         if(next_state[] == states[j+1][])
			//           break;
		}
	}

}
