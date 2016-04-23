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
#include "BddSimulator2.h"
#include "ErrorTraceManager.h"
#include "Logger.h"
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

	next_free_cnf_var_ = 2;

//	cudd_.AutodynEnable(CUDD_REORDER_SIFT);  // it is better to turn off automatic reordering
//	cudd_.EnableReorderingReporting();

// TODO modes...
//	analyze_one_hot_enc_c_signals(testcases);
//	analyze_one_hot_enc_c_constraints(testcases);
//	analyze_binary_enc_c_signals(testcases);
	//analyze_binary_enc_c_and_f_signals(testcases);
	analyze_binary_enc_c_and_f_signals_FREE_INPUTS(testcases);
	// ...

	printStatistics(begin);
	return vulnerable_elements_.size() > 0;

}

void BddAnalysis::analyze_one_hot_enc_c_signals(vector<TestCase>& testcases)
{

	int model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

	AigSimulator sim_(circuit_);

	set<int> latches_to_check_;

	//------------------------------------------------------------------------------------------
	// set up ci signals
	// maps for latch-literals <=> cj-literals: each latch has a corresponding cj literal,
	// which indicates whether the latch is flipped or not.
	stopWatchStart();
	map<int, int> latch_to_cj; // maps latch-literals(cnf) to corresponding cj-literals(cnf)
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)
	map<int, BDD> cj_to_BDD_signal; // maps a cj input literal to the actual cj BDD with cardinality constraints

	int first_cj_var = next_free_cnf_var_;
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		latches_to_check_.insert(circuit_->latches[c_cnt].lit);

		int cj = next_free_cnf_var_++;
		cudd_.bddVar(cj);

		latch_to_cj[circuit_->latches[c_cnt].lit >> 1] = cj;
		cj_to_latch[cj] = circuit_->latches[c_cnt].lit;
	}
	int last_cj_var = next_free_cnf_var_ - 1;

	// single fault assumption: there might be at most one flipped component
	map<int, int>::iterator map_iter;
	map<int, int>::iterator map_iter2;
	for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++) // for each latch:
	{
		BDD real_c_signal = cudd_.bddOne();
		for (map_iter2 = latch_to_cj.begin(); map_iter2 != latch_to_cj.end(); map_iter2++) // for current latch, go over all latches
		{
			if (map_iter == map_iter2) // the one and only cj-signal which can be true for this signal
				real_c_signal &= cudd_.bddVar(map_iter2->second);
			else
				real_c_signal &= ~cudd_.bddVar(map_iter2->second);
		}
		cj_to_BDD_signal[map_iter->second] = real_c_signal;
	}
	stopWatchStore(CREATE_C_SIGNALS);

	//------------------------------------------------------------------------------------------
	BddSimulator2 bddSim(circuit_, cudd_, next_free_cnf_var_);

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

		BDD side_constraints = cudd_.bddOne();

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

			int fi = next_free_cnf_var_++;
			BDD fi_bdd = cudd_.bddVar(fi);

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
			side_constraints &= ~bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			stopWatchStart();
			bddSim.switchToNextState();
			stopWatchStore(SWITCH_NXT_ST);

			stopWatchStart();
			//--------------------------------------------------------------------------------------
			// constraints saying that the current outputs_ok o and o' are different
			BDD output_is_different_bdd = cudd_.bddZero();
			for (unsigned out_idx = 0; out_idx < output_bdds.size(); ++out_idx)
			{
				if (outputs_ok[out_idx] == AIG_TRUE) // simulation result of output is true
					output_is_different_bdd |= ~output_bdds[out_idx];
				else if (outputs_ok[out_idx] == AIG_FALSE)
					output_is_different_bdd |= output_bdds[out_idx];
			}
			//--------------------------------------------------------------------------------------
			stopWatchStore(OUT_IS_DIFF);

			// resize model size if necessary
			if (cudd_.ReadSize() > model_memory_size)
			{
				while (cudd_.ReadSize() > model_memory_size)
					model_memory_size *= 2;

				char* new_model = (char*) realloc(model, model_memory_size);
				if (new_model == 0)
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
				for (int i = first_cj_var; i <= last_cj_var; i++)
				{
					if (model[i] == 1)
					{
						cj = i;
						break;
					}
				}
				cj_to_BDD_signal[cj] = cudd_.bddZero(); // free the memory
				// TODO: set cudd.bddVar(cj) to constant false/remove ??

				// find the one and only true f-signal from model
				int fi = 0;
				for (int i = 0; i < f_inputs.size(); i++)
				{
					if (model[f_inputs[i]] == 1)
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
				side_constraints &= ~cudd_.bddVar(cj);
				check = side_constraints & output_is_different_bdd;
				stopWatchStore(SIDE_CONSTRAINTS);

				stopWatchStart(); // SATISFIABILITY
			}

		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each testcase' ---------------

	free(model);

}

void BddAnalysis::analyze_one_hot_enc_c_constraints(vector<TestCase>& testcases)
{
	int model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

	AigSimulator sim_(circuit_);

	set<int> latches_to_check_;

	//------------------------------------------------------------------------------------------
	// set up ci signals
	// maps for latch-literals <=> cj-literals: each latch has a corresponding cj literal,
	// which indicates whether the latch is flipped or not.
	stopWatchStart();
	map<int, int> latch_to_cj; // maps latch-literals(cnf) to corresponding cj-literals(cnf)
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)
	map<int, BDD> cj_to_BDD_signal; // maps a cj input literal to the actual cj BDD with cardinality constraints

	int first_cj_var = next_free_cnf_var_;
	BDD c_cardinality_constraints = cudd_.bddOne();
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		latches_to_check_.insert(circuit_->latches[c_cnt].lit);

		int cj = next_free_cnf_var_++;
		cj_to_BDD_signal[cj] = cudd_.bddVar(cj);
		latch_to_cj[circuit_->latches[c_cnt].lit >> 1] = cj;
		cj_to_latch[cj] = circuit_->latches[c_cnt].lit;

		// single fault assumption: there might be at most one flipped component
		for (unsigned cnt = first_cj_var; cnt < cj; cnt++)
		{
			c_cardinality_constraints &= ~(cudd_.bddVar(cj) & cudd_.bddVar(cnt));
		}
	}
	int last_cj_var = next_free_cnf_var_ - 1;
	stopWatchStore(CREATE_C_SIGNALS);

	BddSimulator bddSim(circuit_, cudd_, next_free_cnf_var_);

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

			// Concrete simulations:
			sim_.simulateOneTimeStep(testcase[timestep], concrete_state);
			vector<int> outputs_ok = sim_.getOutputs();
			vector<int> next_state = sim_.getNextLatchValues();

			// switch concrete simulation to next state
			concrete_state = next_state;

			// set input values according to TestCase to TRUE or FALSE:
			bddSim.setInputValues(testcase[timestep]);

			//--------------------------------------------------------------------------------------
			// cj indicates whether the corresponding latch is flipped
			// fi indicates whether the component is flipped in step i or not
			// there can only be a flip at time-step i if both cj and fi are true.

			int fi = next_free_cnf_var_++;
			BDD fi_bdd = cudd_.bddVar(fi);

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
			side_constraints &= ~bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			stopWatchStart();
			bddSim.switchToNextState();
			stopWatchStore(SWITCH_NXT_ST);

			stopWatchStart();
			//--------------------------------------------------------------------------------------
			// constraints saying that the current outputs_ok o and o' are different
			BDD output_is_different_bdd = cudd_.bddZero();
			for (unsigned out_idx = 0; out_idx < output_bdds.size(); ++out_idx)
			{
				if (outputs_ok[out_idx] == AIG_TRUE) // simulation result of output is true
					output_is_different_bdd |= ~output_bdds[out_idx];
				else if (outputs_ok[out_idx] == AIG_FALSE)
					output_is_different_bdd |= output_bdds[out_idx];
			}
			//--------------------------------------------------------------------------------------
			stopWatchStore(OUT_IS_DIFF);

			// resize model size if necessary
			if (cudd_.ReadSize() > model_memory_size)
			{
				while (cudd_.ReadSize() > model_memory_size)
					model_memory_size *= 2;

				char* new_model = (char*) realloc(model, model_memory_size);
				if (new_model == 0)
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
				for (int i = first_cj_var; i <= last_cj_var; i++)
				{
					if (model[i] == 1)
					{
						cj = i;
						break;
					}
				}
				cj_to_BDD_signal[cj] = cudd_.bddZero(); // free the memory
				// TODO: set cudd.bddVar(cj) to constant false/remove ??

				// find the one and only true f-signal from model
				int fi = 0;
				for (int i = 0; i < f_inputs.size(); i++)
				{
					if (model[f_inputs[i]] == 1)
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
				side_constraints &= ~cudd_.bddVar(cj);
				check = side_constraints & output_is_different_bdd;
				stopWatchStore(SIDE_CONSTRAINTS);

				stopWatchStart(); // SATISFIABILITY
			}

		} // -- END "for each timestep in testcase" --
	} // ------ END 'for each testcase' ---------------

	free(model);
}

BDD BddAnalysis::binary_conjunct(unsigned binary_encoding,
		const vector<BDD>& input_vars)
{
	MASSERT(binary_encoding < pow(2,input_vars.size()), "not enough input bits provided");

	BDD result = cudd_.bddOne();
	for (unsigned c_cnt = 0; c_cnt < input_vars.size(); ++c_cnt)
	{
		if ((binary_encoding & (1 << c_cnt)) > 0) // bit is TRUE
		{
			result &= input_vars[input_vars.size() - 1 - c_cnt];
		}
		else // bit is FALSE
		{
			result &= ~input_vars[input_vars.size() - 1 - c_cnt];
		}
	}

	return result;
}

void BddAnalysis::create_binary_encoded_c_BDDs(const vector<BDD>& c_vars,
		map<unsigned, BDD>& latch_to_BDD_signal, set<int>& latches_to_check_)
{

	unsigned num_of_signals_to_encode = circuit_->num_latches - num_err_latches_;
	for (unsigned binary_encoding = 0; binary_encoding < num_of_signals_to_encode;
			++binary_encoding)
	{
		unsigned current_latch = circuit_->latches[binary_encoding].lit;
		latches_to_check_.insert(current_latch);
		latch_to_BDD_signal[current_latch] = binary_conjunct(binary_encoding, c_vars);
	}
}

void BddAnalysis::analyze_binary_enc_c_signals(vector<TestCase>& testcases)
{
	int model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

	stopWatchStart();

	// the binary logarithm of the number of latches is the number of c signals
	unsigned num_latches_to_check = circuit_->num_latches - num_err_latches_;
	unsigned num_of_c_vars = ceil(log2(num_latches_to_check));

	int first_cj_var = next_free_cnf_var_;
	int last_cj_var = first_cj_var + num_of_c_vars - 1;

	vector<BDD> c_vars;
	c_vars.reserve(num_of_c_vars);
	for (unsigned c_cnt = 0; c_cnt < num_of_c_vars; ++c_cnt)
		c_vars.push_back(cudd_.bddVar(next_free_cnf_var_++));

	// create the binary encoding for c vars
	map<unsigned, BDD> latch_to_BDD_signal;
	set<int> latches_to_check_;
	create_binary_encoded_c_BDDs(c_vars, latch_to_BDD_signal, latches_to_check_);
	stopWatchStore(CREATE_C_SIGNALS);

	AigSimulator sim_concrete_ok(circuit_);
	BddSimulator2 bddSim(circuit_, cudd_, next_free_cnf_var_);

	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{
		// f = a set of variables fi indicating whether the latch is *flipped in _step_ i* or not
		vector<int> f_inputs;
		vector<BDD> f_prime_bdd;
		map<int, unsigned> fi_to_timestep;

		BDD side_constraints = cudd_.bddOne();

		sim_concrete_ok.initLatches();
		stopWatchStart();
		bddSim.initLatches();
		stopWatchStore(INIT_Latches);

		TestCase& testcase = testcases[tc_number];
		for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_concrete_ok.simulateOneTimeStep(testcase[timestep]);
			vector<int> outputs_ok = sim_concrete_ok.getOutputs();
			sim_concrete_ok.switchToNextState();
			//--------------------------------------------------------------------------------------

			// set input values according to TestCase to TRUE or FALSE:
			bddSim.setInputValues(testcase[timestep]);

			//--------------------------------------------------------------------------------------
			// cj indicates whether the corresponding latch is flipped
			// fi indicates whether the component is flipped in step i or not
			// there can only be a flip at time-step i if both cj and fi are true.

			int fi = next_free_cnf_var_++;
			BDD fi_bdd = cudd_.bddVar(fi);

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
			side_constraints &= ~bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			stopWatchStart();
			bddSim.switchToNextState();
			stopWatchStore(SWITCH_NXT_ST);

			stopWatchStart();
			//--------------------------------------------------------------------------------------
			// constraints saying that the current outputs_ok o and o' are different
			BDD output_is_different_bdd = cudd_.bddZero();
			for (unsigned out_idx = 0; out_idx < output_bdds.size(); ++out_idx)
			{
				if (outputs_ok[out_idx] == AIG_TRUE) // simulation result of output is true
					output_is_different_bdd |= ~output_bdds[out_idx];
				else if (outputs_ok[out_idx] == AIG_FALSE)
					output_is_different_bdd |= output_bdds[out_idx];
			}
			//--------------------------------------------------------------------------------------
			stopWatchStore(OUT_IS_DIFF);

			// resize model size if necessary
			if (cudd_.ReadSize() > model_memory_size)
			{
				while (cudd_.ReadSize() > model_memory_size)
					model_memory_size *= 2;

				char* new_model = (char*) realloc(model, model_memory_size);
				if (new_model == 0)
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
				vector<unsigned> dontcare_c_bits;
				unsigned cj = 0;
				unsigned bit_index = num_of_c_vars - 1;
				BDD blocking_cube = cudd_.bddOne();
				for (int i = first_cj_var; i <= last_cj_var; i++)
				{
					if (model[i] == 1)
					{
						cj |= (1 << bit_index);
						blocking_cube &= c_vars[num_of_c_vars - 1 - bit_index];
					}
					else if (model[i] == 0)
					{
						blocking_cube &= ~c_vars[num_of_c_vars - 1 - bit_index];
					}
					else // input is irrelevant
					{
						dontcare_c_bits.push_back(bit_index);
					}
					bit_index--;
				}
				vector<unsigned> vulnerable_cj_combinations;

				if (dontcare_c_bits.size() == 0) // no bits are irrelevant for satisfiability
				{
					vulnerable_cj_combinations.push_back(cj); // use the concrete value
				}
				else // create all possible c combinations
				{
					for (unsigned concrete_vals = 0;
							concrete_vals < pow(2, dontcare_c_bits.size()); concrete_vals++)
					{
						unsigned concrete_cj = cj;
						for (unsigned i = 0; i < dontcare_c_bits.size(); i++)
						{
							if ((concrete_vals & (1 << i)) > 0) // bit set in combination?
								concrete_cj |= (1 << dontcare_c_bits[i]); // set the bit
						}
						vulnerable_cj_combinations.push_back(concrete_cj);
					}
				}

				// find the one and only true f-signal from model
				int fi = 0;
				for (int i = 0; i < f_inputs.size(); i++)
				{
					if (model[f_inputs[i]] == 1)
					{
						fi = f_inputs[i];
//						cout << "fi" << fi << "timestep" << fi_to_timestep[fi] << endl;
						break;
					}
				}
				stopWatchStore(PARSE_MODEL);
				for (unsigned i = 0; i < vulnerable_cj_combinations.size(); i++)
				{
					cj = vulnerable_cj_combinations[i];
//					latch_to_BDD_signal[cj] = cudd.bddZero(); // NO, DON't do this!!

					if (Options::instance().isUseDiagnosticOutput())
					{
						ErrorTrace* trace = new ErrorTrace;

						trace->error_timestep_ = timestep;
						trace->input_trace_ = testcase;
						trace->latch_index_ = circuit_->latches[cj].lit;
						trace->flipped_timestep_ = fi_to_timestep[fi];

						ErrorTraceManager::instance().error_traces_.push_back(trace);
					}

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

void BddAnalysis::analyze_binary_enc_c_and_f_signals(vector<TestCase>& testcases)
{
	int model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

	stopWatchStart();

	// the binary logarithm of the number of latches is the number of c signals
	unsigned num_latches_to_check = circuit_->num_latches - num_err_latches_;
	unsigned num_of_c_vars = ceil(log2(num_latches_to_check));

	int first_cj_var = next_free_cnf_var_;
	int last_cj_var = first_cj_var + num_of_c_vars - 1;

	vector<BDD> c_vars;
	c_vars.reserve(num_of_c_vars);
	for (unsigned c_cnt = 0; c_cnt < num_of_c_vars; ++c_cnt)
		c_vars.push_back(cudd_.bddVar(next_free_cnf_var_++));

	// create the binary encoding for c vars
	map<unsigned, BDD> latch_to_BDD_signal;
	set<int> latches_to_check_;
	create_binary_encoded_c_BDDs(c_vars, latch_to_BDD_signal, latches_to_check_);
	stopWatchStore(CREATE_C_SIGNALS);

	AigSimulator sim_concrete_ok(circuit_);
	BddSimulator2 bddSim(circuit_, cudd_, next_free_cnf_var_);

	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{
		BDD side_constraints = cudd_.bddOne();

		sim_concrete_ok.initLatches();
		stopWatchStart();
		bddSim.initLatches();
		stopWatchStore(INIT_Latches);

		TestCase& testcase = testcases[tc_number];

		unsigned num_of_f_vars = ceil(log2(testcase.size() + 1));
		int first_f_var = next_free_cnf_var_;
		int last_f_var = first_f_var + num_of_f_vars - 1;
		vector<BDD> f_vars;
		f_vars.reserve(num_of_f_vars);
		for (unsigned f_cnt = 0; f_cnt < num_of_f_vars; ++f_cnt)
			f_vars.push_back(cudd_.bddVar(next_free_cnf_var_++));

		for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_concrete_ok.simulateOneTimeStep(testcase[timestep]);
			vector<int> outputs_ok = sim_concrete_ok.getOutputs();
			sim_concrete_ok.switchToNextState();
			//--------------------------------------------------------------------------------------

			// set input values according to TestCase to TRUE or FALSE:
			bddSim.setInputValues(testcase[timestep]);

			//--------------------------------------------------------------------------------------
			// cj indicates whether the corresponding latch is flipped
			// fi indicates whether the component is flipped in step i or not
			// there can only be a flip at time-step i if both cj and fi are true.

			BDD fi_bdd = binary_conjunct(timestep, f_vars);

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
			// Symbolic simulation of AND gates
			stopWatchStart();
			bddSim.simulateOneTimeStep();
			stopWatchStore(SIM_ANDs);
			side_constraints &= ~bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			stopWatchStart();
			bddSim.switchToNextState(side_constraints);
			stopWatchStore(SWITCH_NXT_ST);

			stopWatchStart();
			//--------------------------------------------------------------------------------------
			// constraints saying that the current outputs_ok o and o' are different
			BDD output_is_different_bdd = cudd_.bddZero();
			for (unsigned out_idx = 0; out_idx < output_bdds.size(); ++out_idx)
			{
				if (outputs_ok[out_idx] == AIG_TRUE) // simulation result of output is true
					output_is_different_bdd |= ~output_bdds[out_idx];
				else if (outputs_ok[out_idx] == AIG_FALSE)
					output_is_different_bdd |= output_bdds[out_idx];
			}
			//--------------------------------------------------------------------------------------
			stopWatchStore(OUT_IS_DIFF);

			// resize model size if necessary
			if (cudd_.ReadSize() > model_memory_size)
			{
				while (cudd_.ReadSize() > model_memory_size)
					model_memory_size *= 2;

				char* new_model = (char*) realloc(model, model_memory_size);
				if (new_model == 0)
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
				vector<unsigned> dontcare_c_bits;
				unsigned cj = 0;
				unsigned bit_index = num_of_c_vars - 1;
				BDD blocking_cube = cudd_.bddOne();
				for (int i = first_cj_var; i <= last_cj_var; i++)
				{
					if (model[i] == 1)
					{
						cj |= (1 << bit_index);
						blocking_cube &= c_vars[num_of_c_vars - 1 - bit_index];
					}
					else if (model[i] == 0)
					{
						blocking_cube &= ~c_vars[num_of_c_vars - 1 - bit_index];
					}
					else // input is irrelevant
					{
						dontcare_c_bits.push_back(bit_index);
					}
					bit_index--;
				}
				vector<unsigned> vulnerable_cj_combinations;

				if (dontcare_c_bits.size() == 0) // no bits are irrelevant for satisfiability
				{
					vulnerable_cj_combinations.push_back(cj); // use the concrete value
				}
				else // create all possible c combinations
				{
					for (unsigned concrete_vals = 0;
							concrete_vals < pow(2, dontcare_c_bits.size()); concrete_vals++)
					{
						unsigned concrete_cj = cj;
						for (unsigned i = 0; i < dontcare_c_bits.size(); i++)
						{
							if ((concrete_vals & (1 << i)) > 0) // bit set in combination?
								concrete_cj |= (1 << dontcare_c_bits[i]); // set the bit
						}
						vulnerable_cj_combinations.push_back(concrete_cj);
					}
				}

				// find the one and only true f-signal from model
				int flip_timestep = 0;
				bit_index = 0;
				for (int i = first_f_var; i <= last_f_var; i++)
				{
					if (model[i] == 1)
					{
						flip_timestep |= (1 << (num_of_f_vars - 1 - bit_index));
					}
					bit_index++;
				}
//				cout << "flip timestep = " << flip_timestep << endl;
				stopWatchStore(PARSE_MODEL);
				for (unsigned i = 0; i < vulnerable_cj_combinations.size(); i++)
				{
					cj = vulnerable_cj_combinations[i];
//					latch_to_BDD_signal[cj] = cudd.bddZero(); // NO, DON't do this!!

					if (Options::instance().isUseDiagnosticOutput())
					{
						ErrorTrace* trace = new ErrorTrace;

						trace->error_timestep_ = timestep;
						trace->input_trace_ = testcase;
						trace->latch_index_ = circuit_->latches[cj].lit;
						trace->flipped_timestep_ = flip_timestep;

						ErrorTraceManager::instance().error_traces_.push_back(trace);
					}

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


void BddAnalysis::analyze_binary_enc_c_and_f_signals_FREE_INPUTS(vector<TestCase>& testcases)
{


	int model_memory_size = 128;
	char* model = (char*) malloc(model_memory_size * sizeof(char));

	stopWatchStart();

	// the binary logarithm of the number of latches is the number of c signals
	unsigned num_latches_to_check = circuit_->num_latches - num_err_latches_;
	unsigned num_of_c_vars = ceil(log2(num_latches_to_check));

	int first_cj_var = next_free_cnf_var_;
	int last_cj_var = first_cj_var + num_of_c_vars - 1;

	vector<BDD> c_vars;
	c_vars.reserve(num_of_c_vars);
	for (unsigned c_cnt = 0; c_cnt < num_of_c_vars; ++c_cnt)
		c_vars.push_back(cudd_.bddVar(next_free_cnf_var_++));

	// create the binary encoding for c vars
	map<unsigned, BDD> latch_to_BDD_signal;
	set<int> latches_to_check_;
	create_binary_encoded_c_BDDs(c_vars, latch_to_BDD_signal, latches_to_check_);
	stopWatchStore(CREATE_C_SIGNALS);

	BddSimulator sim_ok(circuit_,cudd_,next_free_cnf_var_);
	BddSimulator bddSim(circuit_, cudd_, next_free_cnf_var_);

	// for each testcase-step
	for (unsigned tc_number = 0; tc_number < testcases.size(); tc_number++)
	{
		BDD side_constraints = cudd_.bddOne();

		sim_ok.initLatches();
		stopWatchStart();
		bddSim.initLatches();
		stopWatchStore(INIT_Latches);

		TestCase& testcase = testcases[tc_number];

		unsigned num_of_f_vars = ceil(log2(testcase.size() + 1));
		int first_f_var = next_free_cnf_var_;
		int last_f_var = first_f_var + num_of_f_vars - 1;
		vector<BDD> f_vars;
		f_vars.reserve(num_of_f_vars);
		for (unsigned f_cnt = 0; f_cnt < num_of_f_vars; ++f_cnt)
			f_vars.push_back(cudd_.bddVar(next_free_cnf_var_++));

		TestCase real_cnf_inputs; // replaces all unknown input values in the test case with cnf literals
		for (unsigned timestep = 0; timestep < testcase.size(); timestep++)
		{ // -------- BEGIN "for each timestep in testcase" --------------------------------------

			//--------------------------------------------------------------------------------------
			// Concrete simulations:
			sim_ok.simulateOneTimeStep(testcase[timestep]);
			vector<BDD> outputs_ok;
			sim_ok.getOutputValues(outputs_ok);

			// switch concrete simulation to next state
			sim_ok.switchToNextState();
			//--------------------------------------------------------------------------------------

			// set input values according to TestCase to TRUE or FALSE or same open input literal:
			bddSim.setBddInputValues(sim_ok.getInputValues());
			real_cnf_inputs.push_back(sim_ok.getCurrentCnfInputLiterals());


			//--------------------------------------------------------------------------------------
			// cj indicates whether the corresponding latch is flipped
			// fi indicates whether the component is flipped in step i or not
			// there can only be a flip at time-step i if both cj and fi are true.

			BDD fi_bdd = binary_conjunct(timestep, f_vars);

			stopWatchStart();
			set<int>::iterator it;
			for (it = latches_to_check_.begin(); it != latches_to_check_.end(); ++it)
			{
				int latch_output = *it >> 1;

				BDD cj_bdd = latch_to_BDD_signal[*it];
				BDD old_value = bddSim.getResultValue(latch_output);

				BDD new_value = (fi_bdd & cj_bdd) ^ old_value; // flip old_value iff both fi and cj are true
				bddSim.setResultValue(latch_output, new_value);
			}
			stopWatchStore(MODIFY_LATCHES);

			//--------------------------------------------------------------------------------------
			// Symbolic simulation of AND gates
			stopWatchStart();
			bddSim.simulateOneTimeStep();
			stopWatchStore(SIM_ANDs);
			side_constraints &= ~bddSim.getAlarmValue();

			vector<BDD> output_bdds;
			bddSim.getOutputValues(output_bdds);

			stopWatchStart();
			bddSim.switchToNextState();
			stopWatchStore(SWITCH_NXT_ST);

			stopWatchStart();
			//--------------------------------------------------------------------------------------
			// constraints saying that the current outputs_ok o and o' are different
			BDD output_is_different_bdd = cudd_.bddZero();
			for (unsigned out_idx = 0; out_idx < output_bdds.size(); ++out_idx)
			{
				//				if (outputs_ok[out_idx] == cudd_.bddOne()) // simulation result of output is true
				//					output_is_different_bdd |= ~output_bdds[out_idx];
				//				else if (outputs_ok[out_idx] == cudd_.bddZero())
				//					output_is_different_bdd |= output_bdds[out_idx];
				//				else
					output_is_different_bdd |= output_bdds[out_idx] ^ outputs_ok[out_idx];
			}
			//--------------------------------------------------------------------------------------
			stopWatchStore(OUT_IS_DIFF);

			// resize model size if necessary
			if (cudd_.ReadSize() > model_memory_size)
			{
				while (cudd_.ReadSize() > model_memory_size)
					model_memory_size *= 2;

				char* new_model = (char*) realloc(model, model_memory_size);
				if (new_model == 0)
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
				vector<unsigned> dontcare_c_bits;
				unsigned cj = 0;
				unsigned bit_index = num_of_c_vars - 1;
				BDD blocking_cube = cudd_.bddOne();
				for (int i = first_cj_var; i <= last_cj_var; i++)
				{
					if (model[i] == 1)
					{
						cj |= (1 << bit_index);
						blocking_cube &= c_vars[num_of_c_vars - 1 - bit_index];
					}
					else if (model[i] == 0)
					{
						blocking_cube &= ~c_vars[num_of_c_vars - 1 - bit_index];
					}
					else // input is irrelevant
					{
						dontcare_c_bits.push_back(bit_index);
					}
					bit_index--;
				}
				vector<unsigned> vulnerable_cj_combinations;
				if (dontcare_c_bits.size() == 0) // no bits are irrelevant for satisfiability
				{
					vulnerable_cj_combinations.push_back(cj); // use the concrete value
				}
				else // create all possible c combinations
				{
					for (unsigned concrete_vals = 0;
							concrete_vals < pow(2, dontcare_c_bits.size()); concrete_vals++)
					{
						unsigned concrete_cj = cj;
						for (unsigned i = 0; i < dontcare_c_bits.size(); i++)
						{
							if ((concrete_vals & (1 << i)) > 0) // bit set in combination?
								concrete_cj |= (1 << dontcare_c_bits[i]); // set the bit
						}
						vulnerable_cj_combinations.push_back(concrete_cj);
					}
				}

				// find the one and only true f-signal from model
				int flip_timestep = 0;
				bit_index = 0;
				for (int i = first_f_var; i <= last_f_var; i++)
				{
					if (model[i] == 1)
					{
						flip_timestep |= (1 << (num_of_f_vars - 1 - bit_index));
					}
					bit_index++;
				}
//				cout << "flip timestep = " << flip_timestep << endl;
				stopWatchStore(PARSE_MODEL);
				for (unsigned i = 0; i < vulnerable_cj_combinations.size(); i++)
				{
					cj = vulnerable_cj_combinations[i];
//					latch_to_BDD_signal[cj] = cudd.bddZero(); // NO, DON't do this!!

					if (Options::instance().isUseDiagnosticOutput())
					{
						ErrorTrace* trace = new ErrorTrace;

						trace->error_timestep_ = timestep;
						trace->latch_index_ = circuit_->latches[cj].lit;
						trace->flipped_timestep_ = flip_timestep;

						//trace->input_trace_ = testcase;
						TestCase &real_input_values = trace->input_trace_;
						real_input_values.reserve(real_cnf_inputs.size());
						for (TestCase::const_iterator in = real_cnf_inputs.begin();
								in != real_cnf_inputs.end(); ++in)
						{

							vector<int> real_input_vector;
							real_input_vector.reserve(in->size());
							for (vector<int>::const_iterator iv = in->begin(); iv != in->end();
									++iv)
							{
								if (*iv == CNF_FALSE)
									real_input_vector.push_back(AIG_FALSE);
								else if (*iv == CNF_TRUE)
									real_input_vector.push_back(AIG_TRUE);
								else
									real_input_vector.push_back(AIG_FALSE); // set don't cares to false
									//real_input_vector.push_back(model[*iv]);
							}
							real_input_values.push_back(real_input_vector);
						}

						ErrorTraceManager::instance().error_traces_.push_back(trace);
					}

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
	if (!useStatistics_)
		return;

	start_time_ = Stopwatch::start();
}

void BddAnalysis::stopWatchStore(Statistic statistic)
{
	if (!useStatistics_)
		return;

	map<Statistic, double>::iterator it = accumulated_durations_.find(statistic);
	double duration = Stopwatch::getCPUTimeMilliSec(start_time_);
	if (it != accumulated_durations_.end())
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
	if (!useStatistics_)
		return;

	const string stat_name[] =
	{ "CREATE_C_SIGNALS", "SIM_ANDs", "SWITCH_NXT_ST", "OUT_IS_DIFF", "SATISFIABILITY",
			"STORE_MODEL", "INIT_Latches", "SIDE_CONSTRAINTS", "MODIFY_LATCHES", "PARSE_MODEL" };

	double total_time = Stopwatch::getCPUTimeMilliSec(begin);
	double in_stats = 0.0;
	L_DBG(endl << "--------------------------")
	L_DBG("Total execution time: " << total_time);
	map<Statistic, double>::iterator it = accumulated_durations_.begin();
	for (; it != accumulated_durations_.end(); ++it)
	{
		L_DBG(stat_name[it->first] <<" " << it->second);
		in_stats += it->second;
	}
	L_DBG("uncategorized " << (total_time - in_stats));
}
