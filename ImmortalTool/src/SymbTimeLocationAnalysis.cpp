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
SymbTimeLocationAnalysis::SymbTimeLocationAnalysis(aiger* circuit,
		int num_err_latches, int mode) :
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
bool SymbTimeLocationAnalysis::findVulnerabilities(
		vector<string> paths_to_TC_files)
{
	vector<TestCase> testcases;
	//for each test case t[][]
	for (unsigned tc_index_ = 0; tc_index_ < paths_to_TC_files.size();
			tc_index_++)
	{
		TestCase testcase;
		Utils::parseAigSimFile(paths_to_TC_files[tc_index_], testcase,
				circuit_->num_inputs);
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

	// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_;
			++c_cnt)
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
				results[(circuit_->latches[l].lit >> 1)] = 1;
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
				faulty_state[c_cnt] = (faulty_state[c_cnt] == 1) ? 0 : 1;

				// faulty simulation with flipped bit
				sim_->simulateOneTimeStep(testcase[i], faulty_state);
				vector<int> outputs2 = sim_->getOutputs();

				bool alarm = (outputs2[outputs2.size() - 1] == 1);
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
					results[(circuit_->inputs[cnt_i].lit >> 1)] =
							(testcase[i][cnt_i] == 1) ? -1 : 1;
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// fi is a variable that indicates whether the component is flipped in step i or not
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
				//------------------------------------------------------------------------------------

				//------------------------------------------------------------------------------------
				// Symbolic simulation of AND gates
				for (unsigned b = 0; b < circuit_->num_ands; ++b)
				{

					int rhs1_cnf_value = Utils::readCnfValue(results,
							circuit_->ands[b].rhs1);
					int rhs0_cnf_value = Utils::readCnfValue(results,
							circuit_->ands[b].rhs0);

					if (rhs1_cnf_value == 1 || rhs0_cnf_value == 1) // FALSE and .. = FALSE
						results[(circuit_->ands[b].lhs >> 1)] = 1;
					else if (rhs1_cnf_value == -1) // TRUE and X = X
						results[(circuit_->ands[b].lhs >> 1)] = rhs0_cnf_value;
					else if (rhs0_cnf_value == -1) // X and TRUE = X
						results[(circuit_->ands[b].lhs >> 1)] = rhs1_cnf_value;
					else if (rhs0_cnf_value == rhs1_cnf_value) // X and X = X
						results[(circuit_->ands[b].lhs >> 1)] = rhs1_cnf_value;
					else if (rhs0_cnf_value == -rhs1_cnf_value) // X and -X = FALSE
						results[(circuit_->ands[b].lhs >> 1)] = 1;
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
					out_cnf_values.push_back(
							Utils::readCnfValue(results, circuit_->outputs[b].lit));
				}

				vector<int> next_state_cnf_values;
				next_state_cnf_values.reserve(circuit_->num_latches);
				for (unsigned b = 0; b < circuit_->num_latches; ++b)
				{
					int next_state_var = Utils::readCnfValue(results,
							circuit_->latches[b].next);
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
					if (outputs[cnt] == 1) // simulation result of output is true
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
					if (next_state[cnt] == 1) // simulation result of output is true
						lit_to_add = -next_state_cnf_values[cnt]; // add negated output
					else
						lit_to_add = next_state_cnf_values[cnt];
					if (lit_to_add != 1)
						next_state_is_diff_clause.push_back(lit_to_add);
				}
				if (next_state_is_diff_clause.empty())
				{
					vars_to_keep.clear();
					vars_to_keep.push_back(1); // TRUE and FALSE literals
					solver_->startIncrementalSession(vars_to_keep, 0);
					solver_->incAddUnitClause(-1); // -1 = TRUE constant
					next_free_cnf_var = 2;
					f.clear();
					odiff_enable_literals.clear();
				}
				else
				{
					if (next_state_is_diff_clause.size() == 1)
					{
						for (unsigned cnt = 0; cnt < f.size(); cnt++)
							solver_->incAdd2LitClause(-f[cnt], next_state_is_diff_clause[0]);
					}
					else
					{
						next_state_is_diff_clause.push_back(-next_state_is_diff);
						solver_->incAddClause(next_state_is_diff_clause);
						for (unsigned cnt = 0; cnt < f.size(); cnt++)
							solver_->incAdd2LitClause(-f[cnt], next_state_is_diff);
					}
				}
				//------------------------------------------------------------------------------------

			} // -- END "for each timestep in testcase" --
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

}
