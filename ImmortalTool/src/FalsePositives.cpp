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
/// @file FalsePositives.cpp
/// @brief Contains the definition of the class FalsePositives.
// -------------------------------------------------------------------------------------------

#include "FalsePositives.h"
#include "SatSolver.h"
#include "AigSimulator.h"
#include "SymbolicSimulator.h"
#include "Options.h"
#include "Utils.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
FalsePositives::FalsePositives(aiger* circuit, int num_err_latches)
{
	circuit_ = circuit;
	num_err_latches_ = num_err_latches;
	mode_ = 0;

}

// -------------------------------------------------------------------------------------------
FalsePositives::~FalsePositives()
{
}

bool FalsePositives::findFalsePositives_1b(vector<TestCase>& testcases)
{
	int next_free_cnf_var = 2;

	SatSolver* solver_ = Options::instance().getSATSolver();
	AigSimulator* sim_concrete = new AigSimulator(circuit_);
	SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);

	// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
//		cout << "latch " << c_cnt << endl;
		next_free_cnf_var = 2;

		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = component_aig >> 1;

		for (unsigned tci = 0; tci < testcases.size(); tci++)
		{
			TestCase& testcase = testcases[tci];

			// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
			vector<int> concrete_state_ok;
			concrete_state_ok.resize(circuit_->num_latches);
			sim_symb.initLatches(); // initialize latches to false

			// start new incremental SAT-solving session
			vector<int> vars_to_keep;
			vars_to_keep.push_back(CNF_FALSE);
			solver_->startIncrementalSession(vars_to_keep, 0);
			solver_->incAddUnitClause(CNF_TRUE);
			// set of literals to enable or disable the represented clauses, used for incremental solving.
			vector<int> enable_literals;

			// f = a set of variables fi indicating whether the latch is flipped in step i or not
			vector<int> f;
			map<int, unsigned> fi_to_timestep;

			// set of literals representing the alarm output for each timestep
			vector<int> alarm_literals;
			map<int, unsigned> alarmlit_to_timestep;

			for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
			{

				//------------------------------------------------------------------------------------
				// Concrete simulations:
				// correct simulation
				sim_concrete->simulateOneTimeStep(testcase[timestep], concrete_state_ok);
				vector<int> outputs_ok = sim_concrete->getOutputs();
				bool alarm_ok = outputs_ok.back() == AIG_TRUE;
				//Utils::debugPrint(outputs_ok,"outputs_ok");
				if (alarm_ok)
				{
					cout << "Alarm raised without Error!" << endl;
					return true;
				}
				vector<int> next_state_ok = sim_concrete->getNextLatchValues();

				// flip latch
				vector<int> faulty_state = concrete_state_ok;
				faulty_state[c_cnt] = (faulty_state[c_cnt] == AIG_TRUE) ? AIG_FALSE : AIG_TRUE;

				// faulty simulation with flipped latch
				sim_concrete->simulateOneTimeStep(testcase[timestep], faulty_state);
				vector<int> outputs_faulty = sim_concrete->getOutputs();
				vector<int> next_state_faulty = sim_concrete->getNextLatchValues();
				bool alarm_faulty = outputs_faulty.back() == AIG_TRUE;
				//------------------------------------------------------------------------------------

				bool equal_concrete_outputs = isEqualN(outputs_ok, outputs_faulty, 1);
				bool equal_concrete_states = (next_state_ok == next_state_faulty);//isEqualN(next_state_ok, next_state_faulty, num_err_latches_);

				sim_symb.setInputValues(testcase[timestep]);

				if (equal_concrete_outputs && equal_concrete_states && alarm_faulty)
				{
					addSuperfluousTrace(component_aig, testcase, timestep, timestep, timestep + 1);
					sim_symb.simulateOneTimeStep();
					alarm_literals.push_back(sim_symb.getAlarmValue());
					alarmlit_to_timestep[sim_symb.getAlarmValue()] = timestep;
					sim_symb.switchToNextState();

					// switch concrete simulation to next state
					concrete_state_ok = next_state_ok; // OR: change to sim_->switchToNextState();
					continue;
				}

				if (equal_concrete_outputs) // set TRUE for testing
				{
					// f variables:
					int fi = next_free_cnf_var++;
					solver_->addVarToKeep(fi);

					int old_value = sim_symb.getResultValue(component_cnf);
					if (old_value == CNF_TRUE) // old value is true
						sim_symb.setResultValue(component_cnf, -fi);
					else if (old_value == CNF_FALSE) // old value is false
						sim_symb.setResultValue(component_cnf, fi);
					else
					{
						int new_value = next_free_cnf_var++;
						solver_->addVarToKeep(new_value);
						// new_value = fi ? -old_value : old_value
						solver_->incAdd3LitClause(fi, old_value, -new_value);
						solver_->incAdd3LitClause(fi, -old_value, new_value);
						solver_->incAdd3LitClause(-fi, old_value, new_value);
						solver_->incAdd3LitClause(-fi, -old_value, -new_value);
						sim_symb.setResultValue(component_cnf, new_value);
					}

					// there might be at most one flip in one time-step:
					// if fi is true, all oter f must be false (fi -> -f1, fi -> -f2, ...)
					for (unsigned cnt = 0; cnt < f.size(); cnt++)
						solver_->incAdd2LitClause(-fi, -f[cnt]);

					f.push_back(fi);
					fi_to_timestep[fi] = timestep;

				}
				sim_symb.simulateOneTimeStep();
				alarm_literals.push_back(sim_symb.getAlarmValue());
				alarmlit_to_timestep[sim_symb.getAlarmValue()] = timestep;

				//cout << "alarm_value " << sim_symb.getAlarmValue() << endl;



				const vector<int> &state_cnf_values = sim_symb.getLatchValues();

				sim_symb.switchToNextState();
				//------------------------------------------------------------------------------------
				int curr_enable_literal = next_free_cnf_var++;
				solver_->addVarToKeep(curr_enable_literal);
				enable_literals.push_back(curr_enable_literal);

				vector<int> current_alarm_clause = alarm_literals;
				current_alarm_clause.push_back(curr_enable_literal);
				solver_->incAddClause(current_alarm_clause);
				//------------------------------------------------------------------------------------
				// assumptions saying that the states are equal
				vector<int> assumptions;
				assumptions.reserve(state_cnf_values.size() - num_err_latches_ + enable_literals.size());
				assumptions = enable_literals;
				assumptions.back() = -assumptions.back();

				for (unsigned state_idx = 0; state_idx < state_cnf_values.size() - num_err_latches_; ++state_idx)
				{
					if (concrete_state_ok[state_idx] == AIG_FALSE)
						assumptions.push_back(-state_cnf_values[state_idx]);
					else
						assumptions.push_back(state_cnf_values[state_idx]);
				}

				//------------------------------------------------------------------------------------

				// switch concrete simulation to next state
				concrete_state_ok = next_state_ok; // OR: change to sim_->switchToNextState();



				//------------------------------------------------------------------------------------
				// call SAT-solver
				vector<int> vars_of_interest = f;
				vars_of_interest.insert(vars_of_interest.end(), alarm_literals.begin(), alarm_literals.end());

				vector<int> model;
				bool sat = solver_->incIsSatModelOrCore(assumptions, vars_of_interest, model);

				while (sat)
				{
					cout << "sat" << endl;
					Utils::debugPrint(model,"model");
					SuperfluousTrace* sf = new SuperfluousTrace(testcase);
					sf->component_ = component_aig;
					int earliest_alarm_timestep = timestep;
					//parse:
					for (unsigned model_count = 0; model_count < model.size(); model_count++)
					{
						int lit = model[model_count];

						map<int, unsigned>::iterator it = fi_to_timestep.find(lit);
						if (lit > 0 && it != fi_to_timestep.end()) // we have found the fi variable which was set to TRUE
						{
							int fj = lit;
							solver_->incAddUnitClause(-fj);

							sf->flip_timestep_ = it->second;
						}
						else if (lit > 0)
						{
							it = alarmlit_to_timestep.find(lit);
							if (it == alarmlit_to_timestep.end())
							{
								MASSERT(false,"not an alarm lit. should not happen...")
							}

							if(it->second < earliest_alarm_timestep)
							{
								earliest_alarm_timestep = it->second;
							}
						}

					}
					sf->alarm_timestep_ = earliest_alarm_timestep;

					superfluous.push_back(sf);
					sat = solver_->incIsSatModelOrCore(assumptions, vars_of_interest, model);
				}



			} // -- END "for each timestep in testcase" --
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

	delete sim_concrete;

	return superfluous.size() != 0;
}

void FalsePositives::addSuperfluousTrace(int component, TestCase& testcase, unsigned flip_timestep,
		unsigned alarm_timestep, unsigned error_gone_ts)
{
	SuperfluousTrace* sf = new SuperfluousTrace(component, testcase, flip_timestep, alarm_timestep,
			error_gone_ts);
	superfluous.push_back(sf);
}

bool FalsePositives::isEqualN(vector<int> a, vector<int> b, int elements_to_skip)
{
	unsigned length = a.size() - elements_to_skip;

	return equal(a.begin(), a.begin() + length, b.begin());
}
