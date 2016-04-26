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

	// TODO: 	remove testcases, we don't need them here. maybe wrong backend?!
	//			think about architectural changes in OpenSEA ...

	if (mode_ == DefinitelyProtected::STANDARD)	// TODO: split into meaningful BackEnd groups
		findDefinitelyProtected_1();
	else
		MASSERT(false, "unknown mode!");

	return false; // TODO
}

bool DefinitelyProtected::findDefinitelyProtected_1()
{

	vector<int> definitely_protected_latches;

	// ---------------- BEGIN 'for each latch' -------------------------
	// let's assume we know which latch is not part of the protection logic
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		int next_free_cnf_var = 2;
		SatSolver* solver_ = Options::instance().getSATSolver();
		SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);


		// variables for the state
		vector<int> states;
		states.reserve(circuit_->num_latches);
		for(unsigned i = 0; i < circuit_->num_latches; i++)
		{
			int state_current = next_free_cnf_var++;
			states.push_back(state_current);
			sim_symb.setResultValue(circuit_->latches[i].lit, state_current);
		}

		// variables for the inputs
		vector<int> inputs;
		inputs.reserve(circuit_->num_inputs);
		for(unsigned i = 0; i < circuit_->num_inputs; i++)
		{
			int input_current = next_free_cnf_var++;
			inputs.push_back(input_current);
			sim_symb.setResultValue(circuit_->inputs[i].lit, input_current);
		}

		// compute transition relation T(x,i,o,a,x')
		sim_symb.simulateOneTimeStep();

		vector<int> next_state_normal = sim_symb.getNextLatchValues();
		vector<int> outpus_normal = sim_symb.getOutputValues();


		// compute faulty transition relation
		states[c_cnt] = - states[c_cnt]; // flip
		sim_symb.simulateOneTimeStep(inputs,states);

		vector<int> next_state_flip = sim_symb.getNextLatchValues();
		vector<int> outpus_flip = sim_symb.getOutputValues();

		// we only care about situations where the alarm is false
		solver_->incAddUnitClause(-sim_symb.getAlarmValue()); // no alarm


		//TODO:	create clause(s) saying that
		//		(next_state_normal != next_state_flip) OR (outpus_normal != outpus_flip)
		//		and add it to the solver

		if(solver_->incIsSat() == false)
		{
			definitely_protected_latches.push_back(circuit_->latches[c_cnt].lit);
		}


	}



	return false; // TODO
}

