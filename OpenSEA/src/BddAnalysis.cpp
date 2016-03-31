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

#include <math.h> // ceil, log2

// -------------------------------------------------------------------------------------------
BddAnalysis::BddAnalysis(aiger* circuit, int num_err_latches, int mode) :
		BackEnd(circuit, num_err_latches, mode)
{
}



bool BddAnalysis::analyze(vector<TestCase>& testcases)
{
	PointInTime begin = Stopwatch::start();
	accumulated_durations_.clear();
	vulnerable_elements_.clear();


	// TODO modes...
	analyze_one_hot_enc_sig(testcases);
//	analyze_one_hot_enc_constr(testcases);
//	analyze_binary_enc_sig(testcases);
	// ...


	printStatistics(begin);
	return vulnerable_elements_.size() > 0;

}

void BddAnalysis::analyze_one_hot_enc_sig(vector<TestCase>& testcases)
{
	Cudd cudd;
	//cudd.AutodynEnable(CUDD_REORDER_SIFT);  // it is better to turn off automatic reordering

	int model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

	AigSimulator sim_(circuit_);

	// used to store the results of the symbolic simulation
	int next_free_cnf_var = 2;

	set<int> latches_to_check_;

	//------------------------------------------------------------------------------------------
	// set up ci signals
	// maps for latch-literals <=> cj-literals: each latch has a corresponding cj literal,
	// which indicates whether the latch is flipped or not.
	stopWatchStart();
	map<int, int> latch_to_cj; // maps latch-literals(cnf) to corresponding cj-literals(cnf)
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)
	map<int, BDD> cj_to_BDD_signal; // maps a cj input literal to the actual cj BDD with cardinality constraints

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

	// single fault assumption: there might be at most one flipped component
	map<int, int>::iterator map_iter;
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
	stopWatchStore(CREATE_C_SIGNALS);



	//------------------------------------------------------------------------------------------
	BddSimulator bddSim(circuit_, cudd, next_free_cnf_var);


	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{

		// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
		vector<int> concrete_state;
		concrete_state.resize(circuit_->num_latches);

		// f = a set of variables fi indicating whether the latch is *flipped in _step_ i* or not
		vector<int> f_inputs;
		vector<BDD> f_prime_bdd;
		map<int, unsigned> fi_to_timestep;

		BDD side_constraints = cudd.bddOne();

		stopWatchStart();
		bddSim.initLatches();
		stopWatchStore(INIT_Latches);

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

			stopWatchStart();
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
			stopWatchStore(MODIFY_LATCHES);

			//--------------------------------------------------------------------------------------
			// single fault assumption: there might be at most one flip in one time-step
			// if fi is true, all other f must be false
			stopWatchStart();
			for (unsigned cnt = 0; cnt < f_prime_bdd.size(); cnt++)
			{
				side_constraints &= ~(fi_bdd & f_prime_bdd[cnt]);
			}
			stopWatchStore(SIDE_CONSTRAINTS);

			f_inputs.push_back(fi);
			f_prime_bdd.push_back(fi_bdd);

			fi_to_timestep[fi] = timestep;
			//--------------------------------------------------------------------------------------

			// Symbolic simulation of AND gates

			stopWatchStart();
			bddSim.simulateOneTimeStep();
			stopWatchStore(SIM_ANDs);
			side_constraints &= ~ bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			stopWatchStart();
			bddSim.switchToNextState();
			stopWatchStore(SWITCH_NXT_ST);


			stopWatchStart();
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
			stopWatchStore(OUT_IS_DIFF);


			// resize model size if necessary
			if(cudd.ReadSize() > model_memory_size)
			{
				while (cudd.ReadSize() > model_memory_size)
					model_memory_size *= 2;

				char* new_model = (char*) realloc(model, model_memory_size);
				if(new_model == 0)
				{
					free(model);
					MASSERT(false, "out of memory")
				}
				model = new_model;
			}

			//--------------------------------------------------------------------------------------
			// check satisfiability
			BDD check = side_constraints & output_is_different_bdd;

			stopWatchStart();
			while (!check.IsZero())
			{
				stopWatchStore(SATISFIABILITY);

				stopWatchStart();
				check.PickOneCube(model); // store model
				stopWatchStore(STORE_MODEL);


				stopWatchStart();
				// find the one and only true c-signal in model
				int cj = 0;
				for(int i = first_cj_var; i <= last_cj_var; i++)
				{
					if(model[i] == 1)
					{
						cj = i;
						break;
					}
				}
				cj_to_BDD_signal[cj] = cudd.bddZero(); // free the memory
				// TODO: set cudd.bddVar(cj) to constant false/remove ??

				// find the one and only true f-signal from model
				int fi = 0;
				for (int i = 0; i < f_inputs.size(); i++)
				{
					if(model[f_inputs[i]] == 1)
					{
						fi = f_inputs[i];
						break;
					}
				}
				stopWatchStore(PARSE_MODEL);

				if (Options::instance().isUseDiagnosticOutput())
				{
					ErrorTrace* trace = new ErrorTrace;

					trace->error_timestep_ = timestep;
					trace->input_trace_ = testcase;
					trace->latch_index_ = cj_to_latch[cj];
					trace->flipped_timestep_ = fi_to_timestep[fi];

					ErrorTraceManager::instance().error_traces_.push_back(trace);
				}

				vulnerable_elements_.insert(cj_to_latch[cj]);
				stopWatchStart();
				side_constraints &= ~cudd.bddVar(cj);
				check = side_constraints & output_is_different_bdd;
				stopWatchStore(SIDE_CONSTRAINTS);

				stopWatchStart(); // SATISFIABILITY
			}

		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each testcase' ---------------

	free(model);

}

void BddAnalysis::analyze_one_hot_enc_constr(vector<TestCase>& testcases)
{
	Cudd cudd;
	cudd.AutodynEnable(CUDD_REORDER_SIFT);  // it is better to turn off automatic reordering

	int model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

	AigSimulator sim_(circuit_);

	// used to store the results of the symbolic simulation
	int next_free_cnf_var = 2;

	set<int> latches_to_check_;

	//------------------------------------------------------------------------------------------
	// set up ci signals
	// maps for latch-literals <=> cj-literals: each latch has a corresponding cj literal,
	// which indicates whether the latch is flipped or not.
	stopWatchStart();
	map<int, int> latch_to_cj; // maps latch-literals(cnf) to corresponding cj-literals(cnf)
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)
	map<int, BDD> cj_to_BDD_signal; // maps a cj input literal to the actual cj BDD with cardinality constraints

	int first_cj_var = next_free_cnf_var;
	BDD c_cardinality_constraints = cudd.bddOne();
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		latches_to_check_.insert(circuit_->latches[c_cnt].lit);

		int cj = next_free_cnf_var++;
		cj_to_BDD_signal[cj] = cudd.bddVar(cj);
		latch_to_cj[circuit_->latches[c_cnt].lit >> 1] = cj;
		cj_to_latch[cj] = circuit_->latches[c_cnt].lit;

		for (unsigned cnt = first_cj_var; cnt < cj; cnt++)
		{
			c_cardinality_constraints &= ~(cudd.bddVar(cj) & cudd.bddVar(cnt));
		}
	}
	int last_cj_var = next_free_cnf_var - 1;

	// single fault assumption: there might be at most one flipped component//	map<int, BDD>::iterator map_iter;
//	map<int, BDD>::iterator map_iter2;
//	for (map_iter = cj_to_BDD_signal.begin(); map_iter != cj_to_BDD_signal.end(); map_iter++) // for each latch:
//	{
//		map_iter2 = map_iter;
//		map_iter2++;
//		for (map_iter2 = cj_to_BDD_signal.begin(); map_iter2 != cj_to_BDD_signal.end(); map_iter2++) // for current latch, go over all latches
//		{
//			c_cardinality_constraints &= ~ (map_iter->second & map_iter2->second);
//		}
//	}
	cout << "cardinality constraints are SAT: " << !c_cardinality_constraints.IsZero() << endl;
	stopWatchStore(CREATE_C_SIGNALS);



	//------------------------------------------------------------------------------------------
	BddSimulator bddSim(circuit_, cudd, next_free_cnf_var);


	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{

		// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
		vector<int> concrete_state;
		concrete_state.resize(circuit_->num_latches);

		// f = a set of variables fi indicating whether the latch is *flipped in _step_ i* or not
		vector<int> f_inputs;
		vector<BDD> f_prime_bdd;
		map<int, unsigned> fi_to_timestep;

		BDD side_constraints = c_cardinality_constraints;

		stopWatchStart();
		bddSim.initLatches();
		stopWatchStore(INIT_Latches);

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

			stopWatchStart();
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
			stopWatchStore(MODIFY_LATCHES);

			//--------------------------------------------------------------------------------------
			// single fault assumption: there might be at most one flip in one time-step
			// if fi is true, all other f must be false
			stopWatchStart();
			for (unsigned cnt = 0; cnt < f_prime_bdd.size(); cnt++)
			{
				side_constraints &= ~(fi_bdd & f_prime_bdd[cnt]);
			}
			stopWatchStore(SIDE_CONSTRAINTS);

			f_inputs.push_back(fi);
			f_prime_bdd.push_back(fi_bdd);

			fi_to_timestep[fi] = timestep;
			//--------------------------------------------------------------------------------------

			// Symbolic simulation of AND gates

			stopWatchStart();
			bddSim.simulateOneTimeStep();
			stopWatchStore(SIM_ANDs);
			side_constraints &= ~ bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			stopWatchStart();
			bddSim.switchToNextState();
			stopWatchStore(SWITCH_NXT_ST);


			stopWatchStart();
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
			stopWatchStore(OUT_IS_DIFF);


			// resize model size if necessary
			if(cudd.ReadSize() > model_memory_size)
			{
				while (cudd.ReadSize() > model_memory_size)
					model_memory_size *= 2;

				char* new_model = (char*) realloc(model, model_memory_size);
				if(new_model == 0)
				{
					free(model);
					MASSERT(false, "out of memory")
				}
				model = new_model;
			}

			//--------------------------------------------------------------------------------------
			// check satisfiability
			BDD check = side_constraints & output_is_different_bdd;

			stopWatchStart();
			while (!check.IsZero())
			{
				stopWatchStore(SATISFIABILITY);

				cout << "SAAAAAAAAAAAAAAAAAAAAAT" << endl;

				stopWatchStart();
				check.PickOneCube(model); // store model
				stopWatchStore(STORE_MODEL);


				stopWatchStart();
				// find the one and only true c-signal in model
				int cj = 0;
				for(int i = first_cj_var; i <= last_cj_var; i++)
				{
					if(model[i] == 1)
					{
						cj = i;
						break;
					}
				}
				cj_to_BDD_signal[cj] = cudd.bddZero(); // free the memory
				// TODO: set cudd.bddVar(cj) to constant false/remove ??

				// find the one and only true f-signal from model
				int fi = 0;
				for (int i = 0; i < f_inputs.size(); i++)
				{
					if(model[f_inputs[i]] == 1)
					{
						fi = f_inputs[i];
						break;
					}
				}
				stopWatchStore(PARSE_MODEL);

				if (Options::instance().isUseDiagnosticOutput())
				{
					ErrorTrace* trace = new ErrorTrace;

					trace->error_timestep_ = timestep;
					trace->input_trace_ = testcase;
					trace->latch_index_ = cj_to_latch[cj];
					trace->flipped_timestep_ = fi_to_timestep[fi];

					ErrorTraceManager::instance().error_traces_.push_back(trace);
				}

				vulnerable_elements_.insert(cj_to_latch[cj]);
				stopWatchStart();
				side_constraints &= ~cudd.bddVar(cj);
				check = side_constraints & output_is_different_bdd;
				stopWatchStore(SIDE_CONSTRAINTS);

				stopWatchStart(); // SATISFIABILITY
			}

		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each testcase' ---------------

	free(model);
}

void BddAnalysis::analyze_binary_enc_sig(vector<TestCase>& testcases)
{
	Cudd cudd;
	//cudd.AutodynEnable(CUDD_REORDER_SIFT);  // it is better to turn off automatic reordering

	int model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

	AigSimulator sim_(circuit_);

	// used to store the results of the symbolic simulation
	int next_free_cnf_var = 2;

	set<int> latches_to_check_;

	//------------------------------------------------------------------------------------------
	// set up ci signals
	// maps for latch-literals <=> cj-literals: each latch has a corresponding cj literal,
	// which indicates whether the latch is flipped or not.
	stopWatchStart();
	map<unsigned, BDD> latch_to_BDD_signal; // maps a cj input literal to the actual cj BDD with cardinality constraints

	unsigned latches_to_check = circuit_->num_latches - num_err_latches_;
	unsigned num_of_c_vars = ceil(log2(latches_to_check));

	int first_cj_var = next_free_cnf_var;
	int last_cj_var = first_cj_var + num_of_c_vars - 1;
	vector<BDD> c_vars;
	c_vars.reserve(num_of_c_vars);

	for (unsigned c_cnt = 0; c_cnt < num_of_c_vars; ++c_cnt)
		c_vars.push_back(cudd.bddVar(next_free_cnf_var++));

	for (unsigned bin_encoding = 0; bin_encoding < latches_to_check; ++bin_encoding)
	{
		unsigned current_latch = circuit_->latches[bin_encoding].lit;
		latches_to_check_.insert(current_latch);

		BDD real_cj = cudd.bddOne();
		for (unsigned c_cnt = 0; c_cnt < num_of_c_vars; ++c_cnt)
		{
			if ((bin_encoding & (1 << c_cnt)) > 0) // bit is TRUE
			{
				real_cj &= c_vars[num_of_c_vars-1-c_cnt];
			}
			else // bit is FALSE
			{
				real_cj &= ~c_vars[num_of_c_vars-1-c_cnt];
			}
		}
		latch_to_BDD_signal[current_latch] = real_cj;
		cout << "latch "<<bin_encoding << endl;

		real_cj.PrintMinterm();
//		char ass[5];
//		real_cj.PickOneCube(ass);
//		for (int i=0; i<5; i++)
//			cout << static_cast<int>(ass[i]);

	}
	stopWatchStore(CREATE_C_SIGNALS);



	//------------------------------------------------------------------------------------------
	BddSimulator bddSim(circuit_, cudd, next_free_cnf_var);


	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{

		// initial state for concrete simulation = (0 0 0 0 0 0 0)  (AIG literals)
		vector<int> concrete_state;
		concrete_state.resize(circuit_->num_latches);

		// f = a set of variables fi indicating whether the latch is *flipped in _step_ i* or not
		vector<int> f_inputs;
		vector<BDD> f_prime_bdd;
		map<int, unsigned> fi_to_timestep;

		BDD side_constraints = cudd.bddOne();

		stopWatchStart();
		bddSim.initLatches();
		stopWatchStore(INIT_Latches);

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

			stopWatchStart();
			set<int>::iterator it;
			for (it = latches_to_check_.begin(); it != latches_to_check_.end(); ++it)
			{
				int latch_output = *it >> 1;

				BDD cj_bdd = latch_to_BDD_signal[*it];
				BDD old_value = bddSim.getResultValue(latch_output);

				// todo: maybe use Cudd_addIte instead??
				BDD new_value = (fi_bdd & cj_bdd) ^ old_value; // flip old_value iff both fi and cj are true
				bddSim.setResultValue(latch_output, new_value);
			}
			stopWatchStore(MODIFY_LATCHES);

			//--------------------------------------------------------------------------------------
			// single fault assumption: there might be at most one flip in one time-step
			// if fi is true, all other f must be false
			stopWatchStart();
			for (unsigned cnt = 0; cnt < f_prime_bdd.size(); cnt++)
			{
				side_constraints &= ~(fi_bdd & f_prime_bdd[cnt]);
			}
			stopWatchStore(SIDE_CONSTRAINTS);

			f_inputs.push_back(fi);
			f_prime_bdd.push_back(fi_bdd);

			fi_to_timestep[fi] = timestep;
			//--------------------------------------------------------------------------------------

			// Symbolic simulation of AND gates

			stopWatchStart();
			bddSim.simulateOneTimeStep();
			stopWatchStore(SIM_ANDs);
			side_constraints &= ~ bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			stopWatchStart();
			bddSim.switchToNextState();
			stopWatchStore(SWITCH_NXT_ST);


			stopWatchStart();
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
			stopWatchStore(OUT_IS_DIFF);


			// resize model size if necessary
			if(cudd.ReadSize() > model_memory_size)
			{
				while (cudd.ReadSize() > model_memory_size)
					model_memory_size *= 2;

				char* new_model = (char*) realloc(model, model_memory_size);
				if(new_model == 0)
				{
					free(model);
					MASSERT(false, "out of memory")
				}
				model = new_model;
			}

			//--------------------------------------------------------------------------------------
			// check satisfiability
			BDD check = side_constraints & output_is_different_bdd;

			stopWatchStart();
			while (!check.IsZero())
			{
				stopWatchStore(SATISFIABILITY);
				cout << "SATIisfiable!" << endl;
//				check.PrintMinterm();

				stopWatchStart();
				check.PickOneCube(model); // store model
				stopWatchStore(STORE_MODEL);


				stopWatchStart();
				// find the one and only true c-signal in model
				vector<unsigned> irrelevant_c_bits;
				unsigned cj = 0;
				unsigned bit_counter = num_of_c_vars - 1;
				BDD blocking_cube = cudd.bddOne();
//				cout << "first " << first_cj_var << ", last " << last_cj_var << endl;
				for(int i = first_cj_var; i <= last_cj_var; i++)
				{
					cout << "model["<<i<<"]" << static_cast<int>(model[i]) << endl;
					if(model[i] == 1)
					{
						cj |= (1 << bit_counter);
						blocking_cube &= c_vars[num_of_c_vars-1-bit_counter];
					}
					else if(model[i] == 0)
					{
						blocking_cube &= ~ c_vars[num_of_c_vars-1-bit_counter];
					}
					else // input is irrelevant
					{
						irrelevant_c_bits.push_back(bit_counter);
					}

//					MASSERT(model[i] <= 1, "WRONG")

					bit_counter--;
				}
				vector<unsigned> vulnerable_cj_combinations;

				if (irrelevant_c_bits.size() == 0) // is already concrete
				{
					vulnerable_cj_combinations.push_back(cj);
				}
				else // irrelevant input values
				{
					for (unsigned  concrete_vals = 0; concrete_vals < pow(2,irrelevant_c_bits.size()); concrete_vals++)
					{
						unsigned concrete_cj = cj;
						for(unsigned i = 0; i < irrelevant_c_bits.size(); i++)
						{
							if ((concrete_vals & (1 << i)) > 0 ) // bit set in combination?
								concrete_cj |= (1 << irrelevant_c_bits[i]); // set the bit
						}
						vulnerable_cj_combinations.push_back(concrete_cj);
					}
				}

				// find the one and only true f-signal from model
				int fi = 0;
				for (int i = 0; i < f_inputs.size(); i++)
				{
					if(model[f_inputs[i]] == 1)
					{
						fi = f_inputs[i];
						break;
					}
				}
				stopWatchStore(PARSE_MODEL);
				for(unsigned i = 0; i < vulnerable_cj_combinations.size(); i++)
				{
					cj = vulnerable_cj_combinations[i];
					cout << "cj = " << cj << endl;
					latch_to_BDD_signal[cj] = cudd.bddZero(); // free the memory

					if (Options::instance().isUseDiagnosticOutput())
					{
						ErrorTrace* trace = new ErrorTrace;

						trace->error_timestep_ = timestep;
						trace->input_trace_ = testcase;
						trace->latch_index_ = circuit_->latches[cj].lit;
						trace->flipped_timestep_ = fi_to_timestep[fi];

						ErrorTraceManager::instance().error_traces_.push_back(trace);
					}

					cout << "latch " << circuit_->latches[cj].lit << endl;

					vulnerable_elements_.insert(circuit_->latches[cj].lit);
				}

				stopWatchStart();
				side_constraints &= ~blocking_cube;
				check = side_constraints & output_is_different_bdd;
				stopWatchStore(SIDE_CONSTRAINTS);

				stopWatchStart(); // SATISFIABILITY
			}

		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each testcase' ---------------

	free(model);

}

// -------------------------------------------------------------------------------------------
BddAnalysis::~BddAnalysis()
{
}

void BddAnalysis::stopWatchStart()
{
	if(!useStatistics_)
		return;

	start_time_ = Stopwatch::start();
}

void BddAnalysis::stopWatchStore(Statistic statistic)
{
	if(!useStatistics_)
		return;

	map<Statistic,double>::iterator it = accumulated_durations_.find(statistic);
	double duration = Stopwatch::getCPUTimeMilliSec(start_time_);
	if(it != accumulated_durations_.end())
	{
	   it->second += duration;
	}
	else
	{
		accumulated_durations_[statistic] = duration;
	}
}

void BddAnalysis::printStatistics(PointInTime begin)
{
	if(!useStatistics_)
		return;

	const string stat_name[] =
	{ "CREATE_C_SIGNALS", "SIM_ANDs", "SWITCH_NXT_ST", "OUT_IS_DIFF", "SATISFIABILITY",
			"STORE_MODEL", "INIT_Latches", "SIDE_CONSTRAINTS", "MODIFY_LATCHES", "PARSE_MODEL" };

	double total_time = Stopwatch::getCPUTimeMilliSec(begin);
	double in_stats = 0.0;
	cout << endl << "--------------------------" << endl;
	cout << "Total execution time: " << total_time << endl;
	map<Statistic,double>::iterator it = accumulated_durations_.begin();
	for(;it !=accumulated_durations_.end(); ++it)
	{
		cout << stat_name[it->first] <<" " << it->second << endl;
		in_stats += it->second;
	}
	cout << "uncategorized " << (total_time - in_stats) << endl;
}
