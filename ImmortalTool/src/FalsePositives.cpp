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
	SatSolver* solver_ = Options::instance().getSATSolver();
	AigSimulator* sim_ = new AigSimulator(circuit_);
	int next_free_cnf_var = 2;
	SymbolicSimulator symbsim(circuit_, solver_, next_free_cnf_var);
	vector<SuperfluousTrace*> superfluous; // TODO: use it, maybe declare as member
// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = component_aig >> 1;

		next_free_cnf_var = 2;



		for (unsigned tci = 0; tci < testcases.size(); tci++)
		{

			// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
			vector<int> concrete_state_ok;
			concrete_state_ok.resize(circuit_->num_latches);

			// f = a set of variables fi indicating whether the latch is flipped in step i or not
			vector<int> f;
			map<int, unsigned> fi_to_timestep;

			// a set of literals to enable or disable the represented output_is_different clauses,
			// necessary for incremental solving. At each sat-solver call only the newest clause
			// must be active:The newest enable-lit is always set to FALSE, while all other are TRUE
			vector<int> enable_literals;

			// start new incremental SAT-solving session
			vector<int> vars_to_keep;
			vars_to_keep.push_back(CNF_FALSE);
			solver_->startIncrementalSession(vars_to_keep, 0);
			solver_->incAddUnitClause(CNF_TRUE);

			symbsim.initLatches(); // initialize latches to false

			TestCase& testcase = testcases[tci];

			vector<int> alarm_outputs;

			for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
			{ // -------- BEGIN "for each timestep in testcase" ------------------------------------

				//------------------------------------------------------------------------------------
				// Concrete simulations:
				// correct simulation
				sim_->simulateOneTimeStep(testcase[timestep], concrete_state_ok);
				vector<int> outputs_ok = sim_->getOutputs();
				bool alarm_ok = outputs_ok.back() == AIG_TRUE;
				if (alarm_ok)
				{
					cout << "Alarm raised without Error!" << endl;
					return true;
				}
				vector<int> next_state_ok = sim_->getNextLatchValues();

				// faulty simulation: flip component bit
				vector<int> faulty_state = concrete_state_ok;
				faulty_state[c_cnt] = (faulty_state[c_cnt] == AIG_TRUE) ?
				AIG_FALSE :
																			AIG_TRUE;

				// faulty simulation with flipped bit
				sim_->simulateOneTimeStep(testcase[timestep], faulty_state);
				vector<int> outputs_faulty = sim_->getOutputs();
				vector<int> next_state_faulty = sim_->getNextLatchValues();
				bool alarm_faulty = outputs_faulty.back() == AIG_TRUE;

				symbsim.setInputValues(testcase[timestep]);

				if (outputs_ok == outputs_faulty && next_state_ok == next_state_faulty
						&& alarm_faulty)
				{
					SuperfluousTrace* sf = new SuperfluousTrace(component_aig, testcase, timestep, timestep, timestep + 1);
					superfluous.push_back(sf);
					symbsim.simulateOneTimeStep();
					alarm_outputs.push_back(symbsim.getAlarmValue());
					symbsim.switchToNextState();

					// switch concrete simulation to next state
					concrete_state_ok = next_state_ok; // OR: change to sim_->switchToNextState();
					continue;
				}

				if (outputs_ok == outputs_faulty && alarm_faulty)
				{
					// f variables:
					int fi = next_free_cnf_var++;
					solver_->addVarToKeep(fi);

					int old_value = symbsim.getResultValue(component_cnf);
					if (old_value == CNF_TRUE) // old value is true
						symbsim.setResultValue(component_cnf, -fi);
					else if (old_value == CNF_FALSE) // old value is false
						symbsim.setResultValue(component_cnf, fi);
					else
					{
						int new_value = next_free_cnf_var++;
						solver_->addVarToKeep(new_value);
						// new_value == fi ? -old_value : old_value
						solver_->incAdd3LitClause(fi, old_value, -new_value);
						solver_->incAdd3LitClause(fi, -old_value, new_value);
						solver_->incAdd3LitClause(-fi, old_value, new_value);
						solver_->incAdd3LitClause(-fi, -old_value, -new_value);
						symbsim.setResultValue(component_cnf, new_value);
					}

					// there might be at most one flip in one time-step:
					// if fi is true, all oter f must be false (fi -> -f1, fi -> -f2, ...)
					for (unsigned cnt = 0; cnt < f.size(); cnt++)
						solver_->incAdd2LitClause(-fi, -f[cnt]);

					f.push_back(fi);
					fi_to_timestep[fi] = timestep;

				}
				symbsim.simulateOneTimeStep();
				alarm_outputs.push_back(symbsim.getAlarmValue());

				const vector<int> &state_cnf_values = symbsim.getLatchValues();

				symbsim.switchToNextState();
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// clause saying that the outputs_ok o and o' are different
				vector<int> state_is_equal_clause;
				state_is_equal_clause.reserve(state_cnf_values.size() + 1);
				for (unsigned state_idx = 0; state_idx < state_cnf_values.size(); ++state_idx)
				{

					if (concrete_state_ok[state_idx] == AIG_FALSE)
						state_is_equal_clause.push_back(-state_cnf_values[state_idx]); // add false to outputs
					else
						state_is_equal_clause.push_back(state_cnf_values[state_idx]);
				}
				int curr_enable_literal = next_free_cnf_var++;
				state_is_equal_clause.push_back(curr_enable_literal);
				enable_literals.push_back(-curr_enable_literal);
				solver_->addVarToKeep(curr_enable_literal);
				solver_->incAddClause(state_is_equal_clause);

				vector<int> current_alarm_clause = alarm_outputs;
				current_alarm_clause.push_back(curr_enable_literal);
				solver_->incAddClause(current_alarm_clause);
				//------------------------------------------------------------------------------------

				// switch concrete simulation to next state
				concrete_state_ok = next_state_ok; // OR: change to sim_->switchToNextState();
				//------------------------------------------------------------------------------------
				// call SAT-solver

				vector<int> model;
				bool sat = solver_->incIsSatModelOrCore(enable_literals, f, model);

				while (sat)
				{
					// TODO: parse model;
					int component = 0; // TODO
					unsigned flip_timestep = 0; //fi_to_timestep
					unsigned fj = 0;
					unsigned alarm_timestep = 0;

					// TODO: replace by a function
					SuperfluousTrace* sf = new SuperfluousTrace(component, testcase, flip_timestep, alarm_timestep, timestep);
					superfluous.push_back(sf);

					solver_->incAddUnitClause(-fj);
					break;
				}

				enable_literals.back() = -enable_literals.back();

			} // -- END "for each timestep in testcase" --
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

	delete sim_;

	return false;
}
