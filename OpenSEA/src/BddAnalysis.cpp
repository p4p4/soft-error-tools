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
/// @file BddAnalysis.cpp
/// @brief Contains the definition of the class BddAnalysis.
// -------------------------------------------------------------------------------------------

#include "BddAnalysis.h"

#include "AigSimulator.h"
#include "BddSimulator.h"
extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
BddAnalysis::BddAnalysis(aiger* circuit, int num_err_latches, int mode) :
		BackEnd(circuit, num_err_latches, mode)
{


}



bool BddAnalysis::analyze(vector<TestCase>& testcases)
{

	Cudd cudd;
	cudd.AutodynEnable(CUDD_REORDER_SIFT);

	AigSimulator sim_(circuit_);

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
		cudd.bddVar(cj);

		latch_to_cj[circuit_->latches[c_cnt].lit >> 1] = cj;
		cj_to_latch[cj] = circuit_->latches[c_cnt].lit;
	}




	int next_cnf_var_after_ci_vars = next_free_cnf_var;
	//------------------------------------------------------------------------------------------
	BddSimulator bddSim(circuit_, cudd, next_free_cnf_var);


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
		map<int, BDD> cj_to_BDD_signal;



		// start new incremental BDD session
		// TODO
		BDD side_constraints = cudd.bddOne();

		//----------------------------------------------------------------------------------------
		// single fault assumption: there might be at most one flipped component
		map<int, int>::iterator map_iter2;
		for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++) // for each latch:
		{
			BDD real_c_signal = cudd.bddOne();
			for (map_iter2 = latch_to_cj.begin(); map_iter2 != latch_to_cj.end(); map_iter2++) // for current latch, go over all latches
			{
				if (map_iter == map_iter2) // the one and only cj-signal which can be true for this signal
					real_c_signal &= cudd.bddVar(map_iter2->second);
				else
					real_c_signal &= ~ cudd.bddVar(map_iter2->second);
			}
			cj_to_BDD_signal[map_iter->second] = real_c_signal;
		}

		bddSim.initLatches();

		TestCase& testcase = testcases[tc_number];



		for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

			// ;;;;;;;
			/*
			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_->simulateOneTimeStep(testcase[timestep], concrete_state);
			vector<int> outputs_ok = sim_->getOutputs();
			vector<int> next_state = sim_->getNextLatchValues();

			// switch concrete simulation to next state
			concrete_state = next_state;
			//--------------------------------------------------------------------------------------

			// set input values according to TestCase to TRUE or FALSE:
			symbsim.setInputValues(testcase[timestep]);

			//--------------------------------------------------------------------------------------
			// cj is a variable that indicatest whether the corresponding latch is flipped
			// fi is a variable that indicates whether the component is flipped in step i or not
			// there can only be a flip at timestep i if both cj and fi are true.
			int fi = next_free_cnf_var++;

			set<int>::iterator it;
			for (it = latches_to_check_.begin(); it != latches_to_check_.end(); ++it)
			{
				int latch_output = *it >> 1;
				int old_value = symbsim.getResultValue(latch_output);
				int new_value = next_free_cnf_var++;
				int ci_lit = latch_to_cj[latch_output];


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


			//--------------------------------------------------------------------------------------
			// clause saying that the outputs_ok o and o' are different
			vector<int> o_is_diff_clause;
			o_is_diff_clause.reserve(out_cnf_values.size() + 1);
			for (unsigned out_idx = 0; out_idx < out_cnf_values.size(); ++out_idx)
			{

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

*/
		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each testcase' ---------------

}


// -------------------------------------------------------------------------------------------
BddAnalysis::~BddAnalysis()
{
}
