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
/// @file SymbTimeLocationAnalysis.cpp
/// @brief Contains the definition of the class SymbTimeLocationAnalysis.
// -------------------------------------------------------------------------------------------

#include "SymbTimeLocationAnalysis.h"
#include "Utils.h"
#include "AIG2CNF.h"
#include "Options.h"
#include "Logger.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SymbTimeLocationAnalysis::SymbTimeLocationAnalysis(aiger* circuit, int num_err_latches,
		int mode) :
		BackEnd(circuit, num_err_latches, mode)
{
	AIG2CNF::instance().initFromAig(circuit);
	sim_ = new AigSimulator(circuit_);
	solver_ = Options::instance().getSATSolver();
}

// -------------------------------------------------------------------------------------------
SymbTimeLocationAnalysis::~SymbTimeLocationAnalysis()
{
	delete sim_;
	delete solver_;
}

// -------------------------------------------------------------------------------------------
bool SymbTimeLocationAnalysis::findVulnerabilities(vector<TestCase> &testcases)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();

	if (mode_ == STANDARD)
		Analyze2(testcases);
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
	// used to store the results of the symbolic simulation
	vector<int> results;
	results.resize(circuit_->maxvar + 1);
	results[0] = 1; // FALSE and TRUE constants
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

	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{

		// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
		vector<int> concrete_state;
		concrete_state.resize(circuit_->num_latches);

		// f = a set of variables fi indicating whether the latch is *flipped in _step_ i* or not
		vector<int> f;

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

		//----------------------------------------------------------------------------------------
		for (unsigned l = 0; l < circuit_->num_latches; ++l) // initialize latches to false
			results[(circuit_->latches[l].lit >> 1)] = CNF_FALSE;
		//----------------------------------------------------------------------------------------

		TestCase& testcase = testcases[tc_number];
		for (unsigned i = 0; i < testcase.size(); i++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_->simulateOneTimeStep(testcase[i], concrete_state);
			vector<int> outputs = sim_->getOutputs();
			vector<int> next_state = sim_->getNextLatchValues();

			// switch concrete simulation to next state
			concrete_state = next_state; // OR: change to sim_->switchToNextState();
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
			// set input values according to TestCase to TRUE or FALSE:
			for (unsigned cnt_i = 0; cnt_i < circuit_->num_inputs; ++cnt_i)
				results[(circuit_->inputs[cnt_i].lit >> 1)] =
						(testcase[i][cnt_i] == AIG_TRUE) ? CNF_TRUE : CNF_FALSE;
			//--------------------------------------------------------------------------------------

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
				int old_value = results[latch_output];
				int new_value = next_free_cnf_var++;
				int ci_lit = latch_to_cj[latch_output];

				//solver_->addVarToKeep(new_value);

				solver_->incAdd3LitClause(old_value, ci_lit, -new_value);
				solver_->incAdd3LitClause(old_value, fi, -new_value);
				solver_->incAdd4LitClause(-old_value, -ci_lit, -fi, -new_value);
				solver_->incAdd3LitClause(-old_value, ci_lit, new_value);
				solver_->incAdd3LitClause(-old_value, fi, new_value);
				solver_->incAdd4LitClause(old_value, -ci_lit, -fi, new_value);

				results[latch_output] = new_value;
			}

			//--------------------------------------------------------------------------------------
			// single fault assumption: there might be at most one flip in one time-step
			// if fi is true, all oter f must be false (fi -> -f1, fi -> -f2, ...)
			for (unsigned cnt = 0; cnt < f.size(); cnt++)
				solver_->incAdd2LitClause(-fi, -f[cnt]);

			f.push_back(fi);
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
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
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
			// Symbolic Simulation: get Outputs and next state values, switch to next state
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
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
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
			//--------------------------------------------------------------------------------------

			//--------------------------------------------------------------------------------------
			// call SAT-solver
			vector<int> model;
			while (solver_->incIsSatModelOrCore(odiff_enable_literals, cj_literals, model))
			{
//				Utils::debugPrint(model, "sat assignment: ");
				vector<int>::iterator model_iter;
				for (model_iter = model.begin(); model_iter != model.end(); ++model_iter)
				{
					int cj_lit = *model_iter;
					if (cj_lit > 0) // we have found the one and only cj signal which was active
					{
						// Add vulnerable latch (represented by cj) to list of vulnerabilities
						// Add blocking clause so that the sat-solver can report other vulnerabilities
						vulnerable_elements_.insert(cj_to_latch[cj_lit]);
						latches_to_check_.erase(cj_to_latch[cj_lit]);
						solver_->incAddUnitClause(-cj_lit); // blocking clause
						break;
					}
				}
			}

			// negate (=set to positive face) newest odiff_enable_literal to disable
			// the previous o_is_diff_clausefor the next iterations
			odiff_enable_literals.back() = -odiff_enable_literals.back();

			//--------------------------------------------------------------------------------------
			// Optimization: next state does not change,no matter if we flip or not -> remove fi's

			int next_state_is_diff = next_free_cnf_var++;
			vector<int> next_state_is_diff_clause;
			next_state_is_diff_clause.reserve(next_state_cnf_values.size() + 1);
			for (size_t cnt = 0; cnt < next_state_cnf_values.size(); ++cnt)
			{
				int lit_to_add = 0;
				if (next_state[cnt] == 1) // simulation result of output is true
					lit_to_add = -next_state_cnf_values[cnt]; // add negated output
				else
					lit_to_add = next_state_cnf_values[cnt];
				if (lit_to_add != 1)
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
				for (l1_it = latches_to_check_.begin(); l1_it != latches_to_check_.end(); l1_it++)
				{
					l2_it = l1_it;
					l2_it++;
					for (; l2_it != latches_to_check_.end(); l2_it++)
						solver_->incAdd2LitClause(-latch_to_cj[*l1_it >> 1], -latch_to_cj[*l2_it >> 1]);
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
			// 0 = disabled, 1 = every iteration, 2 = every 2nd iteration, ...
			unsigned interval_of_iterations_to_compute_ = 10;


			if (f.size() > 1 && (interval_of_iterations_to_compute_ != 0)
					&& (f.size() % interval_of_iterations_to_compute_ == 0))
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
	} // ------ END 'for each latch' ---------------

}
