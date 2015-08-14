// ----------------------------------------------------------------------------
// Copyright (c) 2013-2014 by Graz University of Technology and
//                            Johannes Kepler University Linz
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

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SymbTimeAnalysis::SymbTimeAnalysis(aiger* circuit, int num_err_latches,
		int mode) :
		BackEnd(circuit, num_err_latches, mode), sim_(0)
{
	AIG2CNF::instance().initFromAig(circuit);
	sim_ = new AigSimulator(circuit_);
	solver_ = Options::instance().getSATSolver();
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
	//for each test case t[][]
	for (unsigned tc_index_ = 0; tc_index_ < testcases.size(); tc_index_++)
	{
		Analyze1(testcases[tc_index_]);
	}

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
bool SymbTimeAnalysis::findVulnerabilities(vector<string> paths_to_TC_files)
{
	//	vulnerable_latches = empty_set/list
	vulnerable_elements_.clear();
	//for each test case t[][]
	for (unsigned tc_index_ = 0; tc_index_ < paths_to_TC_files.size();
			tc_index_++)
	{
		TestCase testcase;
		Utils::parseAigSimFile(paths_to_TC_files[tc_index_], testcase,
				circuit_->num_inputs);
		Analyze1(testcase);
	}

	return (vulnerable_elements_.size() != 0);
}

// -------------------------------------------------------------------------------------------
void SymbTimeAnalysis::Analyze1(TestCase& testcase)
{

	cout << "trans orig = " << endl << AIG2CNF::instance().getTrans().toString() << endl;

// ---------------- BEGIN 'for each latch' -------------------------
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_;
			++c_cnt)
	{

		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = AIG2CNF::instance().aigLitToCnfLit(component_aig);
		// skip latches where we already know that they are vulnerable
		if (vulnerable_elements_.find(component_aig) != vulnerable_elements_.end())
		{
			continue;
		}

		int next_free_cnf_var = AIG2CNF::instance().getMaxCnfVar() + 1;

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
		CNF F; // = TRUE

		CNF T_err = AIG2CNF::instance().getTrans();

		int f_orig = next_free_cnf_var++;
		int poss_neg_state_cnf_var = next_free_cnf_var++;

		vector<int> first_rename_map;
		first_rename_map.reserve(next_free_cnf_var);
		for(int i = 0; i < next_free_cnf_var; ++i)
			first_rename_map.push_back(i);
		first_rename_map[component_cnf] = poss_neg_state_cnf_var;
		T_err.renameVars(first_rename_map);

		T_err.add3LitClause(-f_orig, -component_cnf, -poss_neg_state_cnf_var);
		T_err.add3LitClause(-f_orig, component_cnf, poss_neg_state_cnf_var);
		T_err.add3LitClause(f_orig, -component_cnf, poss_neg_state_cnf_var);
		T_err.add3LitClause(f_orig, component_cnf, -poss_neg_state_cnf_var);


		int max_cnf_var_in_Terr = next_free_cnf_var;

		for (unsigned i = 0; i < testcase.size(); i++)
		{ // -------- BEGIN "for each timestep in testcase" -----------

			// correct simulation
			sim_->simulateOneTimeStep(testcase[i], concrete_state);
			vector<int> outputs = sim_->getOutputs();
			vector<int> next_state = sim_->getNextLatchValues();

			// flip component bit
			vector<int> faulty_state = concrete_state;
			faulty_state[c_cnt] = (faulty_state[c_cnt] == 1) ? 0 : 1;

			// faulty simulation with flipped bit
			sim_->simulateOneTimeStep(testcase[i], faulty_state);
//	  vector<int> next_state2 = sim_->getNextLatchValues();  // unused
			vector<int> outputs2 = sim_->getOutputs();
			int alarm = outputs2[outputs2.size() - 1];

			// check if vulnerablitiy already found
//			if (outputs != outputs2 && alarm == 0)
//			{
//				cout << "BREAK Sim Latch " << component_aig << endl;
//				vulnerable_elements_.insert(component_aig);
//				break;
//			}

			vector<int> real_rename_map(max_cnf_var_in_Terr, 0);
			for(int i = 0; i < real_rename_map.size(); ++i)
				real_rename_map[i] = i;
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

			// rename: set outputs according to test case inputs
			// (ALTERNATIVE: don't rename, set via T_err_copy.setVarValue())
			for (unsigned cnt = 0; cnt < circuit_->num_inputs; ++cnt)
			{
				unsigned input_cnf = (circuit_->inputs[cnt].lit >> 1) + 1;
				int input_bit_value = AIG2CNF::instance().aigLitToCnfLit(
						testcase[i][cnt]);
				real_rename_map[input_cnf] = input_bit_value;
			}

			// indicates if the output of component is flipped (in step i)
			int fi = next_free_cnf_var++;
			f.push_back(fi);
			real_rename_map[f_orig] = fi;
			real_rename_map[poss_neg_state_cnf_var] = next_free_cnf_var++;

			Utils::debugPrint(real_rename_map, "Rename map:");

			//for()


			CNF T_err_copy = T_err;
			// set alarm to false in T_err_copy
			int alarm_cnf_lit = AIG2CNF::instance().getAlarmOutput();
			T_err_copy.setVarValue(alarm_cnf_lit, false);
			T_err_copy.renameVars(real_rename_map);



			F.addCNF(T_err_copy);

			// if fi is true, all oter f must be false
			for (unsigned cnt = 0; cnt < f.size() - 1; cnt++)
				F.add2LitClause(-fi, -f[cnt]);


			// rename each output except alarm output
			vector<int> renamed_out_vars;
			renamed_out_vars.reserve(circuit_->num_outputs - 1);
			for (unsigned cnt = 0; cnt < circuit_->num_outputs - 1; ++cnt)
			{
				int renamed_var = AIG2CNF::instance().aigLitToCnfLit(
						circuit_->outputs[cnt].lit);
				renamed_var =
						(renamed_var > 0) ?
								real_rename_map[renamed_var] : -real_rename_map[-renamed_var];
				renamed_out_vars.push_back(renamed_var);
			}

			// clause saying that the outputs o and o' are different
			vector<int> o_is_diff_clause;
			o_is_diff_clause.reserve(renamed_out_vars.size());
			for (unsigned cnt = 0; cnt < renamed_out_vars.size(); ++cnt)
			{
				if (outputs[cnt] == 1)
					o_is_diff_clause.push_back(-renamed_out_vars[cnt]);
				else
					o_is_diff_clause.push_back(renamed_out_vars[cnt]);
			}

			// build sat-solver clause. TODO: maybe incremental mode?
			CNF F_for_solver = F;
			F_for_solver.addClause(o_is_diff_clause);

			// call SAT-Solver
			bool sat = solver_->isSat(F_for_solver);
			cout << "SAT: "<< sat << endl;
			if (sat)
			{
				vector<int> sat_assignment;
				solver_->isSatModelOrCore(F_for_solver, vector<int>(), F_for_solver.getVars() , sat_assignment);
				Utils::debugPrint(sat_assignment, "Satisfying assignment:");

				cout << F_for_solver.toString() << endl;

				vulnerable_elements_.insert(component_aig);
				break;
			}

			concrete_state = next_state;

			// rename next states
			vector<int> renamed_next_state_vars;
			for (unsigned cnt = 0; cnt < circuit_->num_latches; ++cnt)
			{
				int renamed_var = AIG2CNF::instance().aigLitToCnfLit(
						circuit_->latches[cnt].next);
				renamed_var =
						(renamed_var > 0) ?
								real_rename_map[renamed_var] : -real_rename_map[-renamed_var];
				renamed_next_state_vars.push_back(renamed_var);
			}

			symb_state = renamed_next_state_vars;

		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each latch' ---------------
}
