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
		sim_symb.setInputValuesOpen();
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
		sim_symb.setInputValuesOpen();
		sim_symb.simulateOneTimeStep();
	}


	//---------------------------------------
	vector<int> next_state_normal = sim_symb.getNextLatchValues();
	vector<int> outputs_normal = sim_symb.getOutputValues();

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
		// new_value == cj ? -old_value : old_value
		int old_value = sim_symb.getResultValue(latch_cnf);
		if (old_value == CNF_TRUE) // old value is true
			sim_symb.setResultValue(latch_cnf, -cj);
		else if (old_value == CNF_FALSE) // old value is false
			sim_symb.setResultValue(latch_cnf, cj);
		else
		{
			int new_value = next_free_cnf_var++;
			solver->incAdd3LitClause(cj, old_value, -new_value);
			solver->incAdd3LitClause(cj, -old_value, new_value);
			solver->incAdd3LitClause(-cj, old_value, new_value);
			solver->incAdd3LitClause(-cj, -old_value, -new_value);
			sim_symb.setResultValue(latch_cnf, new_value);
		}

	}

	// one c signal must me true. TODO: this should not be necessary
	solver->incAddClause(c_vars);
	//--------------------------------------------------------------------------------------
	// single fault assumption: there might be at most one flipped component
	// if c is true, all other c must be false (ci -> -c1, ci -> -c2, ...)
	for (unsigned i = 0; i < c_vars.size(); i++)
	{
		for (int j = i + 1; j < c_vars.size(); j++)
			solver->incAdd2LitClause(-c_vars[i], -c_vars[j]);
	}

	// T_err(x,i,c,o_e,x_e'): faulty transition relation where latches can be flipped by SAT-solver
	sim_symb.simulateOneTimeStep();

	vector<int> next_state_flip = sim_symb.getNextLatchValues();
	vector<int> outputs_flip = sim_symb.getOutputValues();

	// we only care about situations where the alarm is false
	solver->incAddUnitClause(-sim_symb.getAlarmValue()); // no alarm


	// create clauses saying that
	//		(next_state_normal != next_state_flip) OR (outputs_normal != outputs_flip)
	vector<int> output_or_next_state_different_clause;

	for(unsigned i = 0; i < circuit_->num_outputs -1; i++)
	{
		if (outputs_normal[i] == outputs_flip[i])
			continue;

		solver->addVarToKeep(next_free_cnf_var);
		int out_is_diff_enable = next_free_cnf_var++;
		output_or_next_state_different_clause.push_back(-out_is_diff_enable);
		solver->incAdd3LitClause(out_is_diff_enable, outputs_normal[i], outputs_flip[i]);
		solver->incAdd3LitClause(out_is_diff_enable, -outputs_normal[i], -outputs_flip[i]);
	}

	for(unsigned i = 0; i < circuit_->num_latches - num_err_latches_; i++)
	{
		if (next_state_normal[i] == next_state_flip[i])
			continue;

		solver->addVarToKeep(next_free_cnf_var);
		int state_is_diff_enable = next_free_cnf_var++;
		output_or_next_state_different_clause.push_back(-state_is_diff_enable);
		solver->incAdd3LitClause(state_is_diff_enable, next_state_normal[i], next_state_flip[i]);
		solver->incAdd3LitClause(state_is_diff_enable, -next_state_normal[i], -next_state_flip[i]);
	}

	solver->incAddClause(output_or_next_state_different_clause);

	// TODO remove
	c_vars.insert(c_vars.end(), outputs_normal.begin(), outputs_normal.end());
	c_vars.insert(c_vars.end(), outputs_flip.begin(), outputs_flip.end());

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
				break;
			}
			return;
		}
//		detected_latches_.remove(latch_aig);
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

	if(solver->incIsSat() == false)
	{
		detected_latches_.insert(latch_aig);
		L_DBG("Definitely protected latch "<< latch_aig << " found. (UNSAT)");
	}
	else
	{
		L_DBG("SAT " << latch_aig << "(not n-step protected)")
	}
}

void DefinitelyProtected::printResults()
{
	float percentage = (float) detected_latches_.size() / (circuit_->num_latches - num_err_latches_) * 100;
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
