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
#include "ErrorTraceManager.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------
BddAnalysis::BddAnalysis(aiger* circuit, int num_err_latches, int mode) :
		BackEnd(circuit, num_err_latches, mode)
{
}



bool BddAnalysis::analyze(vector<TestCase>& testcases)
{
	vulnerable_elements_.clear();

	Cudd cudd;
	cudd.AutodynEnable(CUDD_REORDER_SIFT);

	unsigned model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

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

	int first_cj_var = next_free_cnf_var;
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		latches_to_check_.insert(circuit_->latches[c_cnt].lit);

		int cj = next_free_cnf_var++;
		cudd.bddVar(cj);

		latch_to_cj[circuit_->latches[c_cnt].lit >> 1] = cj;
		cj_to_latch[cj] = circuit_->latches[c_cnt].lit;
	}
	int last_cj_var = next_free_cnf_var - 1;


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
		vector<BDD> f_as_bdd;
		map<int, unsigned> fi_to_timestep;

		// a set of cj-literals indicating whether *the _latch_ C_j is flipped* or not
		vector<int> cj_literals;
		map<int, int>::iterator map_iter;
		for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++)
		{
			cj_literals.push_back(map_iter->second);
		}
		map<int, BDD> cj_to_BDD_signal;


		BDD side_constraints = cudd.bddOne();

		//----------------------------------------------------------------------------------------
		// single fault assumption: there might be at most one flipped component
		// implemented as 1-hot-encoding
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


			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_.simulateOneTimeStep(testcase[timestep], concrete_state);
			vector<int> outputs_ok = sim_.getOutputs();
			vector<int> next_state = sim_.getNextLatchValues();

			// switch concrete simulation to next state
			concrete_state = next_state;
			//--------------------------------------------------------------------------------------

			// set input values according to TestCase to TRUE or FALSE:
			bddSim.setInputValues(testcase[timestep]);

			//--------------------------------------------------------------------------------------
			// cj indicates whether the corresponding latch is flipped
			// fi indicates whether the component is flipped in step i or not
			// there can only be a flip at time-step i if both cj and fi are true.

			int fi = next_free_cnf_var++;
			BDD fi_bdd = cudd.bddVar(fi);

			set<int>::iterator it;
			for (it = latches_to_check_.begin(); it != latches_to_check_.end(); ++it)
			{
				int latch_output = *it >> 1;

				BDD cj_bdd = cj_to_BDD_signal[latch_to_cj[latch_output]];
				BDD old_value = bddSim.getResultValue(latch_output);

				// todo: maybe use Cudd_addIte instead??
				BDD new_value = (fi_bdd & cj_bdd) ^ old_value; // flip old_value iff both fi and cj are true
				bddSim.setResultValue(latch_output, new_value);
			}

			//--------------------------------------------------------------------------------------
			// single fault assumption: there might be at most one flip in one time-step
			// if fi is true, all other f must be false
			for (unsigned cnt = 0; cnt < f_as_bdd.size(); cnt++)
			{
				side_constraints &= ~(fi_bdd & f_as_bdd[cnt]);
			}

			f.push_back(fi);
			f_as_bdd.push_back(fi_bdd);

			fi_to_timestep[fi] = timestep;
			//--------------------------------------------------------------------------------------

			// Symbolic simulation of AND gates
			bddSim.simulateOneTimeStep();

			side_constraints &= ~ bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			bddSim.switchToNextState();

			//--------------------------------------------------------------------------------------
			// constraints saying that the current outputs_ok o and o' are different
			BDD output_is_different_bdd = cudd.bddZero();
			for (unsigned out_idx = 0; out_idx < output_bdds.size(); ++out_idx)
			{
				if (outputs_ok[out_idx] == AIG_TRUE) // simulation result of output is true
					output_is_different_bdd |= ~ output_bdds[out_idx];
				else if (outputs_ok[out_idx] == AIG_FALSE)
					output_is_different_bdd |= output_bdds[out_idx];
			}
			//--------------------------------------------------------------------------------------


			//--------------------------------------------------------------------------------------
			// check satisfiability
			BDD check = side_constraints & output_is_different_bdd;
			while (!check.IsZero())
			{
				cout << "minterm ";
				check.PrintMinterm();
				// resize model size if necessary
				if(cudd.ReadSize() > model_memory_size)
				{
					while (cudd.ReadSize() > model_memory_size)
						model_memory_size *= 2;

					char* new_model = (char*) realloc(model, model_memory_size);
					if(new_model == NULL)
					{
						free(model);
						MASSERT(false, "out of memory")
					}
					model = new_model;
				}

				check.PickOneCube(model); // store model
				for (size_t j = 0; j < next_free_cnf_var; j++)
				{
					std::cout << "var " << j << " has value " << static_cast<int>(model[j]) << std::endl;
				}

				// find the one and only true c-signal in model
				int cj = 0;
				for(unsigned i = first_cj_var; i <= last_cj_var; i++)
				{
					if(model[i] == 1)
					{
						cj = i;
						break;
					}
				}

				cout << "cj = " << cj << endl;

				cj_to_BDD_signal[cj] = cudd.bddZero(); // free the memory
				// TODO: set cudd.bddVar(cj) to constant false/remove ??

				// find the one and only true f-signal from model
				int fi = 0;
				for (unsigned i = 0; i < f.size(); i++)
				{
					if(model[f[i]] == 1)
					{
						fi = f[i];
						break;
					}
				}
				cout << "fi = " << fi << endl;


				if (Options::instance().isUseDiagnosticOutput())
				{
					ErrorTrace* trace = new ErrorTrace;

					trace->error_timestep_ = timestep;
					trace->input_trace_ = testcase;
					trace->latch_index_ = cj_to_latch[cj];
					trace->flipped_timestep_ = fi_to_timestep[fi];

					ErrorTraceManager::instance().error_traces_.push_back(trace);
				}

				cout << "ts=" << timestep << ", latch = " <<  cj_to_latch[cj] << " fi = " << fi_to_timestep[fi] << endl;

				vulnerable_elements_.insert(cj_to_latch[cj]);
				side_constraints &= ~cudd.bddVar(cj);
				check = side_constraints & output_is_different_bdd;

			}

		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each testcase' ---------------

	free(model);

	return vulnerable_elements_.size() > 0;

}


// -------------------------------------------------------------------------------------------
BddAnalysis::~BddAnalysis()
{
}
