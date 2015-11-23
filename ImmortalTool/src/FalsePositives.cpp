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
/// @file FalsePositives.cpp
/// @brief Contains the definition of the class FalsePositives.
// -------------------------------------------------------------------------------------------


#include "FalsePositives.h"
#include "SatSolver.h"
#include "AigSimulator.h"
#include "SymbolicSimulator.h"
#include "Options.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
FalsePositives::FalsePositives(aiger* circuit, int num_err_latches)
{
  circuit_ = circuit;
  num_err_latches_ = num_err_latches;
  mode_ = 0;

}

// -------------------------------------------------------------------------------------------
FalsePositives::~FalsePositives()
{
}

bool FalsePositives::findFalsePositives_1b(vector<TestCase>& testcases)
{
	SatSolver* solver_ =  Options::instance().getSATSolver();
	AigSimulator* sim_ = new AigSimulator(circuit_);
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
			vector<int> concrete_state_ok;
			concrete_state_ok.resize(circuit_->num_latches);

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


			for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
			{ // -------- BEGIN "for each timestep in testcase" ------------------------------------

				//------------------------------------------------------------------------------------
				// Concrete simulations:
				// correct simulation
				sim_->simulateOneTimeStep(testcase[timestep], concrete_state_ok);
				vector<int> outputs_ok = sim_->getOutputs();
				bool alarm_ok = outputs_ok.back() == AIG_TRUE;
				if(alarm_ok)
				{
					cout << "Alarm raised without Error!" << endl;
					return true;
				}
				vector<int> next_state_ok = sim_->getNextLatchValues();

				// faulty simulation: flip component bit
				vector<int> faulty_state = concrete_state_ok;
				faulty_state[c_cnt] = (faulty_state[c_cnt] == AIG_TRUE) ? AIG_FALSE : AIG_TRUE;

				// faulty simulation with flipped bit
				sim_->simulateOneTimeStep(testcase[timestep], faulty_state);
				vector<int> outputs_faulty = sim_->getOutputs();
				vector<int> next_state_faulty = sim_->getNextLatchValues();
				bool alarm_faulty = outputs_faulty.back() == AIG_TRUE;


				// switch concrete simulation to next state
				concrete_state_ok = next_state_ok; // OR: change to sim_->switchToNextState();


				if (outputs_ok == outputs_faulty && next_state_ok == next_state_faulty && alarm_faulty)
				{
					// superfluous.push_back() ..... TODO
					// ...
					continue;
				}

				if (outputs_ok != outputs_faulty || !alarm_faulty)
				{
					// TODO
				}
				else
				{
					// fi etc
				}
				//------------------------------------------------------------------------------------



				//------------------------------------------------------------------------------------
				// call SAT-solver

				vector<int> model;
				bool sat = solver_->incIsSatModelOrCore(odiff_enable_literals, f, model);


				odiff_enable_literals.back() = -odiff_enable_literals.back();

				if (sat)
				{
//					vulnerable_elements_.insert(component_aig);
					break;
				}


			} // -- END "for each timestep in testcase" --
		} // end "for each testcase"
	} // ------ END 'for each latch' ---------------

	delete sim_;

	return false;
}
