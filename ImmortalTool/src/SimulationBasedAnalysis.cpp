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
		circuit_(circuit), sim_(0), mode_(mode), current_TC_(empty_)
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
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();
	//for each test case t[][]
	for (unsigned tc_idx = 0; tc_idx < testcases.size(); tc_idx++)
	{
		sim_->setTestcase(testcases[tc_idx]);
		current_TC_ = testcases[tc_idx];
		findVulnerabilitiesForCurrentTC();
	}

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
bool SimulationBasedAnalysis::findVulnerabilities(
		vector<char*> paths_to_TC_files)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();
	//for each test case t[][]
	for (unsigned tc_idx = 0; tc_idx < paths_to_TC_files.size(); tc_idx++)
	{
		sim_->setTestcase(paths_to_TC_files[tc_idx]);
		current_TC_ = sim_->getTestcase();
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
	// simulate whole TestCase
	while (sim_->simulateOneTimeStep() == true)
	{
		// store results
		// TODO: this could be done more efficient if we directly read from sim_->res_[]
		outputs.push_back(sim_->getOutputs());
		states.push_back(sim_->getLatchValues());

		sim_->switchToNextState();
	}


//  for each latch l without vulnerable_latches:
	for (unsigned l_cnt = 0; l_cnt < circuit_->num_latches; ++l_cnt)
	{
		// skip latches where we already know that they are vulnerable
		if(vulnerable_elements_.find(circuit_->latches[l_cnt].lit) != vulnerable_elements_.end())
		{
			continue;
		}
		bool l_is_vulnerable = false;


		// for all time steps i of t:
		for (unsigned timestep=0; timestep < states.size(); timestep++)
		{
			if(l_is_vulnerable)
				break;

			// state[] = states[i][]
			vector<int> &state = states[timestep];

			// state[l] = !state[l]
			state[l_cnt] = aiger_not(state[l_cnt]); // TODO: restore state again later
			AigSimulator sim_w_flip(circuit_);

			// for all j >= i:
			for (unsigned j = timestep; j < states.size(); ++j)
			{
				// next_state[], out[], alarm = simulate1step(state[], t[j])
				sim_w_flip.simulateOneTimeStep(current_TC_[j], state);
				sim_w_flip.switchToNextState();

				vector<int> outputs_w_flip;
				vector<int> latches_w_flip;
				outputs_w_flip = sim_w_flip.getOutputs();
				latches_w_flip = sim_w_flip.getLatchValues();


				// if(alarm)
				if(outputs_w_flip[circuit_->num_outputs-1] == 1)
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					break;
				}


				// TODO: maybe this already includes the if(alarm){...}
				// if(out[] != outputs[j][])
				//   vulnerable_latches.add(l)
				//   l_is_vulnerable = true
				//   break; // continue with next l
				if(outputs_w_flip != outputs[j])
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					vulnerable_elements_.insert(circuit_->latches[l_cnt].lit);
					l_is_vulnerable = true;
					break;
				}

				// if(next_state[] == states[j+1][])
				if(latches_w_flip == states[j+1])
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					break;
				}


				// if last iteration of loop (and no 'break' statement before)
				if(timestep == states.size() - 1)
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
				}

			}
		}
	}

}
