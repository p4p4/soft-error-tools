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

#include "AigSimulator.h"
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
	//  for each latch l
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_;
			++c_cnt)
	{ // ---- BEGIN 'for each latch' ------
		unsigned component_aig = circuit_->latches[c_cnt].lit;
		int component_cnf = AIG2CNF::instance().aigLitToCnfLit(component_aig);

		// skip latches where we already know that they are vulnerable
		if (vulnerable_elements_.find(component_aig) != vulnerable_elements_.end())
		{
			continue;
		}

//		concrete_state[] = (0 0 0 0 0 0 0)   // AIG literals
		vector<int> concrete_state;
		concrete_state.resize(circuit_->num_latches); // TODO: for all or just for the latches to protect?

//		symb_state[] = (-1 -1 -1 -1 -1)  // CNF literals
		vector<int> symb_state;
		symb_state.reserve(circuit_->num_latches);
		for (unsigned i = 0; i < circuit_->num_latches; i++)
		{
			symb_state.push_back(-1);
		}

		vector<int> f;
		CNF F;
		CNF T_err = AIG2CNF::instance().getTrans();

//		int next_free_cnf_var = aig2cnf(max_aiger_var) + 1;
		int next_free_cnf_var = AIG2CNF::instance().getMaxCnfVar() + 1;
		int f_orig = next_free_cnf_var++;
		int poss_neg_state_cnf_var = next_free_cnf_var++;

		T_err.add3LitClause(-f_orig, -component_cnf, -poss_neg_state_cnf_var);
		T_err.add3LitClause(-f_orig, component_cnf, poss_neg_state_cnf_var);
		T_err.add3LitClause(f_orig, -component_cnf, poss_neg_state_cnf_var);
		T_err.add3LitClause(f_orig, component_cnf, -poss_neg_state_cnf_var);
//
		vector<int> orig_rename_map; // TODO: move outside of loop
		for (int i = 0; i < next_free_cnf_var; i++)
			orig_rename_map[i] = i;

		orig_rename_map[component_cnf] = poss_neg_state_cnf_var;
//
//		for(int i = 0; i < test_case.size(); i++)
//		{
		for (unsigned i = 0; i < testcase.size(); i++)
		{
//		  next_state[], outputs[], alarm = sim(concrete_state, test_case[i]);
			sim_->simulateOneTimeStep(testcase[i], concrete_state);
			vector<int> outputs = sim_->getOutputs();
			vector<int> next_state = sim_->getNextLatchValues();

//		  vector<int> faulty_state = concrete_state;
			vector<int> faulty_state = concrete_state;
//		  faulty_state[C] = negate(faulty_state[C]);
			faulty_state[component_cnf] = (faulty_state[component_cnf] == 1) ? 0 : 1;

//		  next_state2[], outputs2[], alarm2 = sim(faulty_state, test_case[i]);
			sim_->simulateOneTimeStep(testcase[i], faulty_state);
			vector<int> next_state2 = sim_->getNextLatchValues();
			vector<int> outputs2 = sim_->getOutputs();
			int alarm = outputs2[outputs2.size() - 1];
//
//		  if(outputs != outputs2 && alarm2 == 0)
//		  {
//		     vulnerable_latches.push_back(C);
//		     return true;
//		  }
			if (outputs != outputs2 && alarm == 0)
			{
				vulnerable_elements_.insert(component_aig);
				break;
			}
//
//		  vector<int> real_rename_map = orig_rename_map;
			vector<int> real_rename_map = orig_rename_map; // TODO: do we really need a copy?
//		  for each AND gate a from the aiger thingi:
//		    real_rename_map[aig2cnf(a.lhs)] = next_free_cnf_var++;
			for (unsigned cnt = 0; cnt < circuit_->num_ands; ++cnt)
			{
				unsigned and_cnf = circuit_->ands[cnt].lhs >> 1;
				real_rename_map[and_cnf] = next_free_cnf_var++;
			}

//		  int cnt = 0;
//		  for each latch l from the aiger thingi:
//		    real_rename_map[aig2cnf(l.lit)] = symb_state[cnt++]
			for (unsigned cnt = 0; cnt < circuit_->num_latches; ++cnt) // TODO: for all?
			{
				unsigned latch_cnf = circuit_->latches[cnt].lit >> 1;
				real_rename_map[latch_cnf] = symb_state[cnt];
			}
//		  int cnt = 0;
//		  for each input in from the aiger thingi:
//		    real_rename_map[aig2cnf(in.lit)] = aig2cnf(test_case[i][cnt++])
			for (unsigned cnt = 0; cnt < circuit_->num_inputs; ++cnt) // TODO: for all?
			{
				unsigned input_cnf = circuit_->inputs[cnt].lit >> 1;
				real_rename_map[input_cnf] = testcase[i][cnt];
			}

			int fi = next_free_cnf_var++;
			f.push_back(fi);
			real_rename_map[f_orig] = fi;

			CNF T_err_copy = T_err;
			T_err_copy.renameVars(real_rename_map);
//		  T_err_copy.setVarValue(aig2CNF(last_output), false);
			T_err_copy.setVarValue(AIG2CNF::instance().getAlarmOutput(), false);

//		  F.addCNF(T_err_copy);
			F.addCNF(T_err_copy);

//		  for(int a = 0; a < f.size() - 1; ++a)
//		    F.add2LitClause(-fi, -f[a]);
			for (unsigned cnt = 0; cnt < f.size() - 1; cnt++)
				F.add2LitClause(-fi, -f[cnt]);

			vector<int> renamed_out_vars;
//		  for each output o (without alarm) from the aiger thingi:
//		    int renamed_var = aig2cnf(o.lit) > 0 ?
//		        real_rename_map[aig2cnf(o.lit)] :
//		        -real_rename_map[-aig2cnf(o.lit)]
//		    renamed_out_vars.push_back(renamed_var);
			for (unsigned cnt = 0; cnt < circuit_->num_outputs; ++cnt) // TODO: for all?
			{
				int renamed_var = AIG2CNF::instance().aigLitToCnfLit(
						circuit_->outputs[cnt].lit);
				renamed_var =
						(renamed_var > 0) ?
								real_rename_map[renamed_var] : -real_rename_map[-renamed_var];
				renamed_out_vars.push_back(renamed_var);
			}
//
			vector<int> o_is_diff_clause;
			for (unsigned cnt = 0; cnt < renamed_out_vars.size(); ++cnt)
			{
				if (outputs[cnt] == 1)
					o_is_diff_clause.push_back(-renamed_out_vars[cnt]);
				else
					o_is_diff_clause.push_back(renamed_out_vars[cnt]);
			}

			CNF F_for_solver = F;
			F_for_solver.addClause(o_is_diff_clause);
//
//		  bool sat = solver.isSat(F_for_solver);
//		  if(sat)
//		  {
//		     vulnerable_latches.push_back(C);
//		     return true;
//		  }
			bool sat = solver_->isSat(F_for_solver);
			if (sat)
			{
				vulnerable_elements_.insert(component_aig);
				break;
			}

			concrete_state = next_state;

			vector<int> renamed_next_state_vars;
//		  for each latch l from the aiger thingi:
//		    int renamed_var = aig2cnf(l.next) > 0 ?
//		        real_rename_map[aig2cnf(l.next)] :
//		        -real_rename_map[-aig2cnf(l.next)]
//		    renamed_next_state_vars.push_back(renamed_var);
			for (unsigned cnt = 0; cnt < circuit_->num_latches; ++cnt)
			{
				int renamed_var = AIG2CNF::instance().aigLitToCnfLit(
						circuit_->latches[cnt].lit);
				renamed_var =
						(renamed_var > 0) ?
								real_rename_map[renamed_var] : -real_rename_map[-renamed_var];
				renamed_next_state_vars.push_back(renamed_var);
			}
//
		  symb_state = renamed_next_state_vars;

		} // -- END "for each timestep in testcase" ---
	} // ------ END 'for each latch' ------
}
