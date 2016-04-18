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
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file BddSimulator2.cpp
/// @brief Contains the definition of the class BddSimulator2.
// -------------------------------------------------------------------------------------------

#include <fstream>
#include <algorithm>

#include "BddSimulator2.h"
#include "Logger.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
BddSimulator2::BddSimulator2(aiger* circuit, const Cudd &cudd,
		int& next_free_cnf_var_reference) :
		next_free_cnf_var_(next_free_cnf_var_reference), cudd_(cudd)
{
	circuit_ = circuit;
	results_.resize(circuit_->maxvar + 1);
	results_[0] = cudd.bddZero(); // FALSE and TRUE constants


	ref_counters_.resize(circuit->maxvar + 1);

	for (unsigned b = 0; b < circuit_->num_ands; ++b)
	{
		ref_counters_[circuit_->ands[b].rhs0 >> 1]++;
		ref_counters_[circuit_->ands[b].rhs1 >> 1]++;
	}

	for (unsigned b = 0; b < circuit_->num_latches; ++b)
	{
		ref_counters_[circuit_->latches[b].lit >> 1]++;
		ref_counters_[circuit_->latches[b].next >> 1]++;
	}

	for (unsigned b = 0; b < circuit_->num_outputs; ++b)
	{
		ref_counters_[circuit_->outputs[b].lit >> 1]++;
	}
	ref_counters_[circuit_->outputs[circuit_->num_outputs-1].lit >> 1]++;

	initLatches();
}

// -------------------------------------------------------------------------------------------
BddSimulator2::~BddSimulator2()
{
}


// -------------------------------------------------------------------------------------------
void BddSimulator2::simulateOneTimeStep(const vector<int> &input_values)
{
	MASSERT(input_values.size() == circuit_->num_inputs, "Wrong test case provided!");

	setInputValues(input_values); 	// copy inputs into results_
	simulateOneTimeStep();					// simulate AND gates

}

// -------------------------------------------------------------------------------------------
void BddSimulator2::simulateOneTimeStep()
{

	references_left_ = ref_counters_;

	// Symbolic simulation of AND gates
	for (unsigned b = 0; b < circuit_->num_ands; ++b)
	{
		BDD rhs0 = readCnfValue(results_, circuit_->ands[b].rhs0);
		BDD rhs1 = readCnfValue(results_, circuit_->ands[b].rhs1);

		results_[(circuit_->ands[b].lhs >> 1)] = rhs0 & rhs1;

		decrementReference(circuit_->ands[b].rhs0);
		decrementReference(circuit_->ands[b].rhs1);
	}

}

// -------------------------------------------------------------------------------------------
void BddSimulator2::switchToNextState()
{
	vector<BDD>latch_values;
	latch_values.reserve(circuit_->num_latches);
	for (unsigned b = 0; b < circuit_->num_latches; ++b)
	{
		BDD next_state = readCnfValue(results_, circuit_->latches[b].next);
		decrementReference(circuit_->latches[b].next);
		latch_values.push_back(next_state);
	}

	for (unsigned b = 0; b < circuit_->num_latches; ++b)
	{
		results_[(circuit_->latches[b].lit >> 1)] = latch_values[b];
	}

}


void BddSimulator2::switchToNextState(const BDD &restriction)
{
	vector<BDD>latch_values;
	latch_values.reserve(circuit_->num_latches);
	for (unsigned b = 0; b < circuit_->num_latches; ++b)
	{
		BDD next_state = readCnfValue(results_, circuit_->latches[b].next);
		decrementReference(circuit_->latches[b].next);
		latch_values.push_back(next_state);
	}

	for (unsigned b = 0; b < circuit_->num_latches; ++b)
	{
		results_[(circuit_->latches[b].lit >> 1)] = latch_values[b].Restrict(restriction);
	}
}

/*
// -------------------------------------------------------------------------------------------
const vector<int> &BddSimulator2::getOutputValues()
{

	if (true)
	{
		output_values_.clear();
		output_values_.reserve(circuit_->num_outputs);
		for (unsigned b = 0; b < circuit_->num_outputs; ++b)
		{
			output_values_.push_back(Utils::readCnfValue(results_, circuit_->outputs[b].lit));
		}
		output_values_is_latest_ = true;
	}

	return output_values_;

}

// -------------------------------------------------------------------------------------------
const vector<int>& BddSimulator2::getInputValues()
{
	if (true) // TODO
	{
		input_values_.clear();
		input_values_.reserve(circuit_->num_inputs);
		for (unsigned b = 0; b < circuit_->num_inputs; ++b)
		{
			input_values_.push_back(results_[(circuit_->inputs[b].lit >> 1)]);
		}
		//input_values_is_latest_ = true;
	}

	return input_values_;
}

// -------------------------------------------------------------------------------------------
const vector<int> &BddSimulator2::getLatchValues()
{
	if (!latch_values_is_latest_)
	{
		latch_values_.clear();
		latch_values_.reserve(circuit_->num_latches);
		for (unsigned b = 0; b < circuit_->num_latches; ++b)
		{
			int next_state_var = Utils::readCnfValue(results_, circuit_->latches[b].next);
			latch_values_.push_back(next_state_var);
			if (abs(next_state_var) > 1)
				solver_->addVarToKeep(next_state_var);
		}
		latch_values_is_latest_ = true;
	}

	return latch_values_;
}

// -------------------------------------------------------------------------------------------
const vector<int> &BddSimulator2::getNextLatchValues()
{

	if (!next_values_is_latest_)
	{
		next_values_.clear();
		next_values_.reserve(circuit_->num_latches);

		for (unsigned b = 0; b < circuit_->num_latches; ++b)
		{
			int next_state_var = Utils::readCnfValue(results_, circuit_->latches[b].next);
			next_values_.push_back(next_state_var);
		}
		next_values_is_latest_ = true;
	}

	return next_values_;
}

*/

// -------------------------------------------------------------------------------------------
void BddSimulator2::setInputValues(const vector<int>& input_values)
{
	MASSERT(input_values.size() == circuit_->num_inputs, "Input vector has wrong length!");

	// set input values according to TestCase to TRUE or FALSE:
	for (unsigned cnt_i = 0; cnt_i < circuit_->num_inputs; ++cnt_i)
	{
		if (input_values[cnt_i] == AIG_TRUE)
			results_[(circuit_->inputs[cnt_i].lit >> 1)] = cudd_.bddOne();
		else if (input_values[cnt_i] == AIG_FALSE)
			results_[(circuit_->inputs[cnt_i].lit >> 1)] = cudd_.bddZero();
		else // (if LIT_FREE) handle '?' input:
		{
			results_[(circuit_->inputs[cnt_i].lit >> 1)] = cudd_.bddVar(next_free_cnf_var_);
			next_free_cnf_var_++;
			//TODO: add to vector open_input_vars
		}
	}

}

// -------------------------------------------------------------------------------------------
void BddSimulator2::setResultValue(unsigned cnf_lit, BDD value)
{
	results_[cnf_lit] = value;
}

// -------------------------------------------------------------------------------------------
BDD BddSimulator2::getResultValue(unsigned cnf_lit)
{
	return results_[cnf_lit];
}

void BddSimulator2::getOutputValues(vector<BDD>& outputs)
{
	outputs.reserve(circuit_->num_outputs);
	for (unsigned b = 0; b < circuit_->num_outputs; ++b)
	{
		outputs.push_back(readCnfValue(results_, circuit_->outputs[b].lit));
		decrementReference(circuit_->outputs[b].lit);
	}
}

// -------------------------------------------------------------------------------------------
void BddSimulator2::initLatches()
{
	for (unsigned l = 0; l < circuit_->num_latches; ++l)
		results_[(circuit_->latches[l].lit >> 1)] = cudd_.bddZero();
}

// -------------------------------------------------------------------------------------------
BDD BddSimulator2::getAlarmValue()
{
	decrementReference(circuit_->outputs[circuit_->num_outputs - 1].lit);
	return readCnfValue(results_, circuit_->outputs[circuit_->num_outputs - 1].lit);
}

