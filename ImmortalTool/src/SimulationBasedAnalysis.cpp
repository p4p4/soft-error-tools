// ----------------------------------------------------------------------------
// Copyright (c) 2013-2014 by Graz University of Technology
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
#include "Logger.h"
#include "Utils.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SimulationBasedAnalysis::SimulationBasedAnalysis(aiger* circuit,
		int num_err_latches, int mode) :
		BackEnd(circuit, num_err_latches, mode), tc_index_(0)
{
	sim_ = new AigSimulator(circuit_);
}

// -------------------------------------------------------------------------------------------
SimulationBasedAnalysis::~SimulationBasedAnalysis()
{
	delete sim_;
}

// -------------------------------------------------------------------------------------------
bool SimulationBasedAnalysis::findVulnerabilities(vector<TestCase> &testcases)
{
	vulnerable_elements_.clear();
	//for each test case:
	for (tc_index_ = 0; tc_index_ < testcases.size(); tc_index_++)
	{
		sim_->setTestcase(testcases[tc_index_]);
		current_TC_ = testcases[tc_index_];
		findVulnerabilitiesForCurrentTC();
	}

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
bool SimulationBasedAnalysis::findVulnerabilities(
		vector<string> paths_to_TC_files)
{
	vulnerable_elements_.clear();
	//for each test case
	for (tc_index_ = 0; tc_index_ < paths_to_TC_files.size(); tc_index_++)
	{
		sim_->setTestcase(paths_to_TC_files[tc_index_]);
		current_TC_ = sim_->getTestcase();
		findVulnerabilitiesForCurrentTC();
	}

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
void SimulationBasedAnalysis::findVulnerabilitiesForCurrentTC()
{
	vector<vector<int> > outputs;
	vector<vector<int> > states;
	outputs.reserve(current_TC_.size());
	states.reserve(current_TC_.size());

	// simulate whole TestCase without error, store results
	while (sim_->simulateOneTimeStep() == true)
	{
		if(tc_index_==6)
			L_DBG("TC6: " << sim_->getStateString())
		outputs.push_back(sim_->getOutputs());
		states.push_back(sim_->getLatchValues());
		sim_->switchToNextState();
	}

	//  for each latch
	for (unsigned l_cnt = 0; l_cnt < circuit_->num_latches - num_err_latches_;
			++l_cnt)
	{

		// skip latches where we already know that they are vulnerable
		if ((tc_index_ != 0)
				&& (vulnerable_elements_.find(circuit_->latches[l_cnt].lit)
						!= vulnerable_elements_.end()))
		{
			continue;
		}
		bool l_is_vulnerable = false;

		// for all time steps i of t:
		for (unsigned timestep = 0; timestep < states.size(); timestep++)
		{
			if (l_is_vulnerable)
				break;

			// current state
			vector<int> state = states[timestep];

			// flip latch
			state[l_cnt] = aiger_not(state[l_cnt]); // don't forget to restore state again later
			AigSimulator sim_w_flip(circuit_);

//			string debugstring = "\n";
			// for all j >= i:
			for (unsigned j = timestep; j < states.size(); ++j)
			{
				// next_state[], out[], alarm = simulate1step(state[], t[j])
				sim_w_flip.simulateOneTimeStep(current_TC_[j], state);

//				debugstring += sim_w_flip.getStateString() + "\n";
				state  = sim_w_flip.getNextLatchValues(); // state = next state
				vector<int> outputs_w_flip = sim_w_flip.getOutputs();

				// if(alarm)
				if (outputs_w_flip[circuit_->num_outputs - 1] == 1)
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					break;
				}

				// else if: no alarm but different output values
				if (outputs_w_flip != outputs[j])
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					vulnerable_elements_.insert(circuit_->latches[l_cnt].lit);
//					L_DBG("[sim] found vulnerability " << circuit_->latches[l_cnt].lit <<"(latch.lit) at i,j=" << timestep <<","<<j<<" in testcase number " << tc_index_);
//					L_DBG("flipped "<<debugstring)
//					Utils::debugPrint(outputs[j], "outputs");
//					Utils::debugPrint(outputs_w_flip, "outputs_f");

					l_is_vulnerable = true;
					break;
				}

				// else if (next_state[] == states[j+1][])
				if (sim_w_flip.getLatchValues() == states[j + 1])
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					break;
				}

			}
		}
	}

}
