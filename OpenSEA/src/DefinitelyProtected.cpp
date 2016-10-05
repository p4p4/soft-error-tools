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

#include "CnfUtils.h"
#include "SatSolver.h"
#include "AigSimulator.h"
#include "SymbolicSimulator.h"
#include "Options.h"
#include "Utils.h"
#include "Logger.h"
#include "AndCacheMap.h"
#include "SatAssignmentParser.h"

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
		findDefinitelyProtected_1step_deprecated();
	else if (mode_ == 1)
		findDefinitelyProtected_1step_single();
	else if (mode_ == 2)
		findDefinitelyProtected_1step_simultaneously();
	else if (mode_ == 3)
		findDefinitelyProtected_kstep_single_latch();
	else if (mode_ == 4)
		findDefinitelyProtected_kstep_simultaneously();
	else
		MASSERT(false, "unknown mode!");

}

void DefinitelyProtected::computeInitialTransitionRelation(SatSolver* solver,
		SymbolicSimulator& sim_symb, unsigned num_steps)
{
	// first time steps T(x,i,o,a,x') & -a
	sim_symb.setStateValuesOpen();
	for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
	{
		sim_symb.setInputValuesOpen();
		sim_symb.simulateOneTimeStep();
		solver->incAddUnitClause(-sim_symb.getAlarmValue());
		if (s_cnt < num_steps - 1)
			sim_symb.switchToNextState();
	}
}

void DefinitelyProtected::findDefinitelyProtected_1step_deprecated()
{
	// ---------------- BEGIN 'for each latch' -------------------------
	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_,
			num_err_latches_);
	for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
	{
		int next_free_cnf_var = 2;
		SatSolver* solver_ = Options::instance().getSATSolver();
		vector<int> vars_to_keep; // empty
		solver_->startIncrementalSession(vars_to_keep);
		SymbolicSimulator sim_symb(circuit_, solver_, next_free_cnf_var);

		// first time steps T(x,i,o,a,x') & -a
		unsigned num_initial_steps = Options::instance().getDefinitevelyProtectedNumInitialSteps();
		computeInitialTransitionRelation(solver_, sim_symb, num_initial_steps);

		test_single_latch(solver_, sim_symb, next_free_cnf_var, latches_to_check[l_cnt]);

		delete solver_;
	}

}

void DefinitelyProtected::findDefinitelyProtected_1step_single()
{
	int next_free_cnf_var = 2;
	SatSolver* solver = Options::instance().getSATSolver();
	vector<int> vars_to_keep; // empty
	solver->startIncrementalSession(vars_to_keep);
	SymbolicSimulator sim_symb(circuit_, solver, next_free_cnf_var);

	// first time steps T(x,i,o,a,x') & -a
	unsigned num_initial_steps = Options::instance().getDefinitevelyProtectedNumInitialSteps();
	computeInitialTransitionRelation(solver, sim_symb, num_initial_steps);

	// store solver session
	int next_free_cnf_var_after_first_step = next_free_cnf_var;
	solver->incPush();
	vector<int> results_backup = sim_symb.getResults();

	// ---------------- BEGIN 'for each latch' -------------------------
	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_,
			num_err_latches_);
	for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
	{
		test_single_latch(solver, sim_symb, next_free_cnf_var, latches_to_check[l_cnt]);

		// reset solver session
		solver->incPop();
		solver->incPush();
		next_free_cnf_var = next_free_cnf_var_after_first_step;
		sim_symb.setResults(results_backup);
	}

	delete solver;

}


void DefinitelyProtected::test_single_latch(SatSolver* solver, SymbolicSimulator& sim_symb,
		int& next_free_cnf_var, unsigned latch_aig)
{
	sim_symb.simulateOneTimeStep();
	vector<int> next_state_normal = sim_symb.getNextLatchValues();
	vector<int> outputs_normal = sim_symb.getOutputValues();

	int component_cnf = latch_aig >> 1;

	// compute faulty transition relation
	sim_symb.setResultValue(component_cnf, -sim_symb.getResultValue(component_cnf)); // flip latch
	sim_symb.simulateOneTimeStep();

	vector<int> next_state_flip = sim_symb.getNextLatchValues();
	vector<int> outputs_flip = sim_symb.getOutputValues();

	// we only care about situations where the alarm is false
	solver->incAddUnitClause(-sim_symb.getAlarmValue()); // no alarm

	// create clauses saying that
	//		(next_state_normal != next_state_flip) OR (outpus_normal != outpus_flip)
	vector<int> output_or_next_state_different_clause;

	// exclude alarm output
	outputs_normal.pop_back();
	outputs_flip.pop_back();
	CnfUtils::generateVectorIsDifferentClause(outputs_normal, outputs_flip, output_or_next_state_different_clause, next_free_cnf_var, solver);

	// exclude the last 'num_err_latches_' latches (if any)
	next_state_normal.erase(next_state_normal.end() - num_err_latches_, next_state_normal.end());
	next_state_flip.erase(next_state_flip.end() - num_err_latches_, next_state_flip.end());
	CnfUtils::generateVectorIsDifferentClause(next_state_normal, next_state_flip, output_or_next_state_different_clause, next_free_cnf_var, solver);

	solver->incAddClause(output_or_next_state_different_clause);

	vector<int> no_assumptions;
	vector<int> model;

	if (solver->incIsSatModelOrCore(no_assumptions, no_assumptions, model) == false)
	{
		detected_latches_.insert(latch_aig);
		L_DBG("Definitely protected latch "<< latch_aig << " found. (UNSAT)");
	}
	else
	{
		L_DBG("SAT " << latch_aig << "(not n-step protected)")
	}
}

void DefinitelyProtected::findDefinitelyProtected_1step_simultaneously()
{

	SatAssignmentParser parser;

	int next_free_cnf_var = 2;
	SatSolver* solver = Options::instance().getSATSolver();
	vector<int> vars_to_keep; // empty
	solver->startIncrementalSession(vars_to_keep);
	SymbolicSimulator sim_symb(circuit_, solver, next_free_cnf_var);

	// first time steps T(x,i,o,a,x') & -a
	unsigned num_initial_steps = Options::instance().getDefinitevelyProtectedNumInitialSteps();
	computeInitialTransitionRelation(solver, sim_symb, num_initial_steps);

	vector<int> next_state_normal = sim_symb.getNextLatchValues();
	vector<int> outpus_normal = sim_symb.getOutputValues();

	// set up ci signals
	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_,
			num_err_latches_);
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)
	vector<int> c_vars;

	// T_err(x,i,c,o,a,x') & -a:
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
		if (old_value == CNF_TRUE) // old value is true
			sim_symb.setResultValue(latch_cnf, -cj);
		else if (old_value == CNF_FALSE) // old value is false
			sim_symb.setResultValue(latch_cnf, cj);
		else
		{
			int new_value = next_free_cnf_var++;
			solver->addVarToKeep(new_value);
			// new_value == cj ? -old_value : old_value
			solver->incAdd3LitClause(cj, old_value, -new_value);
			solver->incAdd3LitClause(cj, -old_value, new_value);
			solver->incAdd3LitClause(-cj, old_value, new_value);
			solver->incAdd3LitClause(-cj, -old_value, -new_value);
			sim_symb.setResultValue(latch_cnf, new_value);
		}

	}

	sim_symb.simulateOneTimeStep();
	solver->incAddUnitClause(-sim_symb.getAlarmValue());

	//--------------------------------------------------------------------------------------
	// single fault assumption: there can be at most one flipped component
	// if c is true, all other c must be false (ci -> -c1, ci -> -c2, ...)
	for (unsigned i = 0; i < c_vars.size(); i++)
	{
		for (unsigned j = i + 1; j < c_vars.size(); j++)
			solver->incAdd2LitClause(-c_vars[i], -c_vars[j]);
	}

	vector<int> next_state_flip = sim_symb.getNextLatchValues();
	vector<int> outpus_flip = sim_symb.getOutputValues();

	//--------------------------------------------------------------------------------------
	// create clauses saying that
	//		(next_state_normal != next_state_flip) OR (outpus_normal != outpus_flip)
	vector<int> output_or_next_state_different_clause;

	// exclude alarm output
	outpus_normal.pop_back();
	outpus_flip.pop_back();
	CnfUtils::generateVectorIsDifferentClause(outpus_normal, outpus_flip,
			output_or_next_state_different_clause, next_free_cnf_var, solver);

	// exclude the last 'num_err_latches_' latches (if any)
	next_state_normal.erase(next_state_normal.end() - num_err_latches_, next_state_normal.end());
	next_state_flip.erase(next_state_flip.end() - num_err_latches_, next_state_flip.end());
	CnfUtils::generateVectorIsDifferentClause(next_state_normal, next_state_flip,
			output_or_next_state_different_clause, next_free_cnf_var, solver);

	solver->incAddClause(output_or_next_state_different_clause);

	vector<int> no_assumptions; // empty
	vector<int> model;
	parser.addVectorOfInterest(c_vars, "c_vars");
	while (solver->incIsSatModelOrCore(no_assumptions, parser.getVarsOfInterrest(), model))
	{
		if (Logger::instance().isEnabled(Logger::DBG))
			parser.parseAssignment(model);

		int c_selected = parser.findFirstPositiveAssignmentOfVector(model, c_vars);
		MASSERT(c_selected != 0, "Error: output or next state different without a flip!")

		solver->incAddUnitClause(-c_selected);
		detected_latches_.erase(cj_to_latch[c_selected]);
	}

	delete solver;
}

void DefinitelyProtected::findDefinitelyProtected_kstep_single_latch()
{
	unsigned k_steps = Options::instance().getDefinitivelyProtectedKSteps();
	unsigned num_initial_steps = Options::instance().getDefinitevelyProtectedNumInitialSteps();

	int next_free_cnf_var = 2;
	SatSolver* solver = Options::instance().getSATSolver();
	vector<int> vars_to_keep; // empty
	solver->startIncrementalSession(vars_to_keep);
	SymbolicSimulator sim_ok(circuit_, solver, next_free_cnf_var);

	// first time steps T(x,i,o,a,x') & -a
	computeInitialTransitionRelation(solver, sim_ok, num_initial_steps + 1);

	int final_nxt_state_diff_enable = next_free_cnf_var++;

	// store solver session
	int next_free_cnf_var_after_first_step = next_free_cnf_var;
	solver->incPush();
	vector<int> results_backup = sim_ok.getResults();

	// ---------------- BEGIN 'for each latch' -------------------------
	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_,
			num_err_latches_);
	for (unsigned l_cnt = 0; l_cnt < latches_to_check.size(); ++l_cnt)
	{
		vector<int> alarms_in_flipped_version_at_step_i; // maps i -> alarm signal of faulty verstion at step i
		vector<int> output_different_at_step_i; // maps i -> different_at_i

		SymbolicSimulator sim_faulty(circuit_, solver, next_free_cnf_var);
		unsigned latch_aig = latches_to_check[l_cnt];
		int component_cnf = latch_aig >> 1;

		for (unsigned i = 0; i < k_steps; i++)
		{
			// set same open input values for both copies
			sim_ok.setInputValuesOpen();
			sim_faulty.setCnfInputValues(sim_ok.getInputValues());

			sim_ok.simulateOneTimeStep();	// T_ok(x,i,o,a,x')
			if (i == 0) // flip in first step
			{
				sim_faulty.setResults(sim_ok.getResults());
				sim_faulty.setResultValue(component_cnf, -sim_faulty.getResultValue(component_cnf));
			}
			sim_faulty.simulateOneTimeStep();		// T_err(x_e,i,o_e,a_e,x'e)

			// store a_e
			alarms_in_flipped_version_at_step_i.push_back(sim_faulty.getAlarmValue());

			// require no alarm in fault-free version
			solver->incAddUnitClause(-sim_ok.getAlarmValue()); // -a

			// create clause saying that output is different
			vector<int> outputs_ok = sim_ok.getOutputValues();		// o
			vector<int> outputs_flip = sim_faulty.getOutputValues(); // o_e

			int output_different_now_enable = next_free_cnf_var++;
			output_different_at_step_i.push_back(output_different_now_enable);
			vector<int> output_different_now_clause;
			output_different_now_clause.push_back(-output_different_now_enable);

			// exclude alarm output
			outputs_ok.pop_back();
			outputs_flip.pop_back();
			CnfUtils::generateVectorIsDifferentClause(outputs_ok, outputs_flip,
					output_different_now_clause, next_free_cnf_var, solver);
			// looks like: (disabled OR o1_diff OR o2_diff OR ... OR on_diff):

			solver->incAddClause(output_different_now_clause);

			if (i == k_steps - 1) // last step
			{
				// clause saying next state is still different (x' != x_e'):
				vector<int> next_ok = sim_ok.getNextLatchValues();			// x'
				vector<int> next_faulty = sim_faulty.getNextLatchValues();	// x_e'
				// (x'1_diff OR x'2_diff OR ... OR x'n_diff):

				vector<int> next_state_different;
				next_state_different.push_back(-final_nxt_state_diff_enable);

				// exclude the last 'num_err_latches_' latches (if any)
				next_ok.erase(next_ok.end() - num_err_latches_, next_ok.end());
				next_faulty.erase(next_faulty.end() - num_err_latches_, next_faulty.end());
				CnfUtils::generateVectorIsDifferentClause(next_ok, next_faulty,
						next_state_different, next_free_cnf_var, solver);

				solver->incAddClause(next_state_different);

			}
			else // has further steps
			{
				sim_ok.switchToNextState();
				sim_faulty.switchToNextState();
			}

		}

		// maps a literal that is true if there was no alarm set to true until time step k
		vector<int> no_alarm_until;
		no_alarm_until.push_back(-alarms_in_flipped_version_at_step_i[0]); // i=0: not a_0
		for (unsigned i = 1; i < alarms_in_flipped_version_at_step_i.size(); i++)
		{
			if (no_alarm_until[i - 1] == CNF_FALSE)
			{
				no_alarm_until.push_back(CNF_FALSE);
				continue;
			}
			// no_alarm_until[i] <-> no_alarm_until_[i-1] AND not a_i:
			int no_alarm_until_k = next_free_cnf_var++;
			no_alarm_until.push_back(no_alarm_until_k);

			solver->incAdd2LitClause(-no_alarm_until_k, no_alarm_until[i - 1]);
			solver->incAdd2LitClause(-no_alarm_until_k, -alarms_in_flipped_version_at_step_i[i]);
			solver->incAdd3LitClause(no_alarm_until_k, -no_alarm_until[i - 1],
					alarms_in_flipped_version_at_step_i[i]);
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
				int problem_at_final_step = next_free_cnf_var++;
				problem_at_i = problem_at_final_step;
				solver->incAdd2LitClause(problem_at_final_step, -output_different_at_step_i[i]);
				solver->incAdd2LitClause(problem_at_final_step, -final_nxt_state_diff_enable);
				solver->incAdd3LitClause(-problem_at_final_step, output_different_at_step_i[i],
						final_nxt_state_diff_enable);
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

		if (solver->incIsSat())
		{
			L_DBG("SAT " << latch_aig << "(not k-step protected)")
		}
		else
		{
			detected_latches_.insert(latch_aig);
			L_DBG("Definitely protected latch "<< latch_aig << " found. (UNSAT)");
		}

		// reset solver session
		solver->incPop();
		solver->incPush();
		sim_ok.setResults(results_backup);
		next_free_cnf_var = next_free_cnf_var_after_first_step;
	}
	delete solver;

}

void DefinitelyProtected::findDefinitelyProtected_kstep_simultaneously()
{
	unsigned k_steps = Options::instance().getDefinitivelyProtectedKSteps();
	unsigned num_initial_steps = Options::instance().getDefinitevelyProtectedNumInitialSteps();

	int next_free_cnf_var = 2;
	SatSolver* solver = Options::instance().getSATSolver();
	vector<int> vars_to_keep; // empty
	solver->startIncrementalSession(vars_to_keep);
	SymbolicSimulator sim_ok(circuit_, solver, next_free_cnf_var);

	sim_ok.setStateValuesOpen();
	sim_ok.setInputValuesOpen();
	sim_ok.simulateOneTimeStep();
	solver->incAddUnitClause(-sim_ok.getAlarmValue());

	for (unsigned s_cnt = 0; s_cnt < num_initial_steps; s_cnt++)
	{
		sim_ok.switchToNextState();
		sim_ok.setInputValuesOpen();
		sim_ok.simulateOneTimeStep();
		solver->incAddUnitClause(-sim_ok.getAlarmValue());
	}

	vector<int> alarms_in_flipped_version_at_step_i; // maps i -> alarm signal of faulty verstion at step i
	vector<int> output_different_at_step_i; // maps i -> different_at_i
	int final_nxt_state_diff_enable = next_free_cnf_var++;

	vector<unsigned> latches_to_check = Options::instance().removeExcludedLatches(circuit_,
			num_err_latches_);
	map<int, int> cj_to_latch; // maps cj-literals(cnf) to corresponding latch-literals(aig)
	vector<int> c_vars;

	SymbolicSimulator sim_faulty(circuit_, solver, next_free_cnf_var);
	for (unsigned i = 0; i < k_steps; i++)
	{

		// set same open input values for both copies
		sim_ok.setInputValuesOpen();
		sim_faulty.setCnfInputValues(sim_ok.getInputValues());

		sim_ok.simulateOneTimeStep();	// T_ok(x,i,o,a,x')
		if (i == 0) // flip in first step
		{
			sim_faulty.setResults(sim_ok.getResults()); // copies everything (including inputs)
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
				if (old_value == CNF_TRUE) // old value is true
					sim_faulty.setResultValue(latch_cnf, -cj);
				else if (old_value == CNF_FALSE) // old value is false
					sim_faulty.setResultValue(latch_cnf, cj);
				else
				{
					int new_value = next_free_cnf_var++;
					sim_faulty.setResultValue(latch_cnf, new_value);
					solver->addVarToKeep(new_value);
					// new_value == cj ? -old_value : old_value
					solver->incAdd3LitClause(cj, old_value, -new_value);
					solver->incAdd3LitClause(cj, -old_value, new_value);
					solver->incAdd3LitClause(-cj, old_value, new_value);
					solver->incAdd3LitClause(-cj, -old_value, -new_value);
				}

			}

			//--------------------------------------------------------------------------------------
			// single fault assumption: there can be at most one flipped component
			// if c is true, all other c must be false (ci -> -c1, ci -> -c2, ...)
			for (unsigned i = 0; i < c_vars.size(); i++)
			{
				for (unsigned j = i + 1; j < c_vars.size(); j++)
					solver->incAdd2LitClause(-c_vars[i], -c_vars[j]);
			}
		}
		sim_faulty.simulateOneTimeStep();		// T_err(x_e,i,o_e,a_e,x'e)

		// store a_e
		alarms_in_flipped_version_at_step_i.push_back(sim_faulty.getAlarmValue());

		// require no alarm in fault-free version
		solver->incAddUnitClause(-sim_ok.getAlarmValue()); // -a

		// create clause saying that output is different
		vector<int> outputs_ok = sim_ok.getOutputValues();		// o
		vector<int> outputs_flip = sim_faulty.getOutputValues(); // o_e

		int output_different_now_enable = next_free_cnf_var++;
		output_different_at_step_i.push_back(output_different_now_enable);
		vector<int> output_different_now_clause;
		output_different_now_clause.push_back(-output_different_now_enable);

		// exclude alarm output
		outputs_ok.pop_back();
		outputs_flip.pop_back();
		CnfUtils::generateVectorIsDifferentClause(outputs_ok, outputs_flip, output_different_now_clause, next_free_cnf_var, solver);
		// looks like: (disabled OR o1_diff OR o2_diff OR ... OR on_diff):

		solver->incAddClause(output_different_now_clause);

		if (i == k_steps - 1) // last step
		{
			// clause saying next state is still different (x' != x_e'):
			vector<int> next_ok = sim_ok.getNextLatchValues();			// x'
			vector<int> next_faulty = sim_faulty.getNextLatchValues();	// x_e'
			// (x'1_diff OR x'2_diff OR ... OR x'n_diff):

			vector<int> next_state_different;
			next_state_different.push_back(-final_nxt_state_diff_enable);

			// exclude the last 'num_err_latches_' latches (if any)
			next_ok.erase(next_ok.end() - num_err_latches_, next_ok.end());
			next_faulty.erase(next_faulty.end() - num_err_latches_, next_faulty.end());
			CnfUtils::generateVectorIsDifferentClause(next_ok, next_faulty,
					next_state_different, next_free_cnf_var, solver);

			solver->incAddClause(next_state_different);



		}
		else // has further steps
		{
			sim_ok.switchToNextState();
			sim_faulty.switchToNextState();
		}

	}

	// maps a literal that is true if there was no alarm set to true until time step k
	vector<int> no_alarm_until;
	no_alarm_until.push_back(-alarms_in_flipped_version_at_step_i[0]); // i=0: not a_0
	for (unsigned i = 1; i < alarms_in_flipped_version_at_step_i.size(); i++)
	{
		if (no_alarm_until[i - 1] == CNF_FALSE)
		{
			no_alarm_until.push_back(CNF_FALSE);
			continue;
		}
		// no_alarm_until[i] <-> no_alarm_until_[i-1] AND not a_i:
		int no_alarm_until_k = next_free_cnf_var++;
		no_alarm_until.push_back(no_alarm_until_k);

		solver->incAdd2LitClause(-no_alarm_until_k, no_alarm_until[i - 1]);
		solver->incAdd2LitClause(-no_alarm_until_k, -alarms_in_flipped_version_at_step_i[i]);
		solver->incAdd3LitClause(no_alarm_until_k, -no_alarm_until[i - 1],
				alarms_in_flipped_version_at_step_i[i]);
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
			solver->incAdd3LitClause(-out_or_next_different, output_different_at_step_i[i],
					final_nxt_state_diff_enable);
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
		for (unsigned i = 0; i < model.size(); i++)
		{
			int c = model[i];
			if (c > 0) // found the c with positive face
			{
				solver->incAddUnitClause(-c);
				detected_latches_.erase(cj_to_latch[c]);
				L_DBG("latch " << cj_to_latch[c] << " not n-step protected")
				break;
			}
		}
	}

	delete solver;
}

void DefinitelyProtected::printResults()
{
	float percentage = (float) detected_latches_.size() / Options::instance().getNumberOfLatchesToCheck() * 100;
	L_LOG(
			"#Definitely protected latches found: " << detected_latches_.size() << " (" << percentage << " %)");

	if (Options::instance().isUseDiagnosticOutput())
	{
		ostringstream oss;
		if (detected_latches_.size() > 0)
			oss << "Definitely protected latches:" << endl;

		unsigned i = 0;
		for (set<unsigned>::iterator it = detected_latches_.begin(); it != detected_latches_.end();
				++it)
		{
			oss << *it << "\t";

			if ((i + 1) % 5 == 0 && i < detected_latches_.size() - 1)
				oss << endl;

			i++;
		}
		if (Options::instance().isDiagnosticOutputToFile())
		{
			ofstream out_file;
			out_file.open(Options::instance().getDiagnosticOutputPath().c_str());
			MASSERT(out_file,
					"could not write diagnostic output file: "
							+ Options::instance().getDiagnosticOutputPath())

			out_file << oss.str() << endl;
			out_file.close();
		}
		else
		{
			cout << endl << oss.str() << endl;
		}
	}
}
