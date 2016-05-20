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
#include <math.h>

#include "SimulationBasedAnalysis.h"
#include "Logger.h"
#include "Options.h"
#include "Utils.h"
#include "ErrorTraceManager.h"
#include "TestCaseProvider.h"
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

void SimulationBasedAnalysis::analyze()
{
	vector<TestCase> testcases = TestCaseProvider::instance().getTestcases();
	analyze(testcases);
}

// -------------------------------------------------------------------------------------------
bool SimulationBasedAnalysis::analyze(vector<TestCase> &testcases)
{
	detected_latches_.clear();
	//for each test case:
	for (tc_index_ = 0; tc_index_ < testcases.size(); tc_index_++)
	{
		if (mode_ == STANDARD)
			findVulnerabilitiesForTC(testcases[tc_index_]);
		else if (mode_ == FREE_INPUTS)
			findVulnerabilitiesForTCFreeInputs(testcases[tc_index_]);
		else
			MASSERT(false, "unknown mode!");
	}

	return (detected_latches_.size() != 0);
}

// -------------------------------------------------------------------------------------------
void SimulationBasedAnalysis::findVulnerabilitiesForTC(TestCase& test_case)
{
	sim_->setTestcase(test_case);

	vector<vector<int> > outputs_ok;
	vector<vector<int> > states_ok;
	outputs_ok.reserve(test_case.size());
	states_ok.reserve(test_case.size());

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
		TestCase env_tc = Utils::combineTestCases(test_case, outputs_ok);
		output_is_relevant.reserve(env_tc.size());

		AigSimulator environment_sim(environment_model_);
		environment_sim.setTestcase(env_tc);
		while (environment_sim.simulateOneTimeStep())
		{
			output_is_relevant.push_back(environment_sim.getOutputs());
		}
	}


	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_, num_err_latches_);

	// maps literals of latches to check to the corresponding aiger latch indices
	map<unsigned, unsigned> literal_to_idx;
	unsigned j = 0;
	for (unsigned i = 0; i < circuit_->num_latches && j < latches_to_check.size(); ++i)
	{
		while (circuit_->latches[i].lit != latches_to_check[j])
			i++;

		literal_to_idx[latches_to_check[j]] = i;
		j++;
	}

	//  for each latch
	for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
	{

		// skip latches where we already know that they are vulnerable
		if ((tc_index_ != 0)
				&& (detected_latches_.find(latches_to_check[l_cnt])
						!= detected_latches_.end()))
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


			unsigned idx_of_current_latch = literal_to_idx[latches_to_check[l_cnt]];
			// flip latch
			state[idx_of_current_latch] = aiger_not(state[idx_of_current_latch]); // don't forget to restore state again later
			AigSimulator sim_w_flip(circuit_);

			// for all j >= i:
			for (unsigned later_timestep = timestep; later_timestep < states_ok.size(); ++later_timestep)
			{
				// next_state[], out[], alarm = simulate1step(state[], t[later_timestep])
				sim_w_flip.simulateOneTimeStep(test_case[later_timestep], state);

				state = sim_w_flip.getNextLatchValues(); // state = next state
				vector<int> outputs_w_flip = sim_w_flip.getOutputs();

				// if(alarm)
				if (outputs_w_flip[circuit_->num_outputs - 1] == AIG_TRUE)
				{
					state[idx_of_current_latch] = aiger_not(state[idx_of_current_latch]); // undo bit-flip
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
					state[idx_of_current_latch] = aiger_not(state[idx_of_current_latch]); // undo bit-flip
					detected_latches_.insert(circuit_->latches[idx_of_current_latch].lit);

					if (Options::instance().isUseDiagnosticOutput())
					{
						ErrorTrace* trace = new ErrorTrace;
						trace->error_timestep_ = later_timestep;
						trace->flipped_timestep_ = timestep;
						trace->latch_index_ = circuit_->latches[idx_of_current_latch].lit;
						trace->input_trace_ = test_case;
						ErrorTraceManager::instance().error_traces_.push_back(trace);
					}

					l_is_vulnerable = true;
					break;
				}

				// else if (next_state[] == states[later_timestep+1][])
				if (sim_w_flip.getLatchValues() == states_ok[later_timestep + 1])
				{
					state[idx_of_current_latch] = aiger_not(state[idx_of_current_latch]); // undo bit-flip
					break;
				}

			}
		}
	}

}

void SimulationBasedAnalysis::findVulnerabilitiesForTCFreeInputs(TestCase& test_case)
{
	// find number of free inputs
	unsigned num_free_inputs = 0;
	for (unsigned timestep = 0; timestep < test_case.size(); timestep++)
	{
		for(unsigned input= 0; input  < test_case[timestep].size(); input ++)
		{
			if (test_case[timestep][input] == LIT_FREE)
				num_free_inputs++;
		}
	}

	if (num_free_inputs == 0)
		findVulnerabilitiesForTC(test_case);

	// check for overflow
	L_DBG("SIM free inputs: " << num_free_inputs)
	MASSERT(num_free_inputs <= sizeof(unsigned long long) * 8, "too many free inputs")

	for (unsigned long long input_vector = 0; input_vector < pow(2,num_free_inputs); input_vector++)
	{

		TestCase concrete_test_case = test_case;
		unsigned free_input_ctr = 0;
		for (unsigned timestep = 0; timestep < concrete_test_case.size(); timestep++)
		{
			for(unsigned input = 0; input  < concrete_test_case[timestep].size(); input++)
			{
				if (concrete_test_case[timestep][input] == LIT_FREE)
				{
					if ((input_vector & (1 << free_input_ctr)) > 0) // bit is set
						concrete_test_case[timestep][input] = AIG_TRUE;
					else
						concrete_test_case[timestep][input] = AIG_FALSE;

					free_input_ctr++;
				}
			}
		}

		findVulnerabilitiesForTC(concrete_test_case);
	}

}


