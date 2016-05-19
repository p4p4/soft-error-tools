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
	else
		MASSERT(false, "unknown mode!");

}

void DefinitelyProtected::findDefinitelyProtected_1()
{
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
//		AndCacheMap cache(solver_);
		unsigned num_steps = 1;
		for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
		{
//			if (s_cnt == num_steps - 1) // last iteration
//				sim_symb.setCache(&cache);
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
			L_DBG("Definitely protected latch "<< circuit_->latches[c_cnt].lit << " found. (UNSAT)")
		detected_latches_.insert(circuit_->latches[c_cnt].lit);
		}
		else
		{
			L_DBG("SAT..")
			// TODO: are models interesting as counter examples?
		}

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


	// variables for the state
	for(unsigned i = 0; i < circuit_->num_latches; i++)
	{
		//solver->addVarToKeep(next_free_cnf_var);
		sim_symb.setResultValue(circuit_->latches[i].lit/2, next_free_cnf_var++);
	}


	// variables for the inputs
	for(unsigned i = 0; i < circuit_->num_inputs; i++)
	{
		//solver->addVarToKeep(next_free_cnf_var);
		sim_symb.setResultValue(circuit_->inputs[i].lit/2, next_free_cnf_var++);
	}

	// compute transition relation T(x,i,o,a,x')
	sim_symb.simulateOneTimeStep();

	// additional time steps
//	AndCacheMap cache(solver);
	unsigned num_steps = 1;
	for (unsigned s_cnt = 0; s_cnt < num_steps; s_cnt++)
	{
		//if (s_cnt == num_steps - 1) // last iteration
		//	sim_symb.setCache(&cache);
		sim_symb.switchToNextState();
		for(unsigned i = 0; i < circuit_->num_inputs; i++)
		{
			solver->addVarToKeep(next_free_cnf_var);
			sim_symb.setResultValue(circuit_->inputs[i].lit/2, next_free_cnf_var++);
		}

		sim_symb.simulateOneTimeStep();
	}

	vector<int> next_state_normal = sim_symb.getNextLatchValues();
	vector<int> outpus_normal = sim_symb.getOutputValues();

	solver->addVarsToKeep(next_state_normal);
	solver->addVarsToKeep(outpus_normal);

	int next_free_cnf_var_after_first_step = next_free_cnf_var;
	solver->incPush();
	vector<int> results_bakcup = sim_symb.getResults();

	// ---------------- BEGIN 'for each latch' -------------------------
	// let's assume we know which latch is not part of the protection logic
	for (unsigned c_cnt = 0; c_cnt < circuit_->num_latches - num_err_latches_; ++c_cnt)
	{
		unsigned component_cnf = circuit_->latches[c_cnt].lit >> 1;

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
			L_DBG("Definitely protected latch "<< circuit_->latches[c_cnt].lit << " found. (UNSAT)")
		detected_latches_.insert(circuit_->latches[c_cnt].lit);
		}
		else
		{
			L_DBG("SAT..")
			// TODO: are models interesting as counter examples?
		}


		solver->incPop();
		solver->incPush();
		sim_symb.setResults(results_bakcup);
		next_free_cnf_var = next_free_cnf_var_after_first_step;
	}

	delete solver;

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
