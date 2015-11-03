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
/// @file SymbTimeLocationAnalysis.cpp
/// @brief Contains the definition of the class SymbTimeLocationAnalysis.
// -------------------------------------------------------------------------------------------

#include "SymbTimeLocationAnalysis.h"
#include "Utils.h"
#include "AIG2CNF.h"
#include "Options.h"
#include "Logger.h"
#include "SymbolicSimulator.h"
#include "AndCacheMap.h"
#include "AndCacheFor2Simulators.h"
#include "ErrorTraceManager.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SymbTimeLocationAnalysis::SymbTimeLocationAnalysis(aiger* circuit, int num_err_latches,
		int mode) :
		BackEnd(circuit, num_err_latches, mode), sim_(0)
{
	AIG2CNF::instance().initFromAig(circuit);
	solver_ = Options::instance().getSATSolver();
	unsat_core_interval_ = Options::instance().getUnsatCoreInterval();
}

// -------------------------------------------------------------------------------------------
SymbTimeLocationAnalysis::~SymbTimeLocationAnalysis()
{
	delete solver_;
}

// -------------------------------------------------------------------------------------------
bool SymbTimeLocationAnalysis::findVulnerabilities(vector<TestCase> &testcases)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();

	if (mode_ == STANDARD)
		Analyze2(testcases);
	else if (mode_ == FREE_INPUTS)
		Analyze2_free_inputs(testcases);
	else
		MASSERT(false, "unknown mode!");

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
bool SymbTimeLocationAnalysis::findVulnerabilities(vector<string> paths_to_TC_files)
{
	vector<TestCase> testcases;
	//for each test case t[][]
	for (unsigned tc_index_ = 0; tc_index_ < paths_to_TC_files.size(); tc_index_++)
	{
		TestCase testcase;
		Utils::parseAigSimFile(paths_to_TC_files[tc_index_], testcase, circuit_->num_inputs);
		testcases.push_back(testcase);
	}

	return findVulnerabilities(testcases);
}

// -------------------------------------------------------------------------------------------
void SymbTimeLocationAnalysis::Analyze2(vector<TestCase>& testcases)
{
	sim_ = new AigSimulator(circuit_);
	// used to store the results of the symbolic simulation
	int next_free_cnf_var = 2;

	// TODO: move outside of this function ----
	set<int> latches_to_check_;

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
	SymbolicSimulator symbsim(circuit_, solver_, next_free_cnf_var);

	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{

		// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
		vector<int> concrete_state;
		concrete_state.resize(circuit_->num_latches);

		// f = a set of variables fi indicating whether the latch is *flipped in _step_ i* or not
		vector<int> f;
		map<int, unsigned> fi_to_timestep;

		// a set of cj-literals indicating whether *the _latch_ C_j is flipped* or not
		vector<int> cj_literals;
		map<int, int>::iterator map_iter;
		for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++)
		{
			cj_literals.push_back(map_iter->second);
		}

		// a set of literals to enable or disable the represented output_is_different clauses,
		// necessary for incremental solving. At each sat-solver call only the newest clause
		// must be active:The newest enable-lit is always set to FALSE, while all other are TRUE
		vector<int> odiff_enable_literals;

		// start new incremental SAT-solving session
		solver_->startIncrementalSession(cj_literals, 0);
		solver_->addVarToKeep(abs(CNF_TRUE));
		solver_->incAddUnitClause(CNF_TRUE); // CNF_TRUE= unit-clause representing TRUE constant

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

		symbsim.initLatches();

		TestCase& testcase = testcases[tc_number];

		// if environment-model: define which output is relevant at which point in time:
		AigSimulator* environment_sim = 0;
		if (environment_model_)
			environment_sim = new AigSimulator(environment_model_);

		for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_->simulateOneTimeStep(testcase[timestep], concrete_state);
			vector<int> outputs_ok = sim_->getOutputs();
			vector<int> next_state = sim_->getNextLatchValues();

			// switch concrete simulation to next state
			concrete_state = next_state; // OR: change to sim_->switchToNextState();
			//--------------------------------------------------------------------------------------

			// set input values according to TestCase to TRUE or FALSE:
			symbsim.setInputValues(testcase[timestep]);

			//--------------------------------------------------------------------------------------
			// cj is a variable that indicatest whether the corresponding latch is flipped
			// fi is a variable that indicates whether the component is flipped in step i or not
			// there can only be a flip at timestep i if both cj and fi are true.
			int fi = next_free_cnf_var++;
			solver_->addVarToKeep(fi);

			set<int>::iterator it;
			for (it = latches_to_check_.begin(); it != latches_to_check_.end(); ++it)
			{
				int latch_output = *it >> 1;
				int old_value = symbsim.getResultValue(latch_output);
				int new_value = next_free_cnf_var++;
				int ci_lit = latch_to_cj[latch_output];

				//solver_->addVarToKeep(new_value);

				solver_->incAdd3LitClause(old_value, ci_lit, -new_value);
				solver_->incAdd3LitClause(old_value, fi, -new_value);
				solver_->incAdd4LitClause(-old_value, -ci_lit, -fi, -new_value);
				solver_->incAdd3LitClause(-old_value, ci_lit, new_value);
				solver_->incAdd3LitClause(-old_value, fi, new_value);
				solver_->incAdd4LitClause(old_value, -ci_lit, -fi, new_value);

				symbsim.setResultValue(latch_output, new_value);
			}

			//--------------------------------------------------------------------------------------
			// single fault assumption: there might be at most one flip in one time-step
			// if fi is true, all oter f must be false (fi -> -f1, fi -> -f2, ...)
			for (unsigned cnt = 0; cnt < f.size(); cnt++)
				solver_->incAdd2LitClause(-fi, -f[cnt]);

			f.push_back(fi);
			fi_to_timestep[fi] = timestep;
			//--------------------------------------------------------------------------------------

			// Symbolic simulation of AND gates
			symbsim.simulateOneTimeStep();
			// get Outputs and next state values, switch to next state
			solver_->incAddUnitClause(-symbsim.getAlarmValue());
			const vector<int> &out_cnf_values = symbsim.getOutputValues();
			symbsim.switchToNextState();
			const vector<int> &next_state_cnf_values = symbsim.getLatchValues(); // already next st
			//--------------------------------------------------------------------------------------

			vector<int> output_is_relevant;
			if(environment_model_)
			{
				vector<int> env_input;
				env_input.reserve(testcase[timestep].size() + outputs_ok.size());
				env_input.insert(env_input.end(),
						testcase[timestep].begin(), testcase[timestep].end());
				env_input.insert(env_input.end(), outputs_ok.begin(),
						outputs_ok.end());
				environment_sim->simulateOneTimeStep(env_input);
				output_is_relevant = environment_sim->getOutputs();
				environment_sim->switchToNextState();
			}

			//--------------------------------------------------------------------------------------
			// clause saying that the outputs_ok o and o' are different
			vector<int> o_is_diff_clause;
			o_is_diff_clause.reserve(out_cnf_values.size() + 1);
			for (unsigned out_idx = 0; out_idx < out_cnf_values.size(); ++out_idx)
			{
				// skip if output is not relevant
				if (environment_model_ && output_is_relevant[out_idx] == AIG_FALSE)
					continue;

				if (outputs_ok[out_idx] == AIG_TRUE) // simulation result of output is true
					o_is_diff_clause.push_back(-out_cnf_values[out_idx]); // add negated output
				else if (outputs_ok[out_idx] == AIG_FALSE)
					o_is_diff_clause.push_back(out_cnf_values[out_idx]);
			}
			int o_is_diff_enable_literal = next_free_cnf_var++;
			o_is_diff_clause.push_back(o_is_diff_enable_literal);
			odiff_enable_literals.push_back(-o_is_diff_enable_literal);
			solver_->addVarToKeep(o_is_diff_enable_literal);
			solver_->incAddClause(o_is_diff_clause);
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
			// call SAT-solver
			vector<int> model;
			ErrorTrace* trace = 0;
			bool useDiagnostic = Options::instance().isUseDiagnosticOutput();

			vector<int> &vars_of_interest = cj_literals;
			if (useDiagnostic)
			{
				vector<int> cj_and_f = f;
				cj_and_f.insert(cj_and_f.end(), cj_literals.begin(), cj_literals.end());

				vars_of_interest = cj_and_f;
			}

			while (solver_->incIsSatModelOrCore(odiff_enable_literals, vars_of_interest, model))
			{
				if (useDiagnostic)
				{
					trace = new ErrorTrace;

					trace->error_timestep_ = timestep;
					trace->input_trace_ = testcase;
					ErrorTraceManager::instance().error_traces_.push_back(trace);
				}
				vector<int>::iterator model_iter;
				for (model_iter = model.begin(); model_iter != model.end(); ++model_iter)
				{
					int lit = *model_iter;
					if (lit > 0 && lit < next_cnf_var_after_ci_vars) // we have found the one and only active cj signal
					{
						// Add vulnerable latch (represented by cj) to list of vulnerabilities
						// Add blocking clause so that the sat-solver can report other vulnerabilities
						vulnerable_elements_.insert(cj_to_latch[lit]);
						latches_to_check_.erase(cj_to_latch[lit]);
						solver_->incAddUnitClause(-lit); // blocking clause
						if (!useDiagnostic)
							break; // we are done here
						else
							trace->latch_index_ = cj_to_latch[lit];
					}
					else if (useDiagnostic && lit >= next_cnf_var_after_ci_vars) // we have found the one and only active fi signal
					{
						trace->flipped_timestep_ = fi_to_timestep[lit];
					}
				}
			}

			// negate (=set to positive face) newest odiff_enable_literal to disable
			// the previous o_is_diff_clausefor the next iterations
			odiff_enable_literals.back() = -odiff_enable_literals.back();

			continue; // TODO remove this or the following "optimizations"

			//--------------------------------------------------------------------------------------
			// Optimization: next state does not change,no matter if we flip or not -> remove fi's

			int next_state_is_diff = next_free_cnf_var++;
			vector<int> next_state_is_diff_clause;
			next_state_is_diff_clause.reserve(next_state_cnf_values.size() + 1);
			for (size_t cnt = 0; cnt < next_state_cnf_values.size(); ++cnt)
			{
				int lit_to_add = 0;
				if (next_state[cnt] == AIG_TRUE) // simulation result of output is true
					lit_to_add = -next_state_cnf_values[cnt]; // add negated output
				else
					lit_to_add = next_state_cnf_values[cnt];
				if (lit_to_add != CNF_FALSE)
					next_state_is_diff_clause.push_back(lit_to_add);
			}

			//--------------------------------------------------------------------------------------
			if (next_state_is_diff_clause.empty()) // -> start new solver session
			{
				solver_->startIncrementalSession(cj_literals, 0);
				solver_->addVarToKeep(1);
				solver_->incAddUnitClause(CNF_TRUE); // -1 = TRUE constant

				next_free_cnf_var = next_cnf_var_after_ci_vars;

				//------------------------------------------------------------------------------------
				// single fault assumption: there might be at most one flipped component
				set<int>::iterator l1_it;
				set<int>::iterator l2_it;
				for (l1_it = latches_to_check_.begin(); l1_it != latches_to_check_.end();
						l1_it++)
				{
					l2_it = l1_it;
					l2_it++;
					for (; l2_it != latches_to_check_.end(); l2_it++)
						solver_->incAdd2LitClause(-latch_to_cj[*l1_it >> 1],
								-latch_to_cj[*l2_it >> 1]);
				}
				//------------------------------------------------------------------------------------

				f.clear();
				odiff_enable_literals.clear();

				continue;
			}
			else
			{
//				if (next_state_is_diff_clause.size() == 1)									// TODO!
//				{
//					for (unsigned cnt = 0; cnt < f.size(); cnt++)
//						solver_->incAdd2LitClause(-f[cnt], next_state_is_diff_clause[0]);
//				}
//				else
				{
					solver_->addVarToKeep(next_state_is_diff);
					next_state_is_diff_clause.push_back(-next_state_is_diff);
					solver_->incAddClause(next_state_is_diff_clause);
					for (unsigned cnt = 0; cnt < f.size(); cnt++)
						solver_->incAdd2LitClause(-f[cnt], next_state_is_diff);
				}
			}
			//--------------------------------------------------------------------------------------

			//------------------------------------------------------------------------------------
			// Optimization2: compute unsat core

			if (f.size() > 1 && (unsat_core_interval_ != 0)
					&& (f.size() % unsat_core_interval_ == 0)
					&& (testcase.size() - timestep > unsat_core_interval_))
			{

				vector<int> core_assumptions;
				core_assumptions.reserve(f.size());

				for (vector<int>::iterator it = f.begin(); it != f.end(); ++it)
				{
					core_assumptions.push_back(-*it);
				}
				vector<int> more_assumptions;
				more_assumptions.push_back(next_state_is_diff);
				vector<int> core;
				bool is_sat_2 = solver_->incIsSatModelOrCore(core_assumptions, more_assumptions,
						f, core);
				MASSERT(is_sat_2 == false, "must not be satisfiable")

				// TODO: not sure if there could be a more efficient way to do this
				// (e.g. under the assumption that results of core have same order as f):
				Utils::debugPrint(core, "core: ");
				Utils::debugPrint(f, "f: ");
				set<int> useless(f.begin(), f.end());
				for (vector<int>::iterator it = core.begin(); it != core.end(); ++it)
				{
					useless.erase(-*it);
				}

				for (set<int>::iterator it = useless.begin(); it != useless.end(); ++it)
				{
					solver_->incAddUnitClause(-*it);
				}

				int num_reduced_f_variables = f.size() - core.size();
				f.clear();
				for (vector<int>::iterator it = core.begin(); it != core.end(); ++it)
				{
					f.push_back(-*it);
				}
				L_DBG("step"<<timestep<<" reduced f variables: " << num_reduced_f_variables)
				// END TODO

			}

		} // -- END "for each timestep in testcase" --
		if (environment_model_)
			delete environment_sim;
	} // ------ END 'for each testcase' ---------------


	delete sim_;
}

void SymbTimeLocationAnalysis::Analyze2_free_inputs(vector<TestCase>& testcases)
{
	// used to store the results of the symbolic simulation
	int next_free_cnf_var = 2;

	set<int> latches_to_check_;

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
	SymbolicSimulator sim_ok(circuit_, solver_, next_free_cnf_var);
	SymbolicSimulator symbsim(circuit_, solver_, next_free_cnf_var);
	SymbolicSimulator* sim_env = 0;
	if (environment_model_)
		sim_env = new SymbolicSimulator(environment_model_, solver_, next_free_cnf_var);

	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{

		// f = a set of variables fi indicating whether a latch is *flipped in _step_ i* or not
		vector<int> f;
		map<int, unsigned> fi_to_timestep;

		// a set of cj-literals indicating whether *the _latch_ C_j is flipped* or not
		vector<int> cj_literals;
		map<int, int>::iterator map_iter;
		for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++)
		{
			cj_literals.push_back(map_iter->second);
		}

		// a set of literals to enable or disable the represented output_is_different clauses,
		// necessary for incremental solving. At each sat-solver call only the newest clause
		// must be active:The newest enable-lit is always set to FALSE, while all other are TRUE
		vector<int> odiff_enable_literals;

		// start new incremental SAT-solving session
		solver_->startIncrementalSession(cj_literals, 0);
		solver_->addVarToKeep(abs(CNF_TRUE));
		solver_->incAddUnitClause(CNF_TRUE); // CNF_TRUE= unit-clause representing TRUE constant

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

		symbsim.initLatches();
		sim_ok.initLatches();
		if (environment_model_)
			sim_env->initLatches();

//		AndCacheFor2Simulators cache(sim_ok.getResults(), symbsim.getResults(), solver_,
//				next_free_cnf_var);
		AndCacheMap cache(solver_);
		symbsim.setCache(&cache);
		sim_ok.setCache(&cache);
		// TODO ENV set cache?

		TestCase& testcase = testcases[tc_number];
		TestCase real_cnf_inputs;
		for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

//			cache.clearCache();
			//--------------------------------------------------------------------------------------
			// Correct simulation:
			sim_ok.simulateOneTimeStep(testcase[timestep]);
			const vector<int> &correct_outputs = sim_ok.getOutputValues();
//
//			const vector<int> &correct_next_state = sim_ok.getLatchValues();
			const vector<int> &correct_next_state = sim_ok.getNextLatchValues();
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
			// cj is a variable that indicatest whether the corresponding latch is flipped
			// fi is a variable that indicates whether the component is flipped in step timestep or not
			// there can only be a flip at timestep timestep if both cj and fi are true.
			int fi = next_free_cnf_var++;
			solver_->addVarToKeep(fi);

			set<int>::iterator it;
			for (it = latches_to_check_.begin(); it != latches_to_check_.end(); ++it)
			{
				int latch_output = *it >> 1;
				int old_value = symbsim.getResultValue(latch_output);
				int new_value = next_free_cnf_var++;
				int ci_lit = latch_to_cj[latch_output];

				//solver_->addVarToKeep(new_value);

				solver_->incAdd3LitClause(old_value, ci_lit, -new_value);
				solver_->incAdd3LitClause(old_value, fi, -new_value);
				solver_->incAdd4LitClause(-old_value, -ci_lit, -fi, -new_value);
				solver_->incAdd3LitClause(-old_value, ci_lit, new_value);
				solver_->incAdd3LitClause(-old_value, fi, new_value);
				solver_->incAdd4LitClause(old_value, -ci_lit, -fi, new_value);

				symbsim.setResultValue(latch_output, new_value);
			}

			//--------------------------------------------------------------------------------------
			// single fault assumption: there might be at most one flip in one time-step
			// if fi is true, all oter f must be false (fi -> -f1, fi -> -f2, ...)
			for (unsigned cnt = 0; cnt < f.size(); cnt++)
				solver_->incAdd2LitClause(-fi, -f[cnt]);

			f.push_back(fi);

			if (Options::instance().isUseDiagnosticOutput())
			{
				fi_to_timestep[fi] = timestep;
				real_cnf_inputs.push_back(sim_ok.getInputValues());
			}

			//--------------------------------------------------------------------------------------
			// Symbolic simulation of AND gates
			symbsim.setCnfInputValues(sim_ok.getInputValues());
			symbsim.simulateOneTimeStep();

			vector<int> env_outputs;
			if (environment_model_)
			{
				vector<int> env_input;
				const vector<int>& input_values = sim_ok.getInputValues();

				env_input.reserve(input_values.size() + correct_outputs.size());
				env_input.insert(env_input.end(), input_values.begin(),
						input_values.end());
				env_input.insert(env_input.end(), correct_outputs.begin(), correct_outputs.end());

				sim_env->setCnfInputValues(env_input);
				sim_env->simulateOneTimeStep();
				env_outputs = sim_env->getOutputValues();
				sim_env->switchToNextState();
			}
			// get Outputs and next state values, switch to next state
			solver_->incAddUnitClause(-symbsim.getAlarmValue()); // set alarm to false
			const vector<int> &out_cnf_values = symbsim.getOutputValues();
			symbsim.switchToNextState();
			const vector<int> &next_state_cnf_values = symbsim.getLatchValues(); // already next st
			sim_ok.switchToNextState(); // concrete simulation also to next state
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
			// clause saying that the outputs o and o' are different
			int o_is_diff_enable_literal = next_free_cnf_var++;
			vector<int> o_is_diff_clause;

			o_is_diff_clause.reserve(out_cnf_values.size() + 1);
			for (unsigned out_idx = 0; out_idx < out_cnf_values.size(); ++out_idx)
			{
				if (!environment_model_ && correct_outputs[out_idx] == CNF_TRUE) // simulation result of output is true
					o_is_diff_clause.push_back(-out_cnf_values[out_idx]); // add negated output
				else if (!environment_model_ && correct_outputs[out_idx] == CNF_FALSE)
					o_is_diff_clause.push_back(out_cnf_values[out_idx]);
				else if (!environment_model_ && out_cnf_values[out_idx] == CNF_TRUE)
					o_is_diff_clause.push_back(-correct_outputs[out_idx]);
				else if (!environment_model_ && out_cnf_values[out_idx] == CNF_FALSE)
					o_is_diff_clause.push_back(-correct_outputs[out_idx]);
				else if (out_cnf_values[out_idx] != correct_outputs[out_idx]) // both are symbolic and not equal
				{
					int o_is_different_var = next_free_cnf_var++;
					// both outputs are true --> o_is_different_var is false
					solver_->incAdd3LitClause(-correct_outputs[out_idx],
							-out_cnf_values[out_idx], -o_is_different_var);
					// both outputs are false --> o_is_different_var is false
					solver_->incAdd3LitClause(correct_outputs[out_idx], out_cnf_values[out_idx],
							-o_is_different_var);

					if (environment_model_)
					{
						int output_is_relevant = env_outputs[out_idx];

						// res = o_is_diff AND o_is_relevant
						int res = next_free_cnf_var++;
						solver_->incAdd2LitClause(o_is_different_var, -res);
						solver_->incAdd2LitClause(output_is_relevant, -res);
						solver_->incAdd3LitClause(-o_is_different_var, -output_is_relevant,
								res);

						o_is_different_var = res;

					}
					o_is_diff_clause.push_back(o_is_different_var);
				}
			}

			o_is_diff_clause.push_back(o_is_diff_enable_literal);
			odiff_enable_literals.push_back(-o_is_diff_enable_literal);
			solver_->addVarToKeep(o_is_diff_enable_literal);
			solver_->incAddClause(o_is_diff_clause);
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
			// call SAT-solver
			vector<int> model;
			ErrorTrace* trace = 0;
			bool useDiagnostic = Options::instance().isUseDiagnosticOutput();

			vector<int> &vars_of_interest = cj_literals;
			if (useDiagnostic)
			{
				vector<int> cj_f_openinputs = f;
				cj_f_openinputs.insert(cj_f_openinputs.end(), cj_literals.begin(),
						cj_literals.end());
				const vector<int> &open_in = sim_ok.getOpenInputVars();
				cj_f_openinputs.insert(cj_f_openinputs.end(), open_in.begin(), open_in.end());

				vars_of_interest = cj_f_openinputs;
			}

			while (solver_->incIsSatModelOrCore(odiff_enable_literals, vars_of_interest, model))
			{
				if (useDiagnostic)
				{
					trace = new ErrorTrace;

					trace->error_timestep_ = timestep;
					ErrorTraceManager::instance().error_traces_.push_back(trace);
				}

				map<int, unsigned> cnf_input_var_to_aig_truth_lit;
				cnf_input_var_to_aig_truth_lit[CNF_TRUE] = AIG_TRUE;
				cnf_input_var_to_aig_truth_lit[CNF_FALSE] = AIG_FALSE;

				vector<int>::iterator model_iter;
				for (model_iter = model.begin(); model_iter != model.end(); ++model_iter)
				{
					int lit = *model_iter;
					if (lit > 0 && lit < next_cnf_var_after_ci_vars) // we have found the one and only cj signal which was active
					{
						// Add vulnerable latch (represented by cj) to list of vulnerabilities
						// Add blocking clause so that the sat-solver can report other vulnerabilities
						vulnerable_elements_.insert(cj_to_latch[lit]);
						latches_to_check_.erase(cj_to_latch[lit]);
						solver_->incAddUnitClause(-lit); // blocking clause

						if (!useDiagnostic)
							break; // we are done here
						else
							trace->latch_index_ = cj_to_latch[lit];
					}
					else if (useDiagnostic && abs(lit) >= next_cnf_var_after_ci_vars) // not a ci var
					{
						map<int, unsigned>::iterator it = fi_to_timestep.find(lit);
						if (lit > 0 && it != fi_to_timestep.end()) // we have found the fi variable which was set to TRUE
						{
							trace->flipped_timestep_ = it->second; // store timestep
						}
						else if (it == fi_to_timestep.end()) // if not in map, then it must be a free input literal
						{
							cnf_input_var_to_aig_truth_lit[lit] =
									(lit > 0) ? AIG_TRUE : AIG_FALSE;
						}

					}
				} // END read SAT assignment

				if (useDiagnostic)
				{
					TestCase &real_input_values = trace->input_trace_;
					real_input_values.reserve(real_cnf_inputs.size());
					for (TestCase::const_iterator in = real_cnf_inputs.begin();
							in != real_cnf_inputs.end(); ++in)
					{

						vector<int> real_input_vector;
						real_input_vector.reserve(in->size());
						for (vector<int>::const_iterator iv = in->begin(); iv != in->end();
								++iv)
						{
							real_input_vector.push_back(cnf_input_var_to_aig_truth_lit[*iv]);
						}
						real_input_values.push_back(real_input_vector);
					}
				}
			}

			// negate (=set to positive face) newest odiff_enable_literal to disable
			// the previous o_is_diff_clausefor the next iterations
			odiff_enable_literals.back() = -odiff_enable_literals.back();

			continue; // TODO: skip the optimizations for now...

			//--------------------------------------------------------------------------------------
			// Optimization: next state does not change,no matter if we flip or not -> remove fi's

			int next_state_is_diff = next_free_cnf_var++;
			vector<int> next_state_is_diff_clause;
			next_state_is_diff_clause.reserve(next_state_cnf_values.size() + 1);
			for (size_t cnt = 0; cnt < next_state_cnf_values.size(); ++cnt)
			{
				int lit_to_add = 0;
				if (correct_next_state[cnt] == CNF_TRUE) // simulation result of output is true
					lit_to_add = -next_state_cnf_values[cnt]; // add negated output
				else if (correct_next_state[cnt] == CNF_FALSE)
					lit_to_add = next_state_cnf_values[cnt];
				else if (next_state_cnf_values[cnt] == CNF_TRUE) // simulation result of output is true
					lit_to_add = -correct_next_state[cnt]; // add negated output
				else if (next_state_cnf_values[cnt] == CNF_FALSE)
					lit_to_add = correct_next_state[cnt];
				else if (correct_next_state[cnt] != next_state_cnf_values[cnt]) // both are symbolic and not equal
				{
					lit_to_add = next_free_cnf_var++;
					// both outputs are true --> o_is_different_var is false
					solver_->incAdd3LitClause(-correct_next_state[cnt],
							-next_state_cnf_values[cnt], -lit_to_add);
					// both outputs are false --> o_is_different_var is false
					solver_->incAdd3LitClause(correct_next_state[cnt],
							next_state_cnf_values[cnt], -lit_to_add);
				}

				if (lit_to_add != CNF_FALSE)
					next_state_is_diff_clause.push_back(lit_to_add);
			}

			//--------------------------------------------------------------------------------------
			if (next_state_is_diff_clause.empty()) // -> start new solver session
			{
				solver_->startIncrementalSession(cj_literals, 0);
				solver_->addVarToKeep(1);
				solver_->incAddUnitClause(CNF_TRUE); // -1 = TRUE constant

				next_free_cnf_var = next_cnf_var_after_ci_vars;

				//------------------------------------------------------------------------------------
				// single fault assumption: there might be at most one flipped component
				set<int>::iterator l1_it;
				set<int>::iterator l2_it;
				for (l1_it = latches_to_check_.begin(); l1_it != latches_to_check_.end();
						l1_it++)
				{
					l2_it = l1_it;
					l2_it++;
					for (; l2_it != latches_to_check_.end(); l2_it++)
						solver_->incAdd2LitClause(-latch_to_cj[*l1_it >> 1],
								-latch_to_cj[*l2_it >> 1]);
				}
				//------------------------------------------------------------------------------------

				f.clear();
				odiff_enable_literals.clear();

				continue;
			}
//			else
//			{
////				if (next_state_is_diff_clause.size() == 1)									// TODO!
////				{
////					for (unsigned cnt = 0; cnt < f.size(); cnt++)
////						solver_->incAdd2LitClause(-f[cnt], next_state_is_diff_clause[0]);
////				}
////				else
//				{
//					solver_->addVarToKeep(next_state_is_diff);
//					next_state_is_diff_clause.push_back(-next_state_is_diff);
//					solver_->incAddClause(next_state_is_diff_clause);
//					for (unsigned cnt = 0; cnt < f.size(); cnt++)
//						solver_->incAdd2LitClause(-f[cnt], next_state_is_diff);
//				}
//			}
			//--------------------------------------------------------------------------------------

			//------------------------------------------------------------------------------------
			// Optimization2: compute unsat core

			if (f.size() > 1 && (unsat_core_interval_ != 0)
					&& (f.size() % unsat_core_interval_ == 0)
					&& (testcase.size() - timestep > unsat_core_interval_))
			{

				vector<int> core_assumptions;
				core_assumptions.reserve(f.size());

				for (vector<int>::iterator it = f.begin(); it != f.end(); ++it)
				{
					core_assumptions.push_back(-*it);
				}
				vector<int> more_assumptions;
				more_assumptions.push_back(next_state_is_diff);
				vector<int> core;
				bool is_sat_2 = solver_->incIsSatModelOrCore(core_assumptions, more_assumptions,
						f, core);
				MASSERT(is_sat_2 == false, "must not be satisfiable")

				// TODO: not sure if there could be a more efficient way to do this
				// (e.g. under the assumption that results of core have same order as f):
				Utils::logPrint(core, "core: ");
				Utils::logPrint(f, "f: ");
				set<int> useless(f.begin(), f.end());
				for (vector<int>::iterator it = core.begin(); it != core.end(); ++it)
				{
					useless.erase(-*it);
				}

				for (set<int>::iterator it = useless.begin(); it != useless.end(); ++it)
				{
					solver_->incAddUnitClause(-*it);
				}

				int num_reduced_f_variables = f.size() - core.size();
				f.clear();
				for (vector<int>::iterator it = core.begin(); it != core.end(); ++it)
				{
					f.push_back(-*it);
				}
				L_LOG("step"<<timestep<<" reduced f variables: " << num_reduced_f_variables)
				// END TODO

			}

		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each latch' ---------------

	if(sim_env)
		delete sim_env;
}

vector<vector<int> > SymbTimeLocationAnalysis::computeRelevantOutputs(TestCase& testcase)
{
	// if environment-model: define which output is relevant at which point in time:
	vector<vector<int> > output_is_relevant;

	output_is_relevant.reserve(testcase.size());
	AigSimulator environment_sim(environment_model_);
	environment_sim.setTestcase(testcase);
	while (environment_sim.simulateOneTimeStep())
	{
		output_is_relevant.push_back(environment_sim.getOutputs());
	}

	return output_is_relevant;
}

bool SymbTimeLocationAnalysis::isARelevantOutputDifferent(vector<int>& out1, vector<int>& out2,
		vector<int>& out_is_relevant)
{
	for (unsigned out_idx = 0; out_idx < out1.size(); out_idx++)
	{
		// if output is relevant
		if (out_is_relevant[out_idx] == AIG_TRUE)
		{
			// AND output value is different
			if (out2[out_idx] != out1[out_idx])
			{
				return true;
			}
		}
	}
	return false;
}
