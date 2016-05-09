// ----------------------------------------------------------------------------
// Copyright (c) 2016 by Graz University of Technology
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
/// @file DefinitelyProtected.cpp
/// @brief Contains the definition of the class DefinitelyProtected.
// -------------------------------------------------------------------------------------------

#include "DefinitelyProtected.h"
#include "SatSolver.h"
#include "AigSimulator.h"
#include "SymbolicSimulator.h"
#include "Options.h"
#include "Utils.h"
#include "Logger.h"
#include "AndCacheMap.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
DefinitelyProtected::DefinitelyProtected(aiger* circuit, int num_err_latches, int mode) :
				BackEnd(circuit, num_err_latches, mode)
{
	circuit_ = circuit;
	num_err_latches_ = num_err_latches;
	mode_ = mode;

}

// -------------------------------------------------------------------------------------------
DefinitelyProtected::~DefinitelyProtected()
{
}

bool DefinitelyProtected::analyze(vector<TestCase>& testcases)
{

	// TODO: 	remove testcases parameter, we don't need them here. maybe wrong backend?!
	//			think about architectural changes in OpenSEA ...

	if (mode_ == DefinitelyProtected::STANDARD)	// TODO: split into meaningful BackEnd groups
		findDefinitelyProtected_1();
	else
		MASSERT(false, "unknown mode!");

	return false; // TODO
}

bool DefinitelyProtected::findDefinitelyProtected_1()
{
	cout << "hello!" << endl;

	vector<int> definitely_protected_latches;

	// ---------------- BEGIN 'for each latch' -------------------------
	// let's assume we know which latch is not part of the protection logic
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		unsigned component_cnf = circuit_->latches[c_cnt].lit >> 1;
		int next_free_cnf_var = 2;
		SatSolver* solver_ = Options::instance().getSATSolver();
		vector<int> vars_to_keep; // empty
		solver_->startIncrementalSession(vars_to_keep);
		SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);


		// variables for the state
		for(unsigned i = 0; i < circuit_->num_latches; i++)
			sim_symb.setResultValue(circuit_->latches[i].lit/2, next_free_cnf_var++);


		// variables for the inputs
		for(unsigned i = 0; i < circuit_->num_inputs; i++)
			sim_symb.setResultValue(circuit_->inputs[i].lit/2, next_free_cnf_var++);

		// compute transition relation T(x,i,o,a,x')
		sim_symb.simulateOneTimeStep();

		// additional time steps
		AndCacheMap cache(solver_);
		unsigned num_steps = 1;
		for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
		{
			if (s_cnt == num_steps - 1) // last iteration
				sim_symb.setCache(&cache);
			sim_symb.switchToNextState();
			for(unsigned i = 0; i < circuit_->num_inputs; i++)
				sim_symb.setResultValue(circuit_->inputs[i].lit/2, next_free_cnf_var++);

			sim_symb.simulateOneTimeStep();
		}

		vector<int> next_state_normal = sim_symb.getNextLatchValues();
		vector<int> outpus_normal = sim_symb.getOutputValues();


		// compute faulty transition relation

		sim_symb.setResultValue(component_cnf, -sim_symb.getResultValue(component_cnf)); // flip latch
		sim_symb.simulateOneTimeStep();

		vector<int> next_state_flip = sim_symb.getNextLatchValues();
		vector<int> outpus_flip = sim_symb.getOutputValues();

		// we only care about situations where the alarm is false
		solver_->incAddUnitClause(-sim_symb.getAlarmValue()); // no alarm


		// create clauses saying that
		//		(next_state_normal != next_state_flip) OR (outpus_normal != outpus_flip)
		vector<int> output_or_next_state_different_clause;

		for(unsigned i = 0; i < circuit_->num_outputs -1; i++)
		{
			int out_is_diff_enable = next_free_cnf_var++;
			output_or_next_state_different_clause.push_back(-out_is_diff_enable);
			solver_->incAdd3LitClause(out_is_diff_enable, outpus_normal[i], outpus_flip[i]);
			solver_->incAdd3LitClause(out_is_diff_enable, -outpus_normal[i], -outpus_flip[i]);
		}

		for(unsigned i = 0; i < circuit_->num_latches - num_err_latches_; i++)
		{
			int state_is_diff_enable = next_free_cnf_var++;
			output_or_next_state_different_clause.push_back(-state_is_diff_enable);
			solver_->incAdd3LitClause(state_is_diff_enable, next_state_normal[i], next_state_flip[i]);
			solver_->incAdd3LitClause(state_is_diff_enable, -next_state_normal[i], -next_state_flip[i]);
		}

		solver_->incAddClause(output_or_next_state_different_clause);

		if(solver_->incIsSat() == false)
		{
			cout << "Definitely protected latch "<< circuit_->latches[c_cnt].lit << " found. (UNSAT)" << endl;
			definitely_protected_latches.push_back(circuit_->latches[c_cnt].lit);
		}
		else
		{
			cout << "SAT.." << endl;
		}


	}



	return false; // TODO
}

