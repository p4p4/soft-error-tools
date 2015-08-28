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

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SymbTimeAnalysis::SymbTimeAnalysis(aiger* circuit, int num_err_latches, int mode) :
		BackEnd(circuit, num_err_latches, mode), sim_(0)
{
	sim_ = new AigSimulator(circuit_);
	solver_ = Options::instance().getSATSolver();
	unsat_core_interval_ = Options::instance().getUnsatCoreInterval();
}

// -------------------------------------------------------------------------------------------
SymbTimeAnalysis::~SymbTimeAnalysis()
{
	delete sim_;
	delete solver_;
}

// -------------------------------------------------------------------------------------------
bool SymbTimeAnalysis::findVulnerabilities(vector<TestCase> &testcases)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();

	if (mode_ == NAIVE)
		Analyze1_naive(testcases);
	else if (mode_ == SYMBOLIC_SIMULATION)
		Analyze1_symb_sim(testcases);
	else
		MASSERT(false, "unknown mode!");

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
bool SymbTimeAnalysis::findVulnerabilities(vector<string> paths_to_TC_files)
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
void SymbTimeAnalysis::Analyze1_naive(vector<TestCase> &testcases)
{
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

			vector<int> vars_to_keep;
			vars_to_keep.push_back(1); // TRUE and FALSE literals
			solver_->startIncrementalSession(vars_to_keep, 0);

			vector<int> cnf_o_terr_orig = AIG2CNF::instance().getOutputs();
			for (unsigned cnt = 0; cnt < cnf_o_terr_orig.size(); ++cnt)
				cnf_o_terr_orig[cnt] = Utils::applyRen(first_rename_map, cnf_o_terr_orig[cnt]);
			vector<int> cnf_next_terr_orig = AIG2CNF::instance().getNextStateVars();
			for (unsigned cnt = 0; cnt < cnf_next_terr_orig.size(); ++cnt)
				cnf_next_terr_orig[cnt] = Utils::applyRen(first_rename_map, cnf_next_terr_orig[cnt]);

			int max_cnf_var_in_Terr = next_free_cnf_var;
			TestCase& testcase = testcases[tci];

			for (unsigned i = 0; i < testcase.size(); i++)
			{ // -------- BEGIN "for each timestep in testcase" -----------

				// correct simulation
				sim_->simulateOneTimeStep(testcase[i], concrete_state);
				vector<int> outputs = sim_->getOutputs();
				vector<int> next_state = sim_->getNextLatchValues();
//				Utils::debugPrint(next_state, "next state");

				// flip component bit
				vector<int> faulty_state = concrete_state;
				faulty_state[c_cnt] = (faulty_state[c_cnt] == 1) ? 0 : 1;

				// faulty simulation with flipped bit
				sim_->simulateOneTimeStep(testcase[i], faulty_state);
				vector<int> outputs2 = sim_->getOutputs();
				bool alarm = (outputs2[outputs2.size() - 1] == 1);

//				Utils::debugPrint(testcase[i], "inputs");
//				Utils::debugPrint(outputs, "outputs");
//				Utils::debugPrint(outputs2, "outputs2");

				// check if vulnerablitiy already found
				bool equal_outputs = (outputs == outputs2);
				bool err_found_with_simulation = (!equal_outputs && !alarm);
//			if (err_found_with_simulation)
//			{
//				L_DBG("BREAK Sim Latch " << component_aig);
//				vulnerable_elements_.insert(component_aig);
//				break;
//			}

				bool err_is_no_vulnerability = false
						&& (alarm || (equal_outputs && (next_state == sim_->getNextLatchValues())));

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
				// (ALTERNATIVE: don't rename, set via T_err_copy.setVarValue())
				for (unsigned cnt = 0; cnt < circuit_->num_inputs; ++cnt)
				{
					unsigned input_cnf = (circuit_->inputs[cnt].lit >> 1) + 1;
					int input_bit_value = AIG2CNF::instance().aigLitToCnfLit(testcase[i][cnt]);
					real_rename_map[input_cnf] = input_bit_value;
				}

				// C
				int fi = next_free_cnf_var++;
				if (!err_is_no_vulnerability)
				{
					f.push_back(fi);
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
//				Utils::debugPrint(renamed_out_vars, "symbolic outputs: ");

				// clause saying that the outputs o and o' are different
				vector<int> o_is_diff_clause;
				o_is_diff_clause.reserve(renamed_out_vars.size() + 1);
				for (unsigned cnt = 0; cnt < renamed_out_vars.size(); ++cnt)
				{
					if (outputs[cnt] == 1) // simulation result of output is true
						o_is_diff_clause.push_back(-renamed_out_vars[cnt]); // add false to outputs
					else
						o_is_diff_clause.push_back(renamed_out_vars[cnt]);
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
					renamed_next_state_vars.push_back(Utils::applyRen(real_rename_map, cnf_next[cnt]));
				solver_->addVarsToKeep(renamed_next_state_vars);

				// call SAT-Solver
				vector<int> model;
				bool sat = solver_->incIsSatModelOrCore(odiff_literals, T_copy.getVars(), model);
				odiff_literals.back() = -odiff_literals.back();

				if (sat)
				{
					vulnerable_elements_.insert(component_aig);
					break;
				}

				concrete_state = next_state;

				symb_state = renamed_next_state_vars;

			} // -- END "for each timestep in testcase" --
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------
}

void SymbTimeAnalysis::Analyze1_symb_sim(vector<TestCase>& testcases)
{
	// used to store the results of the symbolic simulation
	vector<int> results;
	results.resize(circuit_->maxvar + 1);
	results[0] = 1; // FALSE and TRUE constants

	// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = component_aig >> 1;

		int next_free_cnf_var = 2;

		for (unsigned tci = 0; tci < testcases.size(); tci++)
		{

			// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
			vector<int> concrete_state;
			concrete_state.resize(circuit_->num_latches);

			// f = a set of variables fi indicating whether the latch is flipped in step i or not
			vector<int> f;

			// a set of literals to enable or disable the represented output_is_different clauses,
			// necessary for incremental solving. At each sat-solver call only the newest clause
			// must be active:The newest enable-lit is always set to FALSE, while all other are TRUE
			vector<int> odiff_enable_literals;

			// start new incremental SAT-solving session
			vector<int> vars_to_keep;
			vars_to_keep.push_back(1); // TRUE and FALSE literals
			solver_->startIncrementalSession(vars_to_keep, 0);
			solver_->incAddUnitClause(-1); // -1 = TRUE constant

			//--------------------------------------------------------------------------------------
			for (unsigned l = 0; l < circuit_->num_latches; ++l) // initialize latches to false
				results[(circuit_->latches[l].lit >> 1)] = CNF_FALSE;
			//--------------------------------------------------------------------------------------

			TestCase& testcase = testcases[tci];
			for (unsigned i = 0; i < testcase.size(); i++)
			{ // -------- BEGIN "for each timestep in testcase" ------------------------------------

				//------------------------------------------------------------------------------------
				// Concrete simulations:
				// correct simulation
				sim_->simulateOneTimeStep(testcase[i], concrete_state);
				vector<int> outputs = sim_->getOutputs();
				vector<int> next_state = sim_->getNextLatchValues();

				// faulty simulation: flip component bit
				vector<int> faulty_state = concrete_state;
				faulty_state[c_cnt] = (faulty_state[c_cnt] == AIG_TRUE) ? AIG_FALSE : AIG_TRUE;

				// faulty simulation with flipped bit
				sim_->simulateOneTimeStep(testcase[i], faulty_state);
				vector<int> outputs2 = sim_->getOutputs();

				bool alarm = (outputs2[outputs2.size() - 1] == AIG_TRUE);
				bool equal_outputs = (outputs == outputs2);
				bool err_found_with_simulation = (!equal_outputs && !alarm);
				if (err_found_with_simulation)
				{
					vulnerable_elements_.insert(component_aig);
					break;
				}

				// switch concrete simulation to next state
				concrete_state = next_state; // OR: change to sim_->switchToNextState();
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// set input values according to TestCase to TRUE or FALSE:
				for (unsigned cnt_i = 0; cnt_i < circuit_->num_inputs; ++cnt_i)
					results[(circuit_->inputs[cnt_i].lit >> 1)] = (testcase[i][cnt_i] == 1) ? -1 : 1;
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// fi is a variable that indicates whether the component is flipped in step i or not
				bool err_is_no_vulnerability = (alarm
						|| (equal_outputs && (next_state == sim_->getNextLatchValues())));
				if (!err_is_no_vulnerability)
				{
					int fi = next_free_cnf_var++;
					solver_->addVarToKeep(fi);
					int old_value = results[component_cnf];
					if (old_value == -1) // old value is true
						results[component_cnf] = -fi;
					else if (old_value == 1) // old value is false
						results[component_cnf] = fi;
					else
					{
						int new_value = next_free_cnf_var++;
						solver_->addVarToKeep(new_value);
						// new_value == fi ? -old_value : old_value
						solver_->incAdd3LitClause(fi, old_value, -new_value);
						solver_->incAdd3LitClause(fi, -old_value, new_value);
						solver_->incAdd3LitClause(-fi, old_value, new_value);
						solver_->incAdd3LitClause(-fi, -old_value, -new_value);
						results[component_cnf] = new_value;
					}

					// there might be at most one flip in one time-step:
					// if fi is true, all oter f must be false (fi -> -f1, fi -> -f2, ...)
					for (unsigned cnt = 0; cnt < f.size(); cnt++)
						solver_->incAdd2LitClause(-fi, -f[cnt]);

					f.push_back(fi);
				}
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// Symbolic simulation of AND gates
				for (unsigned b = 0; b < circuit_->num_ands; ++b)
				{

					int rhs1_cnf_value = Utils::readCnfValue(results, circuit_->ands[b].rhs1);
					int rhs0_cnf_value = Utils::readCnfValue(results, circuit_->ands[b].rhs0);

					if (rhs1_cnf_value == CNF_FALSE || rhs0_cnf_value == CNF_FALSE) // FALSE and .. = FALSE
						results[(circuit_->ands[b].lhs >> 1)] = CNF_FALSE;
					else if (rhs1_cnf_value == CNF_TRUE) // TRUE and X = X
						results[(circuit_->ands[b].lhs >> 1)] = rhs0_cnf_value;
					else if (rhs0_cnf_value == CNF_TRUE) // X and TRUE = X
						results[(circuit_->ands[b].lhs >> 1)] = rhs1_cnf_value;
					else if (rhs0_cnf_value == rhs1_cnf_value) // X and X = X
						results[(circuit_->ands[b].lhs >> 1)] = rhs1_cnf_value;
					else if (rhs0_cnf_value == -rhs1_cnf_value) // X and -X = FALSE
						results[(circuit_->ands[b].lhs >> 1)] = CNF_FALSE;
					else
					{
						int res = next_free_cnf_var++;
						// res == rhs1_cnf_value & rhs0_cnf_value:
						// Step 1: (rhs1_cnf_value == false) -> (res == false)
						solver_->incAdd2LitClause(rhs1_cnf_value, -res);
						// Step 2: (rhs0_cnf_value == false) -> (res == false)
						solver_->incAdd2LitClause(rhs0_cnf_value, -res);
						// Step 3: (rhs0_cnf_value == true && rhs1_cnf_value == true)
						//   -> (res == true)
						solver_->incAdd3LitClause(-rhs0_cnf_value, -rhs1_cnf_value, res);
						results[(circuit_->ands[b].lhs >> 1)] = res;

					}
				}
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// get Outputs and next state values, swich to next state
				int alarm_cnf_val = -Utils::readCnfValue(results,
						circuit_->outputs[circuit_->num_outputs - 1].lit);
				solver_->incAddUnitClause(alarm_cnf_val);

				vector<int> out_cnf_values;
				out_cnf_values.reserve(circuit_->num_outputs - 1);
				for (unsigned b = 0; b < circuit_->num_outputs - 1; ++b)
				{
					out_cnf_values.push_back(Utils::readCnfValue(results, circuit_->outputs[b].lit));
				}

				vector<int> next_state_cnf_values;
				next_state_cnf_values.reserve(circuit_->num_latches);
				for (unsigned b = 0; b < circuit_->num_latches; ++b)
				{
					int next_state_var = Utils::readCnfValue(results, circuit_->latches[b].next);
					next_state_cnf_values.push_back(next_state_var);
					if (abs(next_state_var) > 1)
						solver_->addVarToKeep(next_state_var);
				}

				for (unsigned b = 0; b < circuit_->num_latches; ++b)
				{
					results[(circuit_->latches[b].lit >> 1)] = next_state_cnf_values[b];
				}
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// clause saying that the outputs o and o' are different
				vector<int> o_is_diff_clause;
				o_is_diff_clause.reserve(out_cnf_values.size() + 1);
				for (unsigned cnt = 0; cnt < out_cnf_values.size(); ++cnt)
				{
					if (outputs[cnt] == AIG_TRUE) // simulation result of output is true
						o_is_diff_clause.push_back(-out_cnf_values[cnt]); // add negated output
					else
						o_is_diff_clause.push_back(out_cnf_values[cnt]);
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

				bool sat = solver_->incIsSat(odiff_enable_literals);

				// negate (=set to positive face) newest odiff_enable_literal to disable
				// the previous o_is_diff_clausefor the next iterations
				odiff_enable_literals.back() = -odiff_enable_literals.back();
				if (sat)
				{
					vulnerable_elements_.insert(component_aig);
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
					for(vector<int>::iterator it = f.begin(); it != f.end(); ++it)
					{
						core_assumptions.push_back(-*it);
					}
					vector<int> more_assumptions;
					more_assumptions.push_back(next_state_is_diff);
					vector<int> core;
					bool is_sat_2 = solver_->incIsSatModelOrCore(core_assumptions, more_assumptions, f,core);
					MASSERT(is_sat_2 == false, "must not be satisfiable")

					// TODO: not sure if there could be a more efficient way to do this
					// (e.g. under the assumption that results of core have same order as f):
					Utils::logPrint(core, "core: ");
					Utils::logPrint(f,"f: ");
					set<int> useless(f.begin(),f.end());
					for(vector<int>::iterator it = core.begin(); it != core.end(); ++it)
					{
						useless.erase(- *it);
					}

					for(set<int>::iterator it = useless.begin(); it != useless.end(); ++it)
					{
						solver_->incAddUnitClause(-*it);
					}

					int num_reduced_f_variables = f.size() - core.size();
					f.clear();
					for(vector<int>::iterator it = core.begin(); it != core.end(); ++it)
					{
						f.push_back(-*it);
					}
					L_LOG("step"<<i<<" reduced f variables: " << num_reduced_f_variables)
					// END TODO

				}

			} // -- END "for each timestep in testcase" --
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

}
