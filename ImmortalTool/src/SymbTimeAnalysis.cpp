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
// For more information about this software see
//   <http://www.iaik.tugraz.at/content/research/design_verification/others/>
// or email the authors directly.
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file SymbTimeAnalysis.cpp
/// @brief Contains the definition of the class SymbTimeAnalysis.
// -------------------------------------------------------------------------------------------

#include "SymbTimeAnalysis.h"
#include "Utils.h"
#include "AIG2CNF.h"
#include "Options.h"
#include "Logger.h"
#include "SymbolicSimulator.h"
#include "AndCacheMap.h"
#include "ErrorTraceManager.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SymbTimeAnalysis::SymbTimeAnalysis(aiger* circuit, int num_err_latches, int mode) :
		BackEnd(circuit, num_err_latches, mode), sim_(0)
{
	solver_ = Options::instance().getSATSolver();
	unsat_core_interval_ = Options::instance().getUnsatCoreInterval();
}

// -------------------------------------------------------------------------------------------
SymbTimeAnalysis::~SymbTimeAnalysis()
{
	delete solver_;
}

// -------------------------------------------------------------------------------------------
bool SymbTimeAnalysis::analyze(vector<TestCase> &testcases)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();

	if (mode_ == NAIVE)
		Analyze1_naive(testcases);
	else if (mode_ == SYMBOLIC_SIMULATION)
		Analyze1_symb_sim(testcases);
	else if (mode_ == FREE_INPUTS)
		Analyze1_free_inputs(testcases);
	else
		MASSERT(false, "unknown mode!");

	return (vulnerable_elements_.size() != 0);
}


// -------------------------------------------------------------------------------------------
void SymbTimeAnalysis::Analyze1_naive(vector<TestCase> &testcases)
{

	sim_ = new AigSimulator(circuit_);

	AIG2CNF::instance().initFromAig(circuit_);
// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = AIG2CNF::instance().aigLitToCnfLit(component_aig);

		int next_free_cnf_var = AIG2CNF::instance().getMaxCnfVar() + 1;

		CNF T_err = AIG2CNF::instance().getTrans();

		int f_orig = next_free_cnf_var++;
		int poss_neg_state_cnf_var = next_free_cnf_var++;

		vector<int> first_rename_map;
		first_rename_map.reserve(next_free_cnf_var);
		for (int i = 0; i < next_free_cnf_var; ++i)
			first_rename_map.push_back(i);
		first_rename_map[component_cnf] = poss_neg_state_cnf_var;
		T_err.renameVars(first_rename_map);

		T_err.add3LitClause(-f_orig, -component_cnf, -poss_neg_state_cnf_var);
		T_err.add3LitClause(-f_orig, component_cnf, poss_neg_state_cnf_var);
		T_err.add3LitClause(f_orig, -component_cnf, poss_neg_state_cnf_var);
		T_err.add3LitClause(f_orig, component_cnf, -poss_neg_state_cnf_var);

		for (unsigned tci = 0; tci < testcases.size(); tci++)
		{
			// concrete_state[] = (0 0 0 0 0 0 0)   // AIG literals
			vector<int> concrete_state;
			concrete_state.resize(circuit_->num_latches);

			// symb_state[] = (1 1 1 1 1)  // CNF literals
			vector<int> symb_state;
			symb_state.reserve(circuit_->num_latches);
			for (unsigned i = 0; i < circuit_->num_latches; i++)
			{
				symb_state.push_back(1);
			}

			vector<int> f;
			vector<int> odiff_literals;
			map<int, unsigned> fi_to_timestep;

			vector<int> vars_to_keep;
			vars_to_keep.push_back(1); // TRUE and FALSE literals
			solver_->startIncrementalSession(vars_to_keep, 0);

			vector<int> cnf_o_terr_orig = AIG2CNF::instance().getOutputs();
			for (unsigned cnt = 0; cnt < cnf_o_terr_orig.size(); ++cnt)
				cnf_o_terr_orig[cnt] = Utils::applyRen(first_rename_map, cnf_o_terr_orig[cnt]);
			vector<int> cnf_next_terr_orig = AIG2CNF::instance().getNextStateVars();
			for (unsigned cnt = 0; cnt < cnf_next_terr_orig.size(); ++cnt)
				cnf_next_terr_orig[cnt] = Utils::applyRen(first_rename_map,
						cnf_next_terr_orig[cnt]);

			int max_cnf_var_in_Terr = next_free_cnf_var;
			TestCase& testcase = testcases[tci];

			// if environment-model: define which output is relevant at which point in time:
			AigSimulator* environment_sim = 0;
			if (environment_model_)
				environment_sim = new AigSimulator(environment_model_);


			for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
			{ // -------- BEGIN "for each timestep in testcase" -----------

				// correct simulation
				sim_->simulateOneTimeStep(testcase[timestep], concrete_state);
				vector<int> outputs_ok = sim_->getOutputs();
				vector<int> next_state = sim_->getNextLatchValues();
//				Utils::debugPrint(next_state, "next state");

				// flip component bit
				vector<int> faulty_state = concrete_state;
				faulty_state[c_cnt] = (faulty_state[c_cnt] == 1) ? 0 : 1;

				// faulty simulation with flipped bit
				sim_->simulateOneTimeStep(testcase[timestep], faulty_state);
				vector<int> outputs2 = sim_->getOutputs();
				bool alarm = (outputs2[outputs2.size() - 1] == 1);

//				Utils::debugPrint(testcase[timestep], "inputs");
//				Utils::debugPrint(outputs, "outputs");
//				Utils::debugPrint(outputs2, "outputs2");

				// check if vulnerablitiy already found
				bool equal_outputs = (outputs_ok == outputs2);
				bool err_found_with_simulation = (!equal_outputs && !alarm);

				// if we have an environment-model and different outputs, check if output is
				// really relevant
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
				if (environment_model_ && err_found_with_simulation)
				{
					err_found_with_simulation = isARelevantOutputDifferent(outputs_ok, outputs2,
							output_is_relevant);
				}

				if (err_found_with_simulation)
				{
					vulnerable_elements_.insert(component_aig);

					if (Options::instance().isUseDiagnosticOutput())
					{
						ErrorTrace* trace = new ErrorTrace;
						trace->flipped_timestep_ = timestep;
						trace->error_timestep_ = timestep;
						trace->latch_index_ = component_aig;
						trace->input_trace_ = testcase;
						ErrorTraceManager::instance().error_traces_.push_back(trace);
					}
					break;

				}

				bool err_is_no_vulnerability =
						false
								&& (alarm
										|| (equal_outputs
												&& (next_state == sim_->getNextLatchValues())));

				vector<int> real_rename_map(max_cnf_var_in_Terr, 0);
				for (unsigned cnt = 0; cnt < real_rename_map.size(); ++cnt)
					real_rename_map[cnt] = cnt;
				real_rename_map[1] = 1;

				// rename each AND gate with a fresh variable
				for (unsigned cnt = 0; cnt < circuit_->num_ands; ++cnt)
				{
					unsigned and_cnf = (circuit_->ands[cnt].lhs >> 1) + 1;
					real_rename_map[and_cnf] = next_free_cnf_var++;
				}

				// rename: set latch values to our symb_state
				// (ALTERNATIVE: don't rename, set via T_err_copy.setVarValue(), but BEFORE renaming)
				for (unsigned cnt = 0; cnt < circuit_->num_latches; ++cnt)
				{
					unsigned latch_cnf = (circuit_->latches[cnt].lit >> 1) + 1;
					real_rename_map[latch_cnf] = symb_state[cnt];
				}

				// rename: set inputs according to test case inputs
				for (unsigned in_idx = 0; in_idx < circuit_->num_inputs; ++in_idx)
				{
					unsigned input_cnf = (circuit_->inputs[in_idx].lit >> 1) + 1;
					int input_bit_value = AIG2CNF::instance().aigLitToCnfLit(
							testcase[timestep][in_idx]);
					real_rename_map[input_cnf] = input_bit_value;
				}

				// C
				int fi = next_free_cnf_var++;
				if (!err_is_no_vulnerability)
				{
					f.push_back(fi);
					fi_to_timestep[fi] = timestep;

					real_rename_map[f_orig] = fi;
					real_rename_map[poss_neg_state_cnf_var] = next_free_cnf_var++;
					solver_->addVarToKeep(fi);
				}

				CNF T_copy;
				vector<int> &cnf_o = cnf_o_terr_orig;
				vector<int> &cnf_next = cnf_next_terr_orig;
				if (!err_is_no_vulnerability)
				{
					T_copy = T_err;
				}
				else
				{
					T_copy = AIG2CNF::instance().getTrans();
					real_rename_map[component_cnf] = next_free_cnf_var++;

					cnf_o = AIG2CNF::instance().getOutputs();
					cnf_next = AIG2CNF::instance().getNextStateVars();

				}
//				Utils::debugPrint(real_rename_map, "Rename map:");
				T_copy.setVarValue(AIG2CNF::instance().getAlarmOutput(), false); // alarm = false
				T_copy.renameVars(real_rename_map);
				solver_->incAddCNF(T_copy);
//				L_DBG("T_Copy = " << endl <<T_copy.toString() << endl);

				if (!err_is_no_vulnerability)
				{
					// if fi is true, all oter f must be false (fi -> -f_1 AND -f_2 AND .. AND -f_i-1)
					for (unsigned cnt = 0; cnt < f.size() - 1; cnt++)
						solver_->incAdd2LitClause(-fi, -f[cnt]);
				}

				// rename each output except alarm output
				vector<int> renamed_out_vars;
				renamed_out_vars.reserve(cnf_o.size());
				for (unsigned cnt = 0; cnt < cnf_o.size(); ++cnt)
					renamed_out_vars.push_back(Utils::applyRen(real_rename_map, cnf_o[cnt]));
//				Utils::debugPrint(renamed_out_vars, "symbolic outputs_ok: ");

				// clause saying that the outputs_ok o and o' are different
				vector<int> o_is_diff_clause;
				o_is_diff_clause.reserve(renamed_out_vars.size() + 1);
				for (unsigned out_idx = 0; out_idx < renamed_out_vars.size(); ++out_idx)
				{
					// skip if output is not relevant
					if (environment_model_ && output_is_relevant[out_idx] == AIG_FALSE)
						continue;

					if (outputs_ok[out_idx] == AIG_TRUE) // simulation result of output is true
						o_is_diff_clause.push_back(-renamed_out_vars[out_idx]); // add false to outputs_ok
					else
						o_is_diff_clause.push_back(renamed_out_vars[out_idx]);
				}
				int o_is_diff_enable_literal = next_free_cnf_var++;
				solver_->addVarToKeep(o_is_diff_enable_literal);
				o_is_diff_clause.push_back(o_is_diff_enable_literal);
				odiff_literals.push_back(-o_is_diff_enable_literal);

				solver_->incAddClause(o_is_diff_clause);
//				Utils::debugPrint(o_is_diff_clause, "o_is_diff_clause: ");

//				Utils::debugPrint(renamed_out_vars, "renamed_out_vars: ");

				// rename next states
				vector<int> renamed_next_state_vars;
				renamed_next_state_vars.reserve(cnf_next.size());
				for (unsigned cnt = 0; cnt < cnf_next.size(); ++cnt)
					renamed_next_state_vars.push_back(
							Utils::applyRen(real_rename_map, cnf_next[cnt]));
				solver_->addVarsToKeep(renamed_next_state_vars);

				// call SAT-Solver
				vector<int> model;
				bool sat = false;
				if (Options::instance().isUseDiagnosticOutput())
					sat = solver_->incIsSatModelOrCore(odiff_literals, f, model);
				else
					sat = solver_->incIsSat(odiff_literals);

				odiff_literals.back() = -odiff_literals.back();

				if (sat)
				{
					vulnerable_elements_.insert(component_aig);

					if (Options::instance().isUseDiagnosticOutput())
						addErrorTrace(component_aig, timestep, fi_to_timestep, model, testcase);

					break;
				}

				concrete_state = next_state;

				symb_state = renamed_next_state_vars;

			} // -- END "for each timestep in testcase" --
			if (environment_model_)
				delete environment_sim;
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

	delete sim_;


}

void SymbTimeAnalysis::Analyze1_symb_sim(vector<TestCase>& testcases)
{
	sim_ = new AigSimulator(circuit_);
	int next_free_cnf_var = 2;
	SymbolicSimulator symbsim(circuit_, solver_, next_free_cnf_var);

// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = component_aig >> 1;

		next_free_cnf_var = 2;

		for (unsigned tci = 0; tci < testcases.size(); tci++)
		{

			// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
			vector<int> concrete_state;
			concrete_state.resize(circuit_->num_latches);

			// f = a set of variables fi indicating whether the latch is flipped in step i or not
			vector<int> f;
			map<int, unsigned> fi_to_timestep;

			// a set of literals to enable or disable the represented output_is_different clauses,
			// necessary for incremental solving. At each sat-solver call only the newest clause
			// must be active:The newest enable-lit is always set to FALSE, while all other are TRUE
			vector<int> odiff_enable_literals;

			// start new incremental SAT-solving session
			vector<int> vars_to_keep;
			vars_to_keep.push_back(1); // TRUE and FALSE literals
			solver_->startIncrementalSession(vars_to_keep, 0);
			solver_->incAddUnitClause(-1); // -1 = TRUE constant

			symbsim.initLatches(); // initialize latches to false

			TestCase& testcase = testcases[tci];

			// if environment-model: define which output is relevant at which point in time:
			AigSimulator* environment_sim = 0;
			if (environment_model_)
				environment_sim = new AigSimulator(environment_model_);

			for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
			{ // -------- BEGIN "for each timestep in testcase" ------------------------------------

				//------------------------------------------------------------------------------------
				// Concrete simulations:
				// correct simulation
				sim_->simulateOneTimeStep(testcase[timestep], concrete_state);
				vector<int> outputs_ok = sim_->getOutputs();
				vector<int> next_state = sim_->getNextLatchValues();

				// faulty simulation: flip component bit
				vector<int> faulty_state = concrete_state;
				faulty_state[c_cnt] = (faulty_state[c_cnt] == AIG_TRUE) ? AIG_FALSE : AIG_TRUE;

				// faulty simulation with flipped bit
				sim_->simulateOneTimeStep(testcase[timestep], faulty_state);
				vector<int> outputs2 = sim_->getOutputs();

				bool alarm = (outputs2[outputs2.size() - 1] == AIG_TRUE);
				bool equal_outputs = (outputs_ok == outputs2);
				bool err_found_with_simulation = (!equal_outputs && !alarm);

				// if we have an environment-model and different outputs, check if output is
				// really relevant
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
				if (environment_model_ && err_found_with_simulation)
				{
					err_found_with_simulation = isARelevantOutputDifferent(outputs_ok, outputs2,
							output_is_relevant);
				}

				if (err_found_with_simulation)
				{
					vulnerable_elements_.insert(component_aig);

					if (Options::instance().isUseDiagnosticOutput())
					{
						ErrorTrace* trace = new ErrorTrace;
						trace->flipped_timestep_ = timestep;
						trace->error_timestep_ = timestep;
						trace->latch_index_ = component_aig;
						trace->input_trace_ = testcase;
						ErrorTraceManager::instance().error_traces_.push_back(trace);
					}

					break;
				}

				// switch concrete simulation to next state
				concrete_state = next_state; // OR: change to sim_->switchToNextState();
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// set input values according to TestCase to TRUE or FALSE:
				symbsim.setInputValues(testcase[timestep]);
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// fi is a variable that indicates whether the component is flipped in step i or not
				bool err_is_no_vulnerability = (alarm
						|| (equal_outputs && (next_state == sim_->getNextLatchValues())));
				if (!err_is_no_vulnerability)
				{
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
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// Symbolic simulation of AND gates
				symbsim.simulateOneTimeStep();
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// get Outputs and next state values, swich to next state
				solver_->incAddUnitClause(-symbsim.getAlarmValue());

				const vector<int> &out_cnf_values = symbsim.getOutputValues();
				symbsim.switchToNextState();
				const vector<int> &next_state_cnf_values = symbsim.getLatchValues();
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// clause saying that the outputs_ok o and o' are different
				vector<int> o_is_diff_clause;
				o_is_diff_clause.reserve(out_cnf_values.size() + 1);
				for (unsigned out_idx = 0; out_idx < out_cnf_values.size(); ++out_idx)
				{
					// skip if output is not relevant
					if (environment_model_ && output_is_relevant[out_idx] == AIG_FALSE)
						continue;

					if (outputs_ok[out_idx] == AIG_TRUE) // simulation result of output is true
						o_is_diff_clause.push_back(-out_cnf_values[out_idx]); // add false to outputs
					else
						o_is_diff_clause.push_back(out_cnf_values[out_idx]);
				}
				int o_is_diff_enable_literal = next_free_cnf_var++;
				o_is_diff_clause.push_back(o_is_diff_enable_literal);
				odiff_enable_literals.push_back(-o_is_diff_enable_literal);
				solver_->addVarToKeep(o_is_diff_enable_literal);
				solver_->incAddClause(o_is_diff_clause);
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// call SAT-solver
//				vector<int> model; // TODO: maybe make use of the satisfying assignment
//				bool sat = solver_->incIsSatModelOrCore(odiff_enable_literals,
//						vars_to_keep, model);

				vector<int> model;
				bool sat = false;
				if (Options::instance().isUseDiagnosticOutput())
					sat = solver_->incIsSatModelOrCore(odiff_enable_literals, f, model);
				else
					sat = solver_->incIsSat(odiff_enable_literals);

				odiff_enable_literals.back() = -odiff_enable_literals.back();

				if (sat)
				{
					vulnerable_elements_.insert(component_aig);

					if (Options::instance().isUseDiagnosticOutput())
						addErrorTrace(component_aig, timestep, fi_to_timestep, model, testcase);

					break;
				}

				//------------------------------------------------------------------------------------
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
				if (next_state_is_diff_clause.empty())
				{
					vars_to_keep.clear();
					vars_to_keep.push_back(1); // TRUE and FALSE literals
					solver_->startIncrementalSession(vars_to_keep, 0);
					solver_->incAddUnitClause(CNF_TRUE); // -1 = TRUE constant
					next_free_cnf_var = 2;
					f.clear();
					odiff_enable_literals.clear();
					continue; // no need to do Optimization2.
				}
				else
				{
// commented out because of optimization 2
//					if (next_state_is_diff_clause.size() == 1)
//					{
//						for (unsigned cnt = 0; cnt < f.size(); cnt++)
//							solver_->incAdd2LitClause(-f[cnt], next_state_is_diff_clause[0]);
//					}
//					else
					{
						next_state_is_diff_clause.push_back(-next_state_is_diff);
						solver_->incAddClause(next_state_is_diff_clause);
						for (unsigned cnt = 0; cnt < f.size(); cnt++)
							solver_->incAdd2LitClause(-f[cnt], next_state_is_diff);
					}
				}
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// Optimization2: compute unsat core

				if (f.size() > 1 && (unsat_core_interval_ != 0)
						&& (f.size() % unsat_core_interval_ == 0))
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
					bool is_sat_2 = solver_->incIsSatModelOrCore(core_assumptions,
							more_assumptions, f, core);
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
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

	delete sim_;
}

void SymbTimeAnalysis::Analyze1_free_inputs(vector<TestCase>& testcases)
{
	int next_free_cnf_var = 2;

	SymbolicSimulator sim_ok(circuit_, solver_, next_free_cnf_var);
	SymbolicSimulator symbsim(circuit_, solver_, next_free_cnf_var);
	SymbolicSimulator* sim_env = 0;
	if (environment_model_)
		sim_env = new SymbolicSimulator(environment_model_, solver_, next_free_cnf_var);

// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = component_aig >> 1;

		next_free_cnf_var = 2;

		for (unsigned tci = 0; tci < testcases.size(); tci++)
		{

			// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
			vector<int> concrete_state;
			concrete_state.resize(circuit_->num_latches);

			// f = a set of variables fi indicating whether the latch is flipped in step i or not
			vector<int> f;
			map<int, unsigned> fi_to_timestep;

			// a set of literals to enable or disable the represented output_is_different clauses,
			// necessary for incremental solving. At each sat-solver call only the newest clause
			// must be active:The newest enable-lit is always set to FALSE, while all other are TRUE
			vector<int> odiff_enable_literals;

			// start new incremental SAT-solving session
			vector<int> vars_to_keep;
			vars_to_keep.push_back(1); // TRUE and FALSE literals
			solver_->startIncrementalSession(vars_to_keep, 0);
			solver_->incAddUnitClause(CNF_TRUE); // TRUE constant

			sim_ok.initLatches();
			symbsim.initLatches(); // initialize latches to false
			if (environment_model_)
				sim_env->initLatches();

			AndCacheMap cache(solver_);
			sim_ok.setCache(&cache);
			symbsim.setCache(&cache);

			TestCase& testcase = testcases[tci];
			TestCase real_cnf_inputs;

			for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
			{ // -------- BEGIN "for each timestep in testcase" ------------------------------------

				//------------------------------------------------------------------------------------
				// Concrete simulations:
				// correct simulation
				sim_ok.simulateOneTimeStep(testcase[timestep]);
//				sim_->simulateOneTimeStep(testcase[i], concrete_state);
				const vector<int> &outputs_ok = sim_ok.getOutputValues();
				const vector<int> &next_state = sim_ok.getNextLatchValues();

//				// faulty simulation: flip component bit
//				vector<int> faulty_state = concrete_state;
//				faulty_state[c_cnt] = (faulty_state[c_cnt] == AIG_TRUE) ? AIG_FALSE : AIG_TRUE;
//
//				// faulty simulation with flipped bit
//				sim_->simulateOneTimeStep(testcase[i], faulty_state);
//				vector<int> outputs2 = sim_->getOutputs();
//
//				bool alarm = (outputs2[outputs2.size() - 1] == AIG_TRUE);
				bool alarm = sim_ok.getAlarmValue() == CNF_TRUE;
//				bool equal_outputs = (outputs == outputs2);
//				bool err_found_with_simulation = (!equal_outputs && !alarm);
//				if (err_found_with_simulation)
//				{
//					vulnerable_elements_.insert(component_aig);
//					break;
//				}

				// switch concrete simulation to next state
				sim_ok.switchToNextState();
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// set input values according to TestCase to TRUE/FALSE or same symb values as sim_ok:
				symbsim.setCnfInputValues(sim_ok.getInputValues());

				if (Options::instance().isUseDiagnosticOutput())
					real_cnf_inputs.push_back(sim_ok.getInputValues());
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// fi is a variable that indicates whether the component is flipped in step i or not
				if (!alarm)
				{
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
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// Symbolic simulation of AND gates
				symbsim.simulateOneTimeStep();

				//------------------------------------------------------------------------------------
				vector<int> env_outputs;
				if (environment_model_)
				{
					vector<int> env_input;
					const vector<int>& input_values = sim_ok.getInputValues();

					env_input.reserve(input_values.size() + outputs_ok.size());
					env_input.insert(env_input.end(), input_values.begin(),
							input_values.end());
					env_input.insert(env_input.end(), outputs_ok.begin(), outputs_ok.end());

					sim_env->setCnfInputValues(env_input);
					sim_env->simulateOneTimeStep();
					env_outputs = sim_env->getOutputValues();
					sim_env->switchToNextState();
				}
				//------------------------------------------------------------------------------------
				// get Outputs and next state values, swich to next state
				solver_->incAddUnitClause(-symbsim.getAlarmValue());

				const vector<int> &out_cnf_values = symbsim.getOutputValues();
				symbsim.switchToNextState();
				const vector<int> &next_state_cnf_values = symbsim.getLatchValues();

				//------------------------------------------------------------------------------------
				// clause saying that the outputs_ok o and o' are different
				int o_is_diff_enable_literal = next_free_cnf_var++;
				vector<int> o_is_diff_clause;

				o_is_diff_clause.reserve(out_cnf_values.size() + 1);
				for (unsigned out_idx = 0; out_idx < out_cnf_values.size(); ++out_idx)
				{
					if (!environment_model_ && outputs_ok[out_idx] == CNF_TRUE) // simulation result of output is true
						o_is_diff_clause.push_back(-out_cnf_values[out_idx]); // add negated output
					else if (!environment_model_ && outputs_ok[out_idx] == CNF_FALSE)
						o_is_diff_clause.push_back(out_cnf_values[out_idx]);
					else if (!environment_model_ && out_cnf_values[out_idx] == CNF_TRUE)
						o_is_diff_clause.push_back(-outputs_ok[out_idx]);
					else if (!environment_model_ && out_cnf_values[out_idx] == CNF_FALSE)
						o_is_diff_clause.push_back(-outputs_ok[out_idx]);
					else if (out_cnf_values[out_idx] != outputs_ok[out_idx]) // both are symbolic and not equal
					{
						int o_is_different_var = next_free_cnf_var++;
						// both outputs_ok are true --> o_is_different_var is false
						solver_->incAdd3LitClause(-outputs_ok[out_idx], -out_cnf_values[out_idx],
								-o_is_different_var);
						// both outputs_ok are false --> o_is_different_var is false
						solver_->incAdd3LitClause(outputs_ok[out_idx], out_cnf_values[out_idx],
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
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// call SAT-solver
				vector<int> model;
				bool sat = false;
				if (Options::instance().isUseDiagnosticOutput())
				{
					vector<int> vars_of_interest = f;
					const vector<int> &open_in = sim_ok.getOpenInputVars();
					vars_of_interest.insert(vars_of_interest.end(), open_in.begin(),
							open_in.end());
					sat = solver_->incIsSatModelOrCore(odiff_enable_literals, vars_of_interest,
							model);
				}
				else
				{
					sat = solver_->incIsSat(odiff_enable_literals);
				}

				// negate (=set to positive face) newest odiff_enable_literal to disable
				// the previous o_is_diff_clausefor the next iterations
				odiff_enable_literals.back() = -odiff_enable_literals.back();
				if (sat)
				{
					vulnerable_elements_.insert(component_aig);
					if (Options::instance().isUseDiagnosticOutput())
					{
						addErrorTrace(component_aig, timestep, fi_to_timestep, model, real_cnf_inputs,
								true);
					}
					break;
				}

				//------------------------------------------------------------------------------------
				// Optimization: next state does not change,no matter if we flip or not -> remove fi's
				int next_state_is_diff = next_free_cnf_var++;
				vector<int> next_state_is_diff_clause;
				next_state_is_diff_clause.reserve(next_state_cnf_values.size() + 1);
				for (size_t cnt = 0; cnt < next_state_cnf_values.size(); ++cnt)
				{
					int lit_to_add = 0;

					if (next_state[cnt] == CNF_TRUE) // simulation result of output is true
						lit_to_add = -next_state_cnf_values[cnt]; // add negated output
					else if (next_state[cnt] == CNF_FALSE)
						lit_to_add = next_state_cnf_values[cnt];
					else if (next_state_cnf_values[cnt] == CNF_TRUE) // simulation result of output is true
						lit_to_add = -next_state[cnt]; // add negated output
					else if (next_state_cnf_values[cnt] == CNF_FALSE)
						lit_to_add = next_state[cnt];
					else if (next_state[cnt] != next_state_cnf_values[cnt]) // both are symbolic and not equal
					{
						lit_to_add = next_free_cnf_var++;
						// both outputs_ok are true --> o_is_different_var is false
						solver_->incAdd3LitClause(-next_state[cnt], -next_state_cnf_values[cnt],
								-lit_to_add);
						// both outputs_ok are false --> o_is_different_var is false
						solver_->incAdd3LitClause(next_state[cnt], next_state_cnf_values[cnt],
								-lit_to_add);
					}

					if (lit_to_add != CNF_FALSE)
						next_state_is_diff_clause.push_back(lit_to_add);
				}
				if (next_state_is_diff_clause.empty())
				{
					vars_to_keep.clear();
					vars_to_keep.push_back(1); // TRUE and FALSE literals
					solver_->startIncrementalSession(vars_to_keep, 0);
					solver_->incAddUnitClause(CNF_TRUE); // -1 = TRUE constant
					next_free_cnf_var = 2;
					f.clear();
					odiff_enable_literals.clear();
					continue; // no need to do Optimization2.
				}
//				else
//				{
//// commented out because of optimization 2
////					if (next_state_is_diff_clause.size() == 1)
////					{
////						for (unsigned cnt = 0; cnt < f.size(); cnt++)
////							solver_->incAdd2LitClause(-f[cnt], next_state_is_diff_clause[0]);
////					}
////					else
//					{
//						next_state_is_diff_clause.push_back(-next_state_is_diff);
//						solver_->incAddClause(next_state_is_diff_clause);
//						for (unsigned cnt = 0; cnt < f.size(); cnt++)
//							solver_->incAdd2LitClause(-f[cnt], next_state_is_diff);
//					}
//				}
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// Optimization2: compute unsat core

				if (f.size() > 1 && (unsat_core_interval_ != 0)
						&& (f.size() % unsat_core_interval_ == 0))
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
					bool is_sat_2 = solver_->incIsSatModelOrCore(core_assumptions,
							more_assumptions, f, core);
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
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

	if (sim_env)
		delete sim_env;
}

void SymbTimeAnalysis::addErrorTrace(unsigned latch_aig, unsigned err_timestep,
		map<int, unsigned>& f_to_i, const vector<int> &model, const TestCase& tc,
		bool open_inputs)
{
	ErrorTrace* trace = new ErrorTrace;
	trace->error_timestep_ = err_timestep;
	trace->latch_index_ = latch_aig;

	if (!open_inputs)
	{
		for (unsigned model_count = 0; model_count < model.size(); model_count++)
		{
			if (model[model_count] > 0)
			{
				trace->flipped_timestep_ = f_to_i[model[model_count]];
				break;
			}
		}
		trace->input_trace_ = tc;
	}
	else
	{
		for (unsigned model_count = 0; model_count < model.size(); model_count++)
		{
			int lit = model[model_count];
			map<int, unsigned>::iterator it = f_to_i.find(lit);
			if (lit > 0 && it != f_to_i.end()) // we have found the fi variable which was set to TRUE
			{
				trace->flipped_timestep_ = it->second; // store i
			}
			else if (it == f_to_i.end()) // if not in map, then it must be a free input literal
			{
				f_to_i[lit] = (lit > 0) ? AIG_TRUE : AIG_FALSE;
			}
		}

		f_to_i[CNF_TRUE] = AIG_TRUE;
		f_to_i[CNF_FALSE] = AIG_FALSE;

		TestCase &real_input_values = trace->input_trace_;
		real_input_values.reserve(tc.size());
		for (TestCase::const_iterator in = tc.begin(); in != tc.end(); ++in)
		{

			vector<int> real_input_vector;
			real_input_vector.reserve(in->size());
			for (vector<int>::const_iterator iv = in->begin(); iv != in->end(); ++iv)
			{
				real_input_vector.push_back(f_to_i[*iv]);
			}
			real_input_values.push_back(real_input_vector);
		}
	}

	ErrorTraceManager::instance().error_traces_.push_back(trace);
}

vector<vector<int> > SymbTimeAnalysis::computeRelevantOutputs(TestCase& testcase)
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

bool SymbTimeAnalysis::isARelevantOutputDifferent(vector<int>& out1, vector<int>& out2,
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
