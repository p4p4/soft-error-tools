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
/// @file SimulationBasedAnalysis.cpp
/// @brief Contains the definition of the class SimulationBasedAnalysis.
// -------------------------------------------------------------------------------------------

#include "SimulationBasedAnalysis.h"
#include "Logger.h"
#include "Options.h"
#include "Utils.h"
#include "ErrorTraceManager.h"
extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SimulationBasedAnalysis::SimulationBasedAnalysis(aiger* circuit, int num_err_latches, int mode) :
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
bool SimulationBasedAnalysis::findVulnerabilities(vector<string> paths_to_TC_files)
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
	vector<vector<int> > outputs_ok;
	vector<vector<int> > states_ok;
	outputs_ok.reserve(current_TC_.size());
	states_ok.reserve(current_TC_.size());

	// simulate whole TestCase without error, store results
	while (sim_->simulateOneTimeStep() == true)
	{
		outputs_ok.push_back(sim_->getOutputs());
		states_ok.push_back(sim_->getLatchValues());
		sim_->switchToNextState();
	}

	// if environment-model: define which output is relevant at which point in time:
	vector<vector<int> > output_is_relevant;
	if(environment_model_)
	{
		TestCase env_tc = Utils::combineTestCases(current_TC_, outputs_ok);
		output_is_relevant.reserve(env_tc.size());

		AigSimulator environment_sim(environment_model_);
		environment_sim.setTestcase(env_tc);
		while (environment_sim.simulateOneTimeStep())
		{
			output_is_relevant.push_back(environment_sim.getOutputs());
		}
	}

	//  for each latch
	for (unsigned l_cnt = 0; l_cnt < circuit_->num_latches - num_err_latches_; ++l_cnt)
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
		for (unsigned timestep = 0; timestep < states_ok.size(); timestep++)
		{
			if (l_is_vulnerable)
				break;

			// current state
			vector<int> state = states_ok[timestep];

			// flip latch
			state[l_cnt] = aiger_not(state[l_cnt]); // don't forget to restore state again later
			AigSimulator sim_w_flip(circuit_);

			// for all j >= i:
			for (unsigned later_timestep = timestep; later_timestep < states_ok.size(); ++later_timestep)
			{
				// next_state[], out[], alarm = simulate1step(state[], t[later_timestep])
				sim_w_flip.simulateOneTimeStep(current_TC_[later_timestep], state);

				state = sim_w_flip.getNextLatchValues(); // state = next state
				vector<int> outputs_w_flip = sim_w_flip.getOutputs();

				// if(alarm)
				if (outputs_w_flip[circuit_->num_outputs - 1] == AIG_TRUE)
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					break;
				}

				// else if: no alarm but different output values ?
				bool different_outputs = (outputs_w_flip != outputs_ok[later_timestep]);
				bool wrong_outputs = different_outputs;
				if(environment_model_ && different_outputs) // check if output is relevant
				{
					wrong_outputs = false;
					for(unsigned out_idx=0; out_idx < outputs_ok.size(); out_idx++)
					{
						// if output is relevant
						if(output_is_relevant[later_timestep][out_idx] == AIG_TRUE)
						{
							// AND output value is different
							if(outputs_w_flip[out_idx] != outputs_ok[later_timestep][out_idx])
							{
								wrong_outputs = true;
								break;
							}
						}
					}

				}

				if (wrong_outputs)
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					vulnerable_elements_.insert(circuit_->latches[l_cnt].lit);

					if (Options::instance().isUseDiagnosticOutput())
					{
						ErrorTrace* trace = new ErrorTrace;
						trace->error_timestep_ = later_timestep;
						trace->flipped_timestep_ = timestep;
						trace->latch_index_ = circuit_->latches[l_cnt].lit;
						trace->input_trace_ = current_TC_;
						ErrorTraceManager::instance().error_traces_.push_back(trace);
					}

					l_is_vulnerable = true;
					break;
				}

				// else if (next_state[] == states[later_timestep+1][])
				if (sim_w_flip.getLatchValues() == states_ok[later_timestep + 1])
				{
					state[l_cnt] = aiger_not(state[l_cnt]); // undo bit-flip
					break;
				}

			}
		}
	}

}
