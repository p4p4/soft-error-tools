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

void DefinitelyProtected::analyze()
{
	detected_latches_.clear();

	if (mode_ == DefinitelyProtected::STANDARD)
		findDefinitelyProtected_1();
	else if (mode_ == 1)
		findDefinitelyProtected_2();
	else if (mode_ == 2)
		findDefinitelyProtected_3();
	else if (mode_ == 3)
		findDefinitelyProtected_4();
	else if (mode_ == 4)
		findDefinitelyProtected_5();
	else
		MASSERT(false, "unknown mode!");

}

void DefinitelyProtected::findDefinitelyProtected_1()
{
	// ---------------- BEGIN 'for each latch' -------------------------
	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_, num_err_latches_);
	for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
	{
		unsigned component_cnf = latches_to_check[l_cnt] >> 1;
		int next_free_cnf_var = 2;
		SatSolver* solver_ = Options::instance().getSATSolver();
		vector<int> vars_to_keep; // empty
		solver_->startIncrementalSession(vars_to_keep);
		SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);

		// literals for the initial state (x)
		sim_symb.setStateValuesOpen();

		// literals for the inputs at the first time-step (i)
		sim_symb.setInputValuesOpen();

		// compute transition relation T(x,i,o,a,x')
		sim_symb.simulateOneTimeStep();

		// additional time steps
		unsigned num_steps = 1;
		for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
		{

			// x := x'
			sim_symb.switchToNextState();

			// fresh literals for the inputs at the cnt-th time-step (i)
			sim_symb.setInputValuesOpen();

			// conjunct relation T(x, i, o, x') for current time-step
			sim_symb.simulateOneTimeStep();
		}

		test_single_latch(solver_, sim_symb, next_free_cnf_var, latches_to_check[l_cnt]);

		delete solver_;
	}

}


void DefinitelyProtected::findDefinitelyProtected_2()
{
	int next_free_cnf_var = 2;
	SatSolver* solver = Options::instance().getSATSolver();
	vector<int> vars_to_keep; // empty
	solver->startIncrementalSession(vars_to_keep);
	SymbolicSimulator sim_symb(circuit_, solver, next_free_cnf_var);

	// literals for the initial state (x)
	sim_symb.setStateValuesOpen();

	// literals for the inputs at the first time-step (i)
	sim_symb.setInputValuesOpen();

	// compute transition relation T(x,i,o,a,x')
	sim_symb.simulateOneTimeStep();

	// additional time steps
	unsigned num_steps = 1;
	for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
	{
		sim_symb.switchToNextState();
		for(unsigned i = 0; i < circuit_->num_inputs; i++)
		{
			solver->addVarToKeep(next_free_cnf_var);
			sim_symb.setResultValue(circuit_->inputs[i].lit/2, next_free_cnf_var++);
		}

		sim_symb.simulateOneTimeStep();
	}

	int next_free_cnf_var_after_first_step = next_free_cnf_var;
	solver->incPush();
	vector<int> results_bakcup = sim_symb.getResults();

	// ---------------- BEGIN 'for each latch' -------------------------
	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_, num_err_latches_);
	for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
	{
		test_single_latch(solver, sim_symb, next_free_cnf_var, latches_to_check[l_cnt]);

		// reset solver session
		solver->incPop();
		solver->incPush();
		sim_symb.setResults(results_bakcup);
		next_free_cnf_var = next_free_cnf_var_after_first_step;
	}

	delete solver;

}

void DefinitelyProtected::findDefinitelyProtected_3()
{
	int next_free_cnf_var = 2;
	SatSolver* solver = Options::instance().getSATSolver();
	vector<int> vars_to_keep; // empty
	solver->startIncrementalSession(vars_to_keep);
	SymbolicSimulator sim_symb(circuit_, solver, next_free_cnf_var);

	// literals for the initial state (x)
	sim_symb.setStateValuesOpen();

	// literals for the inputs at the first time-step (i)
	sim_symb.setInputValuesOpen();

	// compute transition relation T(x,i,o,a,x')
	sim_symb.simulateOneTimeStep();

	// additional time steps
	unsigned num_steps = 1;
	for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
	{
		sim_symb.switchToNextState();
		for(unsigned i = 0; i < circuit_->num_inputs; i++)
		{
			solver->addVarToKeep(next_free_cnf_var);
			sim_symb.setResultValue(circuit_->inputs[i].lit/2, next_free_cnf_var++);
		}

		sim_symb.simulateOneTimeStep();
	}


	//---------------------------------------
	vector<int> next_state_normal = sim_symb.getNextLatchValues();
	vector<int> outpus_normal = sim_symb.getOutputValues();

	//------------------------------------------------------------------------------------------
	// set up ci signals
	// each latch has a corresponding cj literal that indicates whether the latch is flipped or not.
	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_, num_err_latches_);
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)
	vector<int> c_vars;
	for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
	{
		detected_latches_.insert(latches_to_check[l_cnt]); // first insert all
		int latch_cnf = latches_to_check[l_cnt] >> 1;
		int cj = next_free_cnf_var++;
		c_vars.push_back(cj);
		solver->addVarToKeep(cj);
		cj_to_latch[cj] = latches_to_check[l_cnt];

		// add multiplexer to flip the latch
		int old_value = sim_symb.getResultValue(latch_cnf);
		int new_value = next_free_cnf_var++;
		sim_symb.setResultValue(latch_cnf, new_value);
		solver->addVarToKeep(new_value);
		// new_value == cj ? -old_value : old_value
		solver->incAdd3LitClause(cj, old_value, -new_value);
		solver->incAdd3LitClause(cj, -old_value, new_value);
		solver->incAdd3LitClause(-cj, old_value, new_value);
		solver->incAdd3LitClause(-cj, -old_value, -new_value);

	}

	// require that one c-variable is true
	// TODO: this should not be necessary, right?
	solver->incAddClause(c_vars);

	//--------------------------------------------------------------------------------------
	// single fault assumption: there might be at most one flipped component
	// if c is true, all other c must be false (ci -> -c1, ci -> -c2, ...)
	for (unsigned i = 0; i < c_vars.size(); i++)
	{
		for (int j = i + 1; j < c_vars.size(); j++)
			solver->incAdd2LitClause(-c_vars[i], -c_vars[j]);
	}

	// T_err(x,i,c,o,x'): faulty transition relation where latches can be flipped by SAT-solver
	sim_symb.simulateOneTimeStep();

	vector<int> next_state_flip = sim_symb.getNextLatchValues();
	vector<int> outpus_flip = sim_symb.getOutputValues();

	// we only care about situations where the alarm is false
	solver->incAddUnitClause(-sim_symb.getAlarmValue()); // no alarm


	// create clauses saying that
	//		(next_state_normal != next_state_flip) OR (outpus_normal != outpus_flip)
	vector<int> output_or_next_state_different_clause;

	for(unsigned i = 0; i < circuit_->num_outputs -1; i++)
	{
		solver->addVarToKeep(next_free_cnf_var);
		int out_is_diff_enable = next_free_cnf_var++;
		output_or_next_state_different_clause.push_back(-out_is_diff_enable);
		solver->incAdd3LitClause(out_is_diff_enable, outpus_normal[i], outpus_flip[i]);
		solver->incAdd3LitClause(out_is_diff_enable, -outpus_normal[i], -outpus_flip[i]);
	}

	for(unsigned i = 0; i < circuit_->num_latches - num_err_latches_; i++)
	{
		solver->addVarToKeep(next_free_cnf_var);
		int state_is_diff_enable = next_free_cnf_var++;
		output_or_next_state_different_clause.push_back(-state_is_diff_enable);
		solver->incAdd3LitClause(state_is_diff_enable, next_state_normal[i], next_state_flip[i]);
		solver->incAdd3LitClause(state_is_diff_enable, -next_state_normal[i], -next_state_flip[i]);
	}

	solver->incAddClause(output_or_next_state_different_clause);

	vector<int> no_assumptions; // empty
	vector<int> model;
	while (solver->incIsSatModelOrCore(no_assumptions, c_vars, model))
	{
		Utils::debugPrint(model, "model:");
		for (unsigned i = 0; i < model.size(); i++)
		{
			int c = model[i];
			if (c > 0) // found the c with positive face
			{
				solver->incAddUnitClause(-c);
				detected_latches_.erase(cj_to_latch[c]);
				L_DBG("latch " << cj_to_latch[c] << " not n-step protected")
			}
		}
	}

	delete solver;
}


void DefinitelyProtected::findDefinitelyProtected_4()
{

	unsigned k_steps = 3; // TODO: use parameter
	int next_free_cnf_var = 2;
	SatSolver* solver = Options::instance().getSATSolver();
	vector<int> vars_to_keep; // empty
	solver->startIncrementalSession(vars_to_keep);
	SymbolicSimulator sim_ok(circuit_, solver, next_free_cnf_var);

	// literals for the initial state (x)
	sim_ok.setStateValuesOpen();

	// literals for the inputs at the first time-step (i)
	sim_ok.setInputValuesOpen();

	// compute transition relation T(x,i,o,a,x')
	sim_ok.simulateOneTimeStep();

	// additional time steps
	unsigned num_steps = 1;
	for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
	{
		sim_ok.switchToNextState();
		for(unsigned i = 0; i < circuit_->num_inputs; i++)
		{
			solver->addVarToKeep(next_free_cnf_var);
			sim_ok.setResultValue(circuit_->inputs[i].lit/2, next_free_cnf_var++);
		}

		sim_ok.simulateOneTimeStep();
	}

	int next_free_cnf_var_after_first_step = next_free_cnf_var;
	solver->incPush();
	vector<int> results_fault_free = sim_ok.getResults();

	// ---------------- BEGIN 'for each latch' -------------------------
	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_, num_err_latches_);
	for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
	{


		unsigned latch_aig = latches_to_check[l_cnt];
		int component_cnf = latch_aig >> 1;

		SymbolicSimulator sim_faulty(circuit_, solver, next_free_cnf_var);
		sim_faulty.setResults(results_fault_free);

		vector<int> alarms_in_flipped_version_at_step_i; // maps i -> alarm signal of faulty verstion at step i
		vector<int> output_different_at_step_i; // maps i -> different_at_i
		int final_nxt_state_diff_enable = next_free_cnf_var++;

		for (unsigned i = 0; i < k_steps; i++)
		{
			if (i == 0) // flip in first step
			{
				sim_faulty.setResultValue(component_cnf, -sim_ok.getResultValue(component_cnf));
			}

			// set same open input values for both copies
			sim_ok.setInputValuesOpen();
			sim_faulty.setInputValues(sim_ok.getInputValues());

			// create transition relations and add to solver
			sim_faulty.simulateOneTimeStep();	// T_ok(x,i,o,a,x')
			sim_ok.simulateOneTimeStep();		// T_err(x_e,i,o_e,a_e,x'e)

			// store a_e
			alarms_in_flipped_version_at_step_i.push_back(sim_faulty.getAlarmValue());

			// require no alarm in fault-free version
			solver->incAddUnitClause(-sim_ok.getAlarmValue()); // -a

			// create clause saying that output is different
			vector<int> outputs_ok = sim_ok.getOutputValues();		// o
			vector<int> outpus_flip = sim_faulty.getOutputValues(); // o_e

			int output_different_now_enable = next_free_cnf_var++;
			output_different_at_step_i.push_back(output_different_now_enable);
			vector<int> output_different_now_clause;
			output_different_now_clause.push_back(-output_different_now_enable);
			for(unsigned i = 0; i < circuit_->num_outputs -1; i++)
			{
				solver->addVarToKeep(next_free_cnf_var);
				int out_is_diff_enable = next_free_cnf_var++; // o_i_diff
				output_different_now_clause.push_back(out_is_diff_enable);

				// o_i_diff <-> (o_i_ok != o_i_faulty)
				solver->incAdd3LitClause(-out_is_diff_enable, outputs_ok[i], outpus_flip[i]);
				solver->incAdd3LitClause(-out_is_diff_enable, -outputs_ok[i], -outpus_flip[i]);
			}
			// (disabled OR o1_diff OR o2_diff OR ... OR on_diff):
			solver->incAddClause(output_different_now_clause);

			if (i == k_steps - 1) // last step
			{
				// clause saying next state is still different (x' != x_e'):
				vector<int> next_ok = sim_ok.getNextLatchValues();			// x'
				vector<int> next_faulty = sim_faulty.getNextLatchValues();	// x_e'
				// (x'1_diff OR x'2_diff OR ... OR x'n_diff):

				vector<int> next_state_different;
				next_state_different.push_back(-final_nxt_state_diff_enable);
				for(unsigned i = 0; i < circuit_->num_latches - num_err_latches_; i++)
				{
					solver->addVarToKeep(next_free_cnf_var);
					int state_is_diff_enable = next_free_cnf_var++; // x'i_diff
					next_state_different.push_back(state_is_diff_enable);

					// x'i_diff <-> (x'i_ok != x'i_faulty)
					solver->incAdd3LitClause(-state_is_diff_enable, next_ok[i], next_faulty[i]);
					solver->incAdd3LitClause(-state_is_diff_enable, -next_ok[i], -next_faulty[i]);
				}

			}
			else // has further steps
			{
				sim_ok.switchToNextState();
				sim_faulty.switchToNextState();
			}

		}

		Utils::debugPrint(alarms_in_flipped_version_at_step_i, "alarm outputs");

		// maps a literal that is true if there was no alarm set to true until time step k
		vector<int> no_alarm_until;
		no_alarm_until.push_back(-alarms_in_flipped_version_at_step_i[0]); // i=0: not a_0
		for (unsigned i = 1; i < alarms_in_flipped_version_at_step_i.size(); i++)
		{
			if(no_alarm_until[i-1] == CNF_FALSE)
			{
				no_alarm_until.push_back(CNF_FALSE);
				continue;
			}
			// no_alarm_until[i] <-> no_alarm_until_[i-1] AND not a_i:
			int no_alarm_until_k = next_free_cnf_var++;
			no_alarm_until.push_back(no_alarm_until_k);

			solver->incAdd2LitClause(-no_alarm_until_k, no_alarm_until[i-1]);
			solver->incAdd2LitClause(-no_alarm_until_k, -alarms_in_flipped_version_at_step_i[i]);
			solver->incAdd3LitClause(no_alarm_until_k, -no_alarm_until[i-1], alarms_in_flipped_version_at_step_i[i]);
		}

		// create final clause for sat solver call:
		// there exists a point in time where the outputs are different and alarm has not been raised so far,
		// or the next state of the last iteration is different without raising an alarm:
		// 		_OR_ for i = 1..k-1: (no_alarm_until[i] AND o_i != o_e_i)
		// 		special case: i = k: (no_alarm_until[k] AND ((o_k != o_e_k) OR (x'_k != x'_e_k)) )
		vector<int> possibly_vulnerable;
		for (unsigned i = 0; i < output_different_at_step_i.size(); i++)
		{


			int problem_at_i;
			if (i == output_different_at_step_i.size() - 1) // last step special case
			{
				// problem_at_i <-> ((o_k != o_e_k) OR (x'_k != x'_e_k))
				int out_or_next_different = next_free_cnf_var++;
				problem_at_i = out_or_next_different;
				solver->incAdd2LitClause(out_or_next_different, -output_different_at_step_i[i]);
				solver->incAdd2LitClause(out_or_next_different, -final_nxt_state_diff_enable);
				solver->incAdd3LitClause(-out_or_next_different, output_different_at_step_i[i], final_nxt_state_diff_enable);
			}
			else
			{
				//problem_at_i <-> (o_i != o_e_i):
				problem_at_i = output_different_at_step_i[i];
			}

			int error_at_i = next_free_cnf_var++;
			possibly_vulnerable.push_back(error_at_i);


			solver->incAdd2LitClause(-error_at_i, no_alarm_until[i]);
			solver->incAdd2LitClause(-error_at_i, problem_at_i);
			solver->incAdd3LitClause(error_at_i, -no_alarm_until[i], -problem_at_i);
		}

		solver->incAddClause(possibly_vulnerable);

		vector<int> no_assumptions;
		vector<int> model;

		vector<int> interest;
		interest.push_back(final_nxt_state_diff_enable);
		interest.insert(interest.end(), output_different_at_step_i.begin(), output_different_at_step_i.end());
		interest.insert(interest.end(), no_alarm_until.begin(), no_alarm_until.end());

		Utils::debugPrint(output_different_at_step_i, "output_different_at_step_i");
		Utils::debugPrint(no_alarm_until, "no_alarm_until");
		if(solver->incIsSatModelOrCore(no_assumptions, interest, model) == false)
		{
			detected_latches_.insert(latch_aig);
			L_DBG("Definitely protected latch "<< latch_aig << " found. (UNSAT)");
		}
		else
		{
			Utils::debugPrint(model, "model");
			L_DBG("SAT " << latch_aig << "(not k-step protected)")
		}

		L_DBG(" ");

		// reset solver session
		solver->incPop();
		solver->incPush();
		sim_ok.setResults(results_fault_free);
		next_free_cnf_var = next_free_cnf_var_after_first_step;
	}

	delete solver;

}

void DefinitelyProtected::findDefinitelyProtected_5()
{
	unsigned k_steps = 3; // TODO: use parameter
	int next_free_cnf_var = 2;
	SatSolver* solver = Options::instance().getSATSolver();
	vector<int> vars_to_keep; // empty
	solver->startIncrementalSession(vars_to_keep);
	SymbolicSimulator sim_ok(circuit_, solver, next_free_cnf_var);

	// literals for the initial state (x)
	sim_ok.setStateValuesOpen();

	// literals for the inputs at the first time-step (i)
	sim_ok.setInputValuesOpen();

	// compute transition relation T(x,i,o,a,x')
	sim_ok.simulateOneTimeStep();

	// additional time steps
	unsigned num_steps = 1;
	for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
	{
		sim_ok.switchToNextState();
		for(unsigned i = 0; i < circuit_->num_inputs; i++)
		{
			solver->addVarToKeep(next_free_cnf_var);
			sim_ok.setResultValue(circuit_->inputs[i].lit/2, next_free_cnf_var++);
		}

		sim_ok.simulateOneTimeStep();
	}






		SymbolicSimulator sim_faulty(circuit_, solver, next_free_cnf_var);
		sim_faulty.setResults(sim_ok.getResults());

		vector<int> alarms_in_flipped_version_at_step_i; // maps i -> alarm signal of faulty verstion at step i
		vector<int> output_different_at_step_i; // maps i -> different_at_i
		int final_nxt_state_diff_enable = next_free_cnf_var++;


		vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_, num_err_latches_);
						map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)
						vector<int> c_vars;

		for (unsigned i = 0; i < k_steps; i++)
		{
			if (i == 0) // flip in first step
			{
				//------------------------------------------------------------------------------------------
				// set up ci signals
				// each latch has a corresponding cj literal that indicates whether the latch is flipped or not.

				for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
				{
					detected_latches_.insert(latches_to_check[l_cnt]); // first insert all
					int latch_cnf = latches_to_check[l_cnt] >> 1;
					int cj = next_free_cnf_var++;
					c_vars.push_back(cj);
					solver->addVarToKeep(cj);
					cj_to_latch[cj] = latches_to_check[l_cnt];

					// add multiplexer to flip the latch
					int old_value = sim_faulty.getResultValue(latch_cnf);
					int new_value = next_free_cnf_var++;
					sim_faulty.setResultValue(latch_cnf, new_value);
					solver->addVarToKeep(new_value);
					// new_value == cj ? -old_value : old_value
					solver->incAdd3LitClause(cj, old_value, -new_value);
					solver->incAdd3LitClause(cj, -old_value, new_value);
					solver->incAdd3LitClause(-cj, old_value, new_value);
					solver->incAdd3LitClause(-cj, -old_value, -new_value);

				}

				// require that one c-variable is true
				// TODO: this should not be necessary, right?
				solver->incAddClause(c_vars);

				//--------------------------------------------------------------------------------------
				// single fault assumption: there might be at most one flipped component
				// if c is true, all other c must be false (ci -> -c1, ci -> -c2, ...)
				for (unsigned i = 0; i < c_vars.size(); i++)
				{
					for (int j = i + 1; j < c_vars.size(); j++)
						solver->incAdd2LitClause(-c_vars[i], -c_vars[j]);
				}
			}

			// set same open input values for both copies
			sim_ok.setInputValuesOpen();
			sim_faulty.setInputValues(sim_ok.getInputValues());

			// create transition relations and add to solver
			sim_faulty.simulateOneTimeStep();	// T_ok(x,i,o,a,x')
			sim_ok.simulateOneTimeStep();		// T_err(x_e,i,o_e,a_e,x'e)

			// store a_e
			alarms_in_flipped_version_at_step_i.push_back(sim_faulty.getAlarmValue());

			// require no alarm in fault-free version
			solver->incAddUnitClause(-sim_ok.getAlarmValue()); // -a

			// create clause saying that output is different
			vector<int> outputs_ok = sim_ok.getOutputValues();		// o
			vector<int> outpus_flip = sim_faulty.getOutputValues(); // o_e

			int output_different_now_enable = next_free_cnf_var++;
			output_different_at_step_i.push_back(output_different_now_enable);
			vector<int> output_different_now_clause;
			output_different_now_clause.push_back(-output_different_now_enable);
			for(unsigned i = 0; i < circuit_->num_outputs -1; i++)
			{
				solver->addVarToKeep(next_free_cnf_var);
				int out_is_diff_enable = next_free_cnf_var++; // o_i_diff
				output_different_now_clause.push_back(out_is_diff_enable);

				// o_i_diff <-> (o_i_ok != o_i_faulty)
				solver->incAdd3LitClause(-out_is_diff_enable, outputs_ok[i], outpus_flip[i]);
				solver->incAdd3LitClause(-out_is_diff_enable, -outputs_ok[i], -outpus_flip[i]);
			}
			// (disabled OR o1_diff OR o2_diff OR ... OR on_diff):
			solver->incAddClause(output_different_now_clause);

			if (i == k_steps - 1) // last step
			{
				// clause saying next state is still different (x' != x_e'):
				vector<int> next_ok = sim_ok.getNextLatchValues();			// x'
				vector<int> next_faulty = sim_faulty.getNextLatchValues();	// x_e'
				// (x'1_diff OR x'2_diff OR ... OR x'n_diff):

				vector<int> next_state_different;
				next_state_different.push_back(-final_nxt_state_diff_enable);
				for(unsigned i = 0; i < circuit_->num_latches - num_err_latches_; i++)
				{
					solver->addVarToKeep(next_free_cnf_var);
					int state_is_diff_enable = next_free_cnf_var++; // x'i_diff
					next_state_different.push_back(state_is_diff_enable);

					// x'i_diff <-> (x'i_ok != x'i_faulty)
					solver->incAdd3LitClause(-state_is_diff_enable, next_ok[i], next_faulty[i]);
					solver->incAdd3LitClause(-state_is_diff_enable, -next_ok[i], -next_faulty[i]);
				}

			}
			else // has further steps
			{
				sim_ok.switchToNextState();
				sim_faulty.switchToNextState();
			}

		}

		Utils::debugPrint(alarms_in_flipped_version_at_step_i, "alarm outputs");

		// maps a literal that is true if there was no alarm set to true until time step k
		vector<int> no_alarm_until;
		no_alarm_until.push_back(-alarms_in_flipped_version_at_step_i[0]); // i=0: not a_0
		for (unsigned i = 1; i < alarms_in_flipped_version_at_step_i.size(); i++)
		{
			if(no_alarm_until[i-1] == CNF_FALSE)
			{
				no_alarm_until.push_back(CNF_FALSE);
				continue;
			}
			// no_alarm_until[i] <-> no_alarm_until_[i-1] AND not a_i:
			int no_alarm_until_k = next_free_cnf_var++;
			no_alarm_until.push_back(no_alarm_until_k);

			solver->incAdd2LitClause(-no_alarm_until_k, no_alarm_until[i-1]);
			solver->incAdd2LitClause(-no_alarm_until_k, -alarms_in_flipped_version_at_step_i[i]);
			solver->incAdd3LitClause(no_alarm_until_k, -no_alarm_until[i-1], alarms_in_flipped_version_at_step_i[i]);
		}

		// create final clause for sat solver call:
		// there exists a point in time where the outputs are different and alarm has not been raised so far,
		// or the next state of the last iteration is different without raising an alarm:
		// 		_OR_ for i = 1..k-1: (no_alarm_until[i] AND o_i != o_e_i)
		// 		special case: i = k: (no_alarm_until[k] AND ((o_k != o_e_k) OR (x'_k != x'_e_k)) )
		vector<int> possibly_vulnerable;
		for (unsigned i = 0; i < output_different_at_step_i.size(); i++)
		{


			int problem_at_i;
			if (i == output_different_at_step_i.size() - 1) // last step special case
			{
				// problem_at_i <-> ((o_k != o_e_k) OR (x'_k != x'_e_k))
				int out_or_next_different = next_free_cnf_var++;
				problem_at_i = out_or_next_different;
				solver->incAdd2LitClause(out_or_next_different, -output_different_at_step_i[i]);
				solver->incAdd2LitClause(out_or_next_different, -final_nxt_state_diff_enable);
				solver->incAdd3LitClause(-out_or_next_different, output_different_at_step_i[i], final_nxt_state_diff_enable);
			}
			else
			{
				//problem_at_i <-> (o_i != o_e_i):
				problem_at_i = output_different_at_step_i[i];
			}

			int error_at_i = next_free_cnf_var++;
			possibly_vulnerable.push_back(error_at_i);


			solver->incAdd2LitClause(-error_at_i, no_alarm_until[i]);
			solver->incAdd2LitClause(-error_at_i, problem_at_i);
			solver->incAdd3LitClause(error_at_i, -no_alarm_until[i], -problem_at_i);
		}

		solver->incAddClause(possibly_vulnerable);

		vector<int> no_assumptions; // empty
		vector<int> model;
		while (solver->incIsSatModelOrCore(no_assumptions, c_vars, model))
		{
			Utils::debugPrint(model, "model:");
			for (unsigned i = 0; i < model.size(); i++)
			{
				int c = model[i];
				if (c > 0) // found the c with positive face
				{
					solver->incAddUnitClause(-c);
					detected_latches_.erase(cj_to_latch[c]);
					L_DBG("latch " << cj_to_latch[c] << " not n-step protected")
				}
			}
		}




	delete solver;
}

void DefinitelyProtected::test_single_latch(SatSolver* solver, SymbolicSimulator& sim_symb, int& next_free_cnf_var, unsigned latch_aig)
{

	vector<int> next_state_normal = sim_symb.getNextLatchValues();
	vector<int> outpus_normal = sim_symb.getOutputValues();

	//--------------------

	int component_cnf = latch_aig >> 1;

	// compute faulty transition relation
	sim_symb.setResultValue(component_cnf, -sim_symb.getResultValue(component_cnf)); // flip latch
	sim_symb.simulateOneTimeStep();

	vector<int> next_state_flip = sim_symb.getNextLatchValues();
	vector<int> outpus_flip = sim_symb.getOutputValues();

	// we only care about situations where the alarm is false
	solver->incAddUnitClause(-sim_symb.getAlarmValue()); // no alarm


	// create clauses saying that
	//		(next_state_normal != next_state_flip) OR (outpus_normal != outpus_flip)
	vector<int> output_or_next_state_different_clause;

	for(unsigned i = 0; i < circuit_->num_outputs -1; i++)
	{
		solver->addVarToKeep(next_free_cnf_var);
		int out_is_diff_enable = next_free_cnf_var++;
		output_or_next_state_different_clause.push_back(-out_is_diff_enable);
		solver->incAdd3LitClause(out_is_diff_enable, outpus_normal[i], outpus_flip[i]);
		solver->incAdd3LitClause(out_is_diff_enable, -outpus_normal[i], -outpus_flip[i]);
	}

	for(unsigned i = 0; i < circuit_->num_latches - num_err_latches_; i++)
	{
		solver->addVarToKeep(next_free_cnf_var);
		int state_is_diff_enable = next_free_cnf_var++;
		output_or_next_state_different_clause.push_back(-state_is_diff_enable);
		solver->incAdd3LitClause(state_is_diff_enable, next_state_normal[i], next_state_flip[i]);
		solver->incAdd3LitClause(state_is_diff_enable, -next_state_normal[i], -next_state_flip[i]);
	}

	solver->incAddClause(output_or_next_state_different_clause);



	vector<int> no_assumptions;
	vector<int> model;

	vector<int> interest = sim_symb.getInputValues();

	if(solver->incIsSatModelOrCore(no_assumptions, interest, model) == false)
	{
		detected_latches_.insert(latch_aig);
		L_DBG("Definitely protected latch "<< latch_aig << " found. (UNSAT)");
	}
	else
	{
		L_DBG("SAT " << latch_aig << "(not n-step protected)")

		Utils::debugPrint(model, "Model: ");
	}
}

void DefinitelyProtected::printResults()
{
	float percentage = (float) detected_latches_.size() / Options::instance().getNumberOfLatchesToCheck() * 100;
	L_LOG("#Definitely protected latches found: " << detected_latches_.size() << " (" << percentage << " %)");

	if (Options::instance().isUseDiagnosticOutput())
	{
		ostringstream oss;
		if (detected_latches_.size() > 0)
			oss << "Definitely protected latches:" << endl;

		int i = 0;
		for(set<unsigned>::iterator it = detected_latches_.begin(); it != detected_latches_.end(); ++it)
		{
			oss << *it << "\t";

			if ((i+1)%5 == 0 && i < detected_latches_.size() - 1)
				oss << endl;

			i++;
		}
		if (Options::instance().isDiagnosticOutputToFile())
		{
		  ofstream out_file;
		  out_file.open(Options::instance().getDiagnosticOutputPath().c_str());
			MASSERT(out_file,
					"could not write diagnostic output file: " + Options::instance().getDiagnosticOutputPath())

		  out_file << oss.str() << endl;

		  out_file.close();
		}
		else
		{
			cout << endl << oss.str() << endl;
		}
	}
}
