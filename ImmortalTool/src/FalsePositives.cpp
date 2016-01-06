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
#include "Logger.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
FalsePositives::FalsePositives(aiger* circuit, int num_err_latches, int mode) :
				BackEnd(circuit, num_err_latches, mode)
{
	circuit_ = circuit;
	num_err_latches_ = num_err_latches;
	mode_ = mode;

}

// -------------------------------------------------------------------------------------------
FalsePositives::~FalsePositives()
{
}

bool FalsePositives::analyze(vector<TestCase>& testcases)
{
	//	vulnerable_latches = empty_set/list
	superfluous.clear();

	if (mode_ == 0)	// TODO: create modes, split into meaningful BackEnd groups
		findFalsePositives_1b(testcases);
	else if (mode_ == 1)
		findFalsePositives_2b(testcases);
	else
		MASSERT(false, "unknown mode!");

	return (superfluous.size() != 0);
}

bool FalsePositives::findFalsePositives_1b(vector<TestCase>& testcases)
{
	superfluous.clear();
	int next_free_cnf_var = 2;

	SatSolver* solver_ = Options::instance().getSATSolver();
	AigSimulator* sim_concrete = new AigSimulator(circuit_);
	SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);

	// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{

		next_free_cnf_var = 2;

		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = component_aig >> 1;

		cout << "==============================" << endl << "latch " << component_aig << endl;

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
//				Utils::debugPrint(testcase[timestep], "Test inputs");
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

				//------------------------------------------------------------------------------------
				// unit clauses saying that the current (TODO: "RELEVANT") outputs are equal
				const vector<int> &out_cnf_values = sim_symb.getOutputValues();
				for (unsigned output_idx = 0;
						output_idx < outputs_ok.size() - 1;	++output_idx) {
					if (outputs_ok[output_idx] == AIG_FALSE)
						solver_->incAddUnitClause(-out_cnf_values[output_idx]);
					else
						solver_->incAddUnitClause(out_cnf_values[output_idx]);

				}
				//------------------------------------------------------------------------------------

                sim_symb.switchToNextState();
				const vector<int> &next_cnf_values = sim_symb.getLatchValues();


				//------------------------------------------------------------------------------------
				int curr_enable_literal = next_free_cnf_var++;
				solver_->addVarToKeep(curr_enable_literal);
				enable_literals.push_back(curr_enable_literal);

				vector<int> current_alarm_clause = alarm_literals;
				current_alarm_clause.push_back(curr_enable_literal);
				solver_->incAddClause(current_alarm_clause);
				//---------------------------------------------timeste---------------------------------------
				// assumptions saying that the states are equal
				vector<int> assumptions;
				assumptions.reserve(next_cnf_values.size() - num_err_latches_ + enable_literals.size());
				assumptions = enable_literals;
				assumptions.back() = -assumptions.back();

				for (unsigned state_idx = 0;
						state_idx < next_cnf_values.size() - num_err_latches_;
						++state_idx) {
					if (next_state_ok[state_idx] == AIG_FALSE)
						assumptions.push_back(-next_cnf_values[state_idx]);
					else
						assumptions.push_back(next_cnf_values[state_idx]);

				}
//				Utils::debugPrint(assumptions, "Assumptions: ");

				//------------------------------------------------------------------------------------

				// switch concrete simulation to next state
				concrete_state_ok = next_state_ok; // OR: change to sim_->switchToNextState();

				//------------------------------------------------------------------------------------
				// call SAT-solver
				vector<int> vars_of_interest = f;
				vars_of_interest.insert(vars_of_interest.end(), alarm_literals.begin(), alarm_literals.end());

				vector<int> model;
				while (solver_->incIsSatModelOrCore(assumptions, vars_of_interest, model))
				{
					Utils::debugPrint(model,"model");
					SuperfluousTrace* sf = new SuperfluousTrace(testcase);
					sf->component_ = component_aig;
					sf->error_gone_timestep_ = timestep + 1;
					unsigned earliest_alarm_timestep = timestep + 1;
					//parse:
					int fj = 0;
					for (unsigned model_count = 0; model_count < model.size(); model_count++)
					{
						int lit = model[model_count];

						if (lit < 0)
							continue;

						// parse f time step
						map<int, unsigned>::iterator it = fi_to_timestep.find(lit);
						if (lit > 0 && it != fi_to_timestep.end()) // we have found the fi variable which was set to TRUE
						{
							fj = lit;
							solver_->incAddUnitClause(-fj);

							sf->flip_timestep_ = it->second;
							continue;
						}

						// parse earliest alarm time step
						it = alarmlit_to_timestep.find(lit);
						if(it != alarmlit_to_timestep.end() && it->second < earliest_alarm_timestep)
						{
							earliest_alarm_timestep = it->second;
						}

					}

					if(earliest_alarm_timestep != timestep + 1)
						sf->alarm_timestep_ = earliest_alarm_timestep;
					else
					{
						//L_DBG("same literal for f and alarm..")
						sf->alarm_timestep_ = alarmlit_to_timestep[fj];
					}
					L_DBG("[sat]  flip_timestep=" << sf->flip_timestep_ << ", alarm_timestep=" << sf->alarm_timestep_ << ",error_gone_ts=" << timestep+1)
					superfluous.push_back(sf);
				}



			} // -- END "for each timestep in testcase" --
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

	delete sim_concrete;

	return superfluous.size() != 0;
}

bool FalsePositives::findFalsePositives_2b(vector<TestCase>& testcases)
{
	superfluous.clear();

	int next_free_cnf_var = 2;

	SatSolver* solver_ = Options::instance().getSATSolver();
	AigSimulator* sim_concrete = new AigSimulator(circuit_);
	SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);


	// TODO: move outside of this function ----
	set<int> latches_to_check_; // TODO: always use all latches for the false positives algorithm?

	//------------------------------------------------------------------------------------------
	// set up ci signals
	// maps for latch-literals <=> cj-literals: each latch has a corresponding cj literal,
	// which indicates whether the latch is flipped or not.
	map<int, int> latch_to_cj; // maps latch-literals(cnf) to corresponding cj-literals(cnf)
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)

	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		latches_to_check_.insert(circuit_->latches[c_cnt].lit);
		int cj = next_free_cnf_var++;
		latch_to_cj[circuit_->latches[c_cnt].lit >> 1] = cj;
		cj_to_latch[cj] = circuit_->latches[c_cnt].lit;
	}
	int next_cnf_var_after_ci_vars = next_free_cnf_var;
	//------------------------------------------------------------------------------------------


	// for each testcase
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{
		TestCase& testcase = testcases[tc_number];

		// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
		vector<int> concrete_state_ok;
		concrete_state_ok.resize(circuit_->num_latches);
		sim_symb.initLatches();

		// f = a set of variables fi indicating whether the latch is *flipped in _step_ i* or not
		vector<int> f;
		map<int, unsigned> fi_to_timestep;

		// set of literals representing the alarm output for each timestep
		vector<int> alarm_literals;
		map<int, unsigned> alarmlit_to_timestep;

		// a set of cj-literals indicating whether *the _latch_ C_j is flipped* or not
		vector<int> cj_literals;
		map<int, int>::iterator map_iter;
		for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++)
		{
			cj_literals.push_back(map_iter->second);
		}

		// start new incremental SAT-solving session
		solver_->startIncrementalSession(cj_literals, 0);
		solver_->addVarToKeep(abs(CNF_TRUE));
		solver_->incAddUnitClause(CNF_TRUE); // CNF_TRUE= unit-clause representing TRUE constant

		// set of literals to enable or disable the represented clauses, used for incremental solving.
		vector<int> enable_literals;

		//----------------------------------------------------------------------------------------
		// single fault assumption: there might be at most one flipped component
		map<int, int>::iterator map_iter2;
		for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++)
		{
			map_iter2 = map_iter;
			map_iter2++;
			for (; map_iter2 != latch_to_cj.end(); map_iter2++)
				solver_->incAdd2LitClause(-map_iter->second, -map_iter2->second);
		}





		// if environment-model: define which output is relevant at which point in time:
//		AigSimulator* environment_sim = 0;
//		if (environment_model_)
//			environment_sim = new AigSimulator(environment_model_);

		for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_concrete->simulateOneTimeStep(testcase[timestep], concrete_state_ok);
			vector<int> outputs_ok = sim_concrete->getOutputs();
			vector<int> next_state_ok = sim_concrete->getNextLatchValues();
			bool alarm_ok = outputs_ok.back() == AIG_TRUE;
			if (alarm_ok)
			{
				cout << "Alarm raised without Error!" << endl;
				return true;
			}

			//--------------------------------------------------------------------------------------

			// set input values according to TestCase to TRUE or FALSE:
			sim_symb.setInputValues(testcase[timestep]);

			//--------------------------------------------------------------------------------------
			// cj is a variable that indicatest whether the corresponding latch is flipped
			// fi is a variable that indicates whether the component is flipped in step i or not
			// there can only be a flip at timestep i if both cj and fi are true.
			int fi = next_free_cnf_var++;
			solver_->addVarToKeep(fi);

			set<int>::iterator it;
			for (it = latches_to_check_.begin(); it != latches_to_check_.end(); ++it) // TODO: for ALL latches
			{
				int latch_output = *it >> 1;
				int old_value = sim_symb.getResultValue(latch_output);
				int new_value = next_free_cnf_var++;
				int ci_lit = latch_to_cj[latch_output];

				//solver_->addVarToKeep(new_value);

				solver_->incAdd3LitClause(old_value, ci_lit, -new_value);
				solver_->incAdd3LitClause(old_value, fi, -new_value);
				solver_->incAdd4LitClause(-old_value, -ci_lit, -fi, -new_value);
				solver_->incAdd3LitClause(-old_value, ci_lit, new_value);
				solver_->incAdd3LitClause(-old_value, fi, new_value);
				solver_->incAdd4LitClause(old_value, -ci_lit, -fi, new_value);

				sim_symb.setResultValue(latch_output, new_value);
			}

			// there might be at most one flip in one time-step
			// if fi is true, all other f must be false (fi -> -f1, fi -> -f2, ...)
			for (unsigned cnt = 0; cnt < f.size(); cnt++)
				solver_->incAdd2LitClause(-fi, -f[cnt]);

			f.push_back(fi);
			fi_to_timestep[fi] = timestep;


			sim_symb.simulateOneTimeStep();
			alarm_literals.push_back(sim_symb.getAlarmValue());
			alarmlit_to_timestep[sim_symb.getAlarmValue()] = timestep;


			//------------------------------------------------------------------------------------
			// unit clauses saying that the current (TODO: "RELEVANT") outputs are equal
			const vector<int> &out_cnf_values = sim_symb.getOutputValues();
			for (unsigned output_idx = 0;
					output_idx < outputs_ok.size() - 1;	++output_idx) {
				if (outputs_ok[output_idx] == AIG_FALSE)
					solver_->incAddUnitClause(-out_cnf_values[output_idx]);
				else
					solver_->incAddUnitClause(out_cnf_values[output_idx]);

			}
			//--------------------------------------------------------------------------------------

			// get Outputs and next state values, switch to next state
			sim_symb.switchToNextState();
			const vector<int> &next_cnf_values = sim_symb.getLatchValues();


			int curr_enable_literal = next_free_cnf_var++;
			solver_->addVarToKeep(curr_enable_literal);
			enable_literals.push_back(curr_enable_literal);

			vector<int> current_alarm_clause = alarm_literals;
			current_alarm_clause.push_back(curr_enable_literal);
			solver_->incAddClause(current_alarm_clause);

			// assumptions saying that the states are equal
			vector<int> assumptions;
			assumptions.reserve(next_cnf_values.size() - num_err_latches_ + enable_literals.size());
			assumptions = enable_literals;
			assumptions.back() = -assumptions.back();

			for (unsigned state_idx = 0;
					state_idx < next_cnf_values.size() - num_err_latches_;
					++state_idx) {
				if (next_state_ok[state_idx] == AIG_FALSE)
					assumptions.push_back(-next_cnf_values[state_idx]);
				else
					assumptions.push_back(next_cnf_values[state_idx]);

			}

			// switch concrete simulation to next state
			concrete_state_ok = next_state_ok;

//			vector<int> output_is_relevant;
//			if(environment_model_)
//			{
//				vector<int> env_input;
//				env_input.reserve(testcase[timestep].size() + outputs_ok.size());
//				env_input.insert(env_input.end(),
//						testcase[timestep].begin(), testcase[timestep].end());
//				env_input.insert(env_input.end(), outputs_ok.begin(),
//						outputs_ok.end());
//				environment_sim->simulateOneTimeStep(env_input);
//				output_is_relevant = environment_sim->getOutputs();
//				environment_sim->switchToNextState();
//			}

			//--------------------------------------------------------------------------------------
			// call SAT-solver

			vector<int> vars_of_interest = f;
			vars_of_interest.insert(vars_of_interest.end(), alarm_literals.begin(), alarm_literals.end());
			vars_of_interest.insert(vars_of_interest.end(), cj_literals.begin(), cj_literals.end());


			vector<int> model;
			while (solver_->incIsSatModelOrCore(assumptions, vars_of_interest, model))
			{
				Utils::debugPrint(model, "model");



				unsigned earliest_alarm_timestep = timestep + 1;
				int fi = CNF_TRUE;
				int cj = CNF_TRUE;

				for (unsigned model_count = 0; model_count < model.size(); model_count++)
				{
					int lit = model[model_count];


					if (lit > 0 && lit < next_cnf_var_after_ci_vars) // we have found the one and only active cj signal
					{
						cj = lit;
						continue;
					}

					// parse f time step
					map<int, unsigned>::iterator it = fi_to_timestep.find(lit);
					if (lit > 0 && it != fi_to_timestep.end()) // we have found the fi variable which was set to TRUE
					{
						fi = lit;
						continue;
					}

					// parse earliest alarm time step
					it = alarmlit_to_timestep.find(lit);
					if(it != alarmlit_to_timestep.end() && it->second < earliest_alarm_timestep)
					{
						earliest_alarm_timestep = it->second;
					}
				}

				// blocking clause: ignore flips at this particular time step for this particular latch in the future
				solver_->incAdd2LitClause(-fi, -cj);

				SuperfluousTrace* sf = new SuperfluousTrace(testcase);
				sf->error_gone_timestep_ = timestep +1;
				sf->component_ = cj_to_latch[cj];
				sf->flip_timestep_ = fi_to_timestep[fi];
				if(earliest_alarm_timestep != timestep + 1)
					sf->alarm_timestep_ = earliest_alarm_timestep;
				else
					sf->alarm_timestep_ = alarmlit_to_timestep[fi];
				superfluous.push_back(sf);
				L_DBG(sf->toString())
			}



		} // -- END "for each timestep in testcase" --


//		if (environment_model_)
//			delete environment_sim;
	} // ------ END 'for each testcase' ---------------


	delete sim_concrete;
	delete solver_;

	return superfluous.size() != 0;
}

bool FalsePositives::findFalsePositives_1b_free_inputs(vector<TestCase>& testcases)
{
	superfluous.clear();
	int next_free_cnf_var = 2;

	SatSolver* solver_ = Options::instance().getSATSolver();
	SymbolicSimulator sim_ok(circuit_,solver_, next_free_cnf_var);
	SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);

	// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{

		next_free_cnf_var = 2;

		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = component_aig >> 1;

		cout << "==============================" << endl << "latch " << component_aig << endl;

		for (unsigned tci = 0; tci < testcases.size(); tci++)
		{
			TestCase& testcase = testcases[tci];
			TestCase testcase_with_cnf_literals;

			sim_symb.initLatches(); // initialize latches to false
			sim_ok.initLatches();

			AndCacheMap cache(solver_);
			sim_ok.setCache(&cache);
			sim_symb.setCache(&cache);

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
//				Utils::debugPrint(testcase[timestep], "Test inputs");
				//------------------------------------------------------------------------------------
				// Concrete simulations:
				// correct simulation
				sim_ok.simulateOneTimeStep(testcase[timestep]);
				vector<int> outputs_ok = sim_ok.getOutputValues();
				bool alarm_ok = outputs_ok.back() == CNF_TRUE;
				//Utils::debugPrint(outputs_ok,"outputs_ok");
				if (alarm_ok)
				{
					cout << "Alarm raised without Error!" << endl;
					return true;
				}
				vector<int> next_state_ok = sim_ok.getNextLatchValues();

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


				testcase_with_cnf_literals.push_back(sim_ok.getInputValues());
				sim_symb.setCnfInputValues(sim_ok.getInputValues());
				sim_symb.simulateOneTimeStep();
				alarm_literals.push_back(sim_symb.getAlarmValue());
				alarmlit_to_timestep[sim_symb.getAlarmValue()] = timestep;

				//------------------------------------------------------------------------------------
				// unit clauses saying that the current (TODO: "RELEVANT") outputs are equal
				const vector<int> &out_cnf_values = sim_symb.getOutputValues();
				for (unsigned output_idx = 0;
						output_idx < outputs_ok.size() - 1;	++output_idx) {
					if (outputs_ok[output_idx] == CNF_FALSE)
						solver_->incAddUnitClause(-out_cnf_values[output_idx]);
					else if (outputs_ok[output_idx] == CNF_TRUE)
						solver_->incAddUnitClause(out_cnf_values[output_idx]);
					else {
						solver_->incAdd2LitClause(-outputs_ok[output_idx], out_cnf_values[output_idx]);
						solver_->incAdd2LitClause(outputs_ok[output_idx], -out_cnf_values[output_idx]);
					}

				}
				//--------------------------------------------------------------------------------------

                sim_symb.switchToNextState();
				const vector<int> &next_cnf_values = sim_symb.getLatchValues();


				//------------------------------------------------------------------------------------
				int curr_enable_literal = next_free_cnf_var++;
				solver_->addVarToKeep(curr_enable_literal);
				enable_literals.push_back(curr_enable_literal);

				vector<int> current_alarm_clause = alarm_literals;
				current_alarm_clause.push_back(curr_enable_literal);
				solver_->incAddClause(current_alarm_clause);
				//---------------------------------------------timeste---------------------------------------
				// assumptions saying that the states are equal
				vector<int> assumptions;
				assumptions.reserve(next_cnf_values.size() - num_err_latches_ + enable_literals.size());
				assumptions = enable_literals;
				assumptions.back() = -assumptions.back();

				for (unsigned state_idx = 0;
						state_idx < next_cnf_values.size() - num_err_latches_;
						++state_idx) {
					int ok_lit = next_state_ok[state_idx];
					int symb_lit = next_cnf_values[state_idx];

					if (ok_lit == CNF_FALSE)
						assumptions.push_back(-symb_lit);
					else if (ok_lit == CNF_TRUE)
						assumptions.push_back(symb_lit);
					else
					{
						// TODO check
						solver_->incAdd3LitClause(curr_enable_literal, -ok_lit, symb_lit);
						solver_->incAdd3LitClause(curr_enable_literal, ok_lit, -symb_lit);
					}

				}
//				Utils::debugPrint(assumptions, "Assumptions: ");

				//------------------------------------------------------------------------------------

				// switch concrete simulation to next state
				sim_ok.switchToNextState();
				//------------------------------------------------------------------------------------
				// call SAT-solver
				vector<int> vars_of_interest = f;
				vars_of_interest.insert(vars_of_interest.end(), alarm_literals.begin(), alarm_literals.end());
				const vector<int> &open_in = sim_ok.getOpenInputVars();
				vars_of_interest.insert(vars_of_interest.end(), open_in.begin(), open_in.end());

				vector<int> model;
				while (solver_->incIsSatModelOrCore(assumptions, vars_of_interest, model))
				{

					map<int, unsigned> cnf_input_var_to_aig_truth_lit;
					cnf_input_var_to_aig_truth_lit[CNF_TRUE] = AIG_TRUE;
					cnf_input_var_to_aig_truth_lit[CNF_FALSE] = AIG_FALSE;

					Utils::debugPrint(model,"model");

					SuperfluousTrace* sf = new SuperfluousTrace();
					sf->testcase_.reserve(testcase_with_cnf_literals.size());

					sf->component_ = component_aig;
					sf->error_gone_timestep_ = timestep + 1;
					unsigned earliest_alarm_timestep = timestep + 1;
					//parse:
					int fj = 0;
					for (unsigned model_count = 0; model_count < model.size(); model_count++)
					{
						int lit = model[model_count];
						cnf_input_var_to_aig_truth_lit[lit] = (lit > 0) ? AIG_TRUE : AIG_FALSE;

						if (lit < 0)
							continue;

						// parse f time step
						map<int, unsigned>::iterator it = fi_to_timestep.find(lit);
						if (lit > 0 && it != fi_to_timestep.end()) // we have found the fi variable which was set to TRUE
						{
							fj = lit;
							solver_->incAddUnitClause(-fj);

							sf->flip_timestep_ = it->second;
							continue;
						}

						// parse earliest alarm time step
						it = alarmlit_to_timestep.find(lit);
						if(it != alarmlit_to_timestep.end() && it->second < earliest_alarm_timestep)
						{
							earliest_alarm_timestep = it->second;
						}

					}

					if(earliest_alarm_timestep != timestep + 1)
						sf->alarm_timestep_ = earliest_alarm_timestep;
					else
					{
						//L_DBG("same literal for f and alarm..")
						sf->alarm_timestep_ = alarmlit_to_timestep[fj];
					}
					L_DBG("[sat]  flip_timestep=" << sf->flip_timestep_ << ", alarm_timestep=" << sf->alarm_timestep_ << ",error_gone_ts=" << timestep+1)


					for (TestCase::const_iterator in = testcase_with_cnf_literals.begin();
							in != testcase_with_cnf_literals.end(); ++in)
					{
						vector<int> concrete_input_vector;
						concrete_input_vector.reserve(in->size());
						for (vector<int>::const_iterator iv = in->begin(); iv != in->end();
								++iv)
						{
							concrete_input_vector.push_back(cnf_input_var_to_aig_truth_lit[*iv]);
						}
						sf->testcase_.push_back(concrete_input_vector);
					}

					superfluous.push_back(sf);
				}



			} // -- END "for each timestep in testcase" --
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------


	return superfluous.size() != 0;
}

bool FalsePositives::findFalsePositives_2b_free_inputs(vector<TestCase>& testcases)
{
	superfluous.clear();
	int next_free_cnf_var = 2;

	SatSolver* solver_ = Options::instance().getSATSolver();
	SymbolicSimulator sim_ok(circuit_,solver_, next_free_cnf_var);
	SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);


	// TODO: move outside of this function ----
	set<int> latches_to_check_; // TODO: always use all latches for the false positives algorithm?

	//------------------------------------------------------------------------------------------
	// set up ci signals
	// maps for latch-literals <=> cj-literals: each latch has a corresponding cj literal,
	// which indicates whether the latch is flipped or not.
	map<int, int> latch_to_cj; // maps latch-literals(cnf) to corresponding cj-literals(cnf)
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)

	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		latches_to_check_.insert(circuit_->latches[c_cnt].lit);
		int cj = next_free_cnf_var++;
		latch_to_cj[circuit_->latches[c_cnt].lit >> 1] = cj;
		cj_to_latch[cj] = circuit_->latches[c_cnt].lit;
	}
	int next_cnf_var_after_ci_vars = next_free_cnf_var;
	//------------------------------------------------------------------------------------------


	// for each testcase
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{
		TestCase& testcase = testcases[tc_number];
		TestCase testcase_with_cnf_literals;

		sim_ok.initLatches();
		sim_symb.initLatches();

		AndCacheMap cache(solver_);
		sim_ok.setCache(&cache);
		sim_symb.setCache(&cache);

		// f = a set of variables fi indicating whether the latch is *flipped in _step_ i* or not
		vector<int> f;
		map<int, unsigned> fi_to_timestep;

		// set of literals representing the alarm output for each timestep
		vector<int> alarm_literals;
		map<int, unsigned> alarmlit_to_timestep;

		// a set of cj-literals indicating whether *the _latch_ C_j is flipped* or not
		vector<int> cj_literals;
		map<int, int>::iterator map_iter;
		for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++)
		{
			cj_literals.push_back(map_iter->second);
		}

		// start new incremental SAT-solving session
		solver_->startIncrementalSession(cj_literals, 0);
		solver_->addVarToKeep(abs(CNF_TRUE));
		solver_->incAddUnitClause(CNF_TRUE); // CNF_TRUE= unit-clause representing TRUE constant

		// set of literals to enable or disable the represented clauses, used for incremental solving.
		vector<int> enable_literals;

		//----------------------------------------------------------------------------------------
		// single fault assumption: there might be at most one flipped component
		map<int, int>::iterator map_iter2;
		for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++)
		{
			map_iter2 = map_iter;
			map_iter2++;
			for (; map_iter2 != latch_to_cj.end(); map_iter2++)
				solver_->incAdd2LitClause(-map_iter->second, -map_iter2->second);
		}


		// if environment-model: define which output is relevant at which point in time:
//		AigSimulator* environment_sim = 0;
//		if (environment_model_)
//			environment_sim = new AigSimulator(environment_model_);

		for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_ok.simulateOneTimeStep(testcase[timestep]);
			vector<int> outputs_ok = sim_ok.getOutputValues();
			vector<int> next_state_ok = sim_ok.getNextLatchValues();
			bool alarm_ok = outputs_ok.back() == CNF_TRUE;
			if (alarm_ok)
			{
				cout << "Alarm raised without Error!" << endl;
				return true;
			}

			//--------------------------------------------------------------------------------------

			// set input values according to TestCase to TRUE or FALSE:
			sim_symb.setInputValues(testcase[timestep]);

			//--------------------------------------------------------------------------------------
			// cj is a variable that indicatest whether the corresponding latch is flipped
			// fi is a variable that indicates whether the component is flipped in step i or not
			// there can only be a flip at timestep i if both cj and fi are true.
			int fi = next_free_cnf_var++;
			solver_->addVarToKeep(fi);

			set<int>::iterator it;
			for (it = latches_to_check_.begin(); it != latches_to_check_.end(); ++it) // TODO: for ALL latches
			{
				int latch_output = *it >> 1;
				int old_value = sim_symb.getResultValue(latch_output);
				int new_value = next_free_cnf_var++;
				int ci_lit = latch_to_cj[latch_output];

				//solver_->addVarToKeep(new_value);

				solver_->incAdd3LitClause(old_value, ci_lit, -new_value);
				solver_->incAdd3LitClause(old_value, fi, -new_value);
				solver_->incAdd4LitClause(-old_value, -ci_lit, -fi, -new_value);
				solver_->incAdd3LitClause(-old_value, ci_lit, new_value);
				solver_->incAdd3LitClause(-old_value, fi, new_value);
				solver_->incAdd4LitClause(old_value, -ci_lit, -fi, new_value);

				sim_symb.setResultValue(latch_output, new_value);
			}

			// there might be at most one flip in one time-step
			// if fi is true, all other f must be false (fi -> -f1, fi -> -f2, ...)
			for (unsigned cnt = 0; cnt < f.size(); cnt++)
				solver_->incAdd2LitClause(-fi, -f[cnt]);

			f.push_back(fi);
			fi_to_timestep[fi] = timestep;
			//--------------------------------------------------------------------------------------

			testcase_with_cnf_literals.push_back(sim_ok.getInputValues());
			sim_symb.setCnfInputValues(sim_ok.getInputValues());
			sim_symb.simulateOneTimeStep();
			alarm_literals.push_back(sim_symb.getAlarmValue());
			alarmlit_to_timestep[sim_symb.getAlarmValue()] = timestep;

			//------------------------------------------------------------------------------------
			// unit clauses saying that the current (TODO: "RELEVANT") outputs are equal
			const vector<int> &out_cnf_values = sim_symb.getOutputValues();
			for (unsigned output_idx = 0;
					output_idx < outputs_ok.size() - 1;	++output_idx) {
				if (outputs_ok[output_idx] == CNF_FALSE)
					solver_->incAddUnitClause(-out_cnf_values[output_idx]);
				else if (outputs_ok[output_idx] == CNF_TRUE)
					solver_->incAddUnitClause(out_cnf_values[output_idx]);
				else {
					solver_->incAdd2LitClause(-outputs_ok[output_idx], out_cnf_values[output_idx]);
					solver_->incAdd2LitClause(outputs_ok[output_idx], -out_cnf_values[output_idx]);
				}

			}
			//--------------------------------------------------------------------------------------


			sim_symb.switchToNextState();
			const vector<int> &next_cnf_values = sim_symb.getLatchValues();


			int curr_enable_literal = next_free_cnf_var++;
			solver_->addVarToKeep(curr_enable_literal);
			enable_literals.push_back(curr_enable_literal);

			vector<int> current_alarm_clause = alarm_literals;
			current_alarm_clause.push_back(curr_enable_literal);
			solver_->incAddClause(current_alarm_clause);

			// assumptions saying that the states are equal
			vector<int> assumptions;
			assumptions.reserve(next_cnf_values.size() - num_err_latches_ + enable_literals.size());
			assumptions = enable_literals;
			assumptions.back() = -assumptions.back();

			for (unsigned state_idx = 0;
					state_idx < next_cnf_values.size() - num_err_latches_;
					++state_idx) {
				int ok_lit = next_state_ok[state_idx];
				int symb_lit = next_cnf_values[state_idx];

				if (ok_lit == CNF_FALSE)
					assumptions.push_back(-symb_lit);
				else if (ok_lit == CNF_TRUE)
					assumptions.push_back(symb_lit);
				else
				{
					// TODO check
					solver_->incAdd3LitClause(curr_enable_literal, -ok_lit, symb_lit);
					solver_->incAdd3LitClause(curr_enable_literal, ok_lit, -symb_lit);
				}

			}

			// switch concrete simulation to next state
			sim_ok.switchToNextState();

//			vector<int> output_is_relevant;
//			if(environment_model_)
//			{
//				vector<int> env_input;
//				env_input.reserve(testcase[timestep].size() + outputs_ok.size());
//				env_input.insert(env_input.end(),
//						testcase[timestep].begin(), testcase[timestep].end());
//				env_input.insert(env_input.end(), outputs_ok.begin(),
//						outputs_ok.end());
//				environment_sim->simulateOneTimeStep(env_input);
//				output_is_relevant = environment_sim->getOutputs();
//				environment_sim->switchToNextState();
//			}

			//--------------------------------------------------------------------------------------
			// call SAT-solver

			vector<int> vars_of_interest = f;
			vars_of_interest.insert(vars_of_interest.end(), alarm_literals.begin(), alarm_literals.end());
			vars_of_interest.insert(vars_of_interest.end(), cj_literals.begin(), cj_literals.end());
			const vector<int> &open_in = sim_ok.getOpenInputVars();
			vars_of_interest.insert(vars_of_interest.end(), open_in.begin(), open_in.end());

			vector<int> model;
			while (solver_->incIsSatModelOrCore(assumptions, vars_of_interest, model))
			{
				Utils::debugPrint(model, "model");

				map<int, unsigned> cnf_input_var_to_aig_truth_lit;
				cnf_input_var_to_aig_truth_lit[CNF_TRUE] = AIG_TRUE;
				cnf_input_var_to_aig_truth_lit[CNF_FALSE] = AIG_FALSE;

				unsigned earliest_alarm_timestep = timestep + 1;
				int fi = CNF_TRUE;
				int cj = CNF_TRUE;

				for (unsigned model_count = 0; model_count < model.size(); model_count++)
				{
					int lit = model[model_count];
					cnf_input_var_to_aig_truth_lit[lit] = (lit > 0) ? AIG_TRUE : AIG_FALSE;

					if (lit > 0 && lit < next_cnf_var_after_ci_vars) // we have found the one and only active cj signal
					{
						cj = lit;
						continue;
					}

					// parse f time step
					map<int, unsigned>::iterator it = fi_to_timestep.find(lit);
					if (lit > 0 && it != fi_to_timestep.end()) // we have found the fi variable which was set to TRUE
					{
						fi = lit;
						continue;
					}

					// parse earliest alarm time step
					it = alarmlit_to_timestep.find(lit);
					if(it != alarmlit_to_timestep.end() && it->second < earliest_alarm_timestep)
					{
						earliest_alarm_timestep = it->second;
					}
				}

				// blocking clause: ignore flips at this particular time step for this particular latch in the future
				solver_->incAdd2LitClause(-fi, -cj);

				SuperfluousTrace* sf = new SuperfluousTrace();
				sf->error_gone_timestep_ = timestep +1;
				sf->component_ = cj_to_latch[cj];
				sf->flip_timestep_ = fi_to_timestep[fi];
				if(earliest_alarm_timestep != timestep + 1)
					sf->alarm_timestep_ = earliest_alarm_timestep;
				else
					sf->alarm_timestep_ = alarmlit_to_timestep[fi];

				for (TestCase::const_iterator in = testcase_with_cnf_literals.begin();
						in != testcase_with_cnf_literals.end(); ++in)
				{
					vector<int> concrete_input_vector;
					concrete_input_vector.reserve(in->size());
					for (vector<int>::const_iterator iv = in->begin(); iv != in->end();
							++iv)
					{
						concrete_input_vector.push_back(cnf_input_var_to_aig_truth_lit[*iv]);
					}
					sf->testcase_.push_back(concrete_input_vector);
				}
				superfluous.push_back(sf);
				L_DBG(sf->toString())
			}



		} // -- END "for each timestep in testcase" --


//		if (environment_model_)
//			delete environment_sim;
	} // ------ END 'for each testcase' ---------------


	delete solver_;

	return superfluous.size() != 0;
}

bool FalsePositives::isEqualN(vector<int> a, vector<int> b, int elements_to_skip)
{
	unsigned length = a.size() - elements_to_skip;

	return equal(a.begin(), a.begin() + length, b.begin());
}

void FalsePositives::addSuperfluousTrace(int component, TestCase& testcase, unsigned flip_timestep,
		unsigned alarm_timestep, unsigned error_gone_ts)
{
	L_DBG("       flip_timestep=" << flip_timestep << ", alarm_timestep=" << alarm_timestep << ",error_gone_ts=" << error_gone_ts)
	SuperfluousTrace* sf = new SuperfluousTrace(component, testcase, flip_timestep, alarm_timestep,
			error_gone_ts);
	superfluous.push_back(sf);
}
