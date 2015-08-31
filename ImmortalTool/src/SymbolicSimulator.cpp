// ----------------------------------------------------------------------------
// Copyright (c) 2013-2014 by Graz University of Technology
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
// For more information about this software see
//   <http://www.iaik.tugraz.at/content/research/design_verification/others/>
// or email the authors directly.
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file SymbolicSimulator.cpp
/// @brief Contains the definition of the class SymbolicSimulator.
// -------------------------------------------------------------------------------------------

#include <fstream>
#include <algorithm>

#include "SymbolicSimulator.h"
#include "Logger.h"
#include "Utils.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
SymbolicSimulator::SymbolicSimulator(aiger* circuit, SatSolver* solver,
		int& next_free_cnf_var_reference) :
		time_index_(0), solver_(solver), next_free_cnf_var_(next_free_cnf_var_reference)
{
	circuit_ = circuit;
	results_.resize(circuit_->maxvar + 1);
	results_[0] = CNF_FALSE; // FALSE and TRUE constants

	initLatches();
}

// -------------------------------------------------------------------------------------------
SymbolicSimulator::~SymbolicSimulator()
{
}

// -------------------------------------------------------------------------------------------
void SymbolicSimulator::simulateOneTimeStep(const vector<int>& input_values,
		const vector<int>& latch_values)
{
	MASSERT(latch_values.size() == circuit_->num_latches,
			"Wrong test case (number of latches) provided!");

	// set latch values:
	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		results_[aiger_lit2var(circuit_->latches[cnt].lit)] = latch_values[cnt];
	}

	simulateOneTimeStep(input_values);
}

// -------------------------------------------------------------------------------------------
void SymbolicSimulator::simulateOneTimeStep(const vector<int> &input_values)
{
	MASSERT(input_values.size() == circuit_->num_inputs, "Wrong test case provided!");

	setInputValues(input_values); 	// copy inputs into results_
	simulateOneTimeStep();					// simulate AND gates

}

// -------------------------------------------------------------------------------------------
void SymbolicSimulator::simulateOneTimeStep()
{
	// Symbolic simulation of AND gates
	for (unsigned b = 0; b < circuit_->num_ands; ++b)
	{

		int rhs1_cnf_value = Utils::readCnfValue(results_, circuit_->ands[b].rhs1);
		int rhs0_cnf_value = Utils::readCnfValue(results_, circuit_->ands[b].rhs0);

		if (rhs1_cnf_value == CNF_FALSE || rhs0_cnf_value == CNF_FALSE) // FALSE and .. = FALSE
			results_[(circuit_->ands[b].lhs >> 1)] = CNF_FALSE;
		else if (rhs1_cnf_value == CNF_TRUE) // TRUE and X = X
			results_[(circuit_->ands[b].lhs >> 1)] = rhs0_cnf_value;
		else if (rhs0_cnf_value == CNF_TRUE) // X and TRUE = X
			results_[(circuit_->ands[b].lhs >> 1)] = rhs1_cnf_value;
		else if (rhs0_cnf_value == rhs1_cnf_value) // X and X = X
			results_[(circuit_->ands[b].lhs >> 1)] = rhs1_cnf_value;
		else if (rhs0_cnf_value == -rhs1_cnf_value) // X and -X = FALSE
			results_[(circuit_->ands[b].lhs >> 1)] = CNF_FALSE;
		else
		{
			int res = next_free_cnf_var_++;
			// res == rhs1_cnf_value & rhs0_cnf_value:
			// Step 1: (rhs1_cnf_value == false) -> (res == false)
			solver_->incAdd2LitClause(rhs1_cnf_value, -res);
			// Step 2: (rhs0_cnf_value == false) -> (res == false)
			solver_->incAdd2LitClause(rhs0_cnf_value, -res);
			// Step 3: (rhs0_cnf_value == true && rhs1_cnf_value == true)
			//   -> (res == true)
			solver_->incAdd3LitClause(-rhs0_cnf_value, -rhs1_cnf_value, res);
			results_[(circuit_->ands[b].lhs >> 1)] = res;

		}
	}

	time_index_++;
}

// -------------------------------------------------------------------------------------------
string SymbolicSimulator::getStateString()	// TODO: change from AIGER to CNF
{
	ostringstream str;
	// Latch current state values (.lit)
	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		str << results_[aiger_lit2var(circuit_->latches[cnt].lit)];
	}

	if (circuit_->num_latches > 0)
	{
		str << " ";
	}

	// Inputs:
	for (size_t cnt = 0; cnt < circuit_->num_inputs; ++cnt)
	{
		str << results_[aiger_lit2var(circuit_->inputs[cnt].lit)];
	}

	if (circuit_->num_inputs > 0)
	{
		str << " ";
	}

	//Outputs
	const vector<int>& out_values = getOutputValues();
	for (size_t cnt = 0; cnt < circuit_->num_outputs; ++cnt)
	{
		str << out_values[cnt];
	}

	if (circuit_->num_outputs > 0 && circuit_->num_latches > 0)
	{
		str << " ";
	}

	// Latch next state values (.next)
	const vector<int>& next_values = getNextLatchValues();
	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		str << next_values[cnt];
	}

	return str.str();
}

// -------------------------------------------------------------------------------------------
string SymbolicSimulator::getVerboseStateString() // TODO: change from AIGER to CNF/check
{
	ostringstream str;
	str << "Time-step: " << time_index_ << endl;

	for (size_t cnt = 0; cnt < circuit_->num_inputs; ++cnt)
	{
		str << "input: " << circuit_->inputs[cnt].lit << "=´"
				<< results_[aiger_lit2var(circuit_->inputs[cnt].lit)] << "´" << endl;
	}

	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		str << "latch: " << circuit_->latches[cnt].lit << "(next=" << circuit_->latches[cnt].next
				<< ")" << "=´" << results_[aiger_lit2var(circuit_->latches[cnt].lit)] << "´" << endl;
	}

	for (size_t cnt = 0; cnt < circuit_->num_ands; ++cnt)
	{
		str << "AND: " << circuit_->ands[cnt].lhs << "=" << circuit_->ands[cnt].rhs0 << "AND"
				<< circuit_->ands[cnt].rhs1 << "=´" << results_[aiger_lit2var(circuit_->ands[cnt].lhs)]
				<< "´" << endl;
	}
	for (size_t cnt = 0; cnt < circuit_->num_outputs; ++cnt)
	{
		str << "output: " << circuit_->outputs[cnt].lit << "=´";

		if (circuit_->ands[cnt].rhs1 % 2 != 0)
		{
			str << results_[aiger_lit2var(circuit_->outputs[cnt].lit)] << "´" << endl;
		}
		else
		{
			str << aiger_not(results_[aiger_lit2var(circuit_->outputs[cnt].lit)]) << "´" << endl;
		}

	}

	return str.str();
}

// -------------------------------------------------------------------------------------------
void SymbolicSimulator::switchToNextState()
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

	for (unsigned b = 0; b < circuit_->num_latches; ++b)
	{
		results_[(circuit_->latches[b].lit >> 1)] = latch_values_[b];
	}
}

// -------------------------------------------------------------------------------------------
const vector<int> &SymbolicSimulator::getOutputValues()
{
	output_values_.clear();
	output_values_.reserve(circuit_->num_outputs - 1);
	for (unsigned b = 0; b < circuit_->num_outputs - 1; ++b)
	{
		output_values_.push_back(Utils::readCnfValue(results_, circuit_->outputs[b].lit));
	}

	return output_values_;
}

// -------------------------------------------------------------------------------------------
const vector<int> &SymbolicSimulator::getLatchValues()
{
	return latch_values_;
}

// -------------------------------------------------------------------------------------------
const vector<int> &SymbolicSimulator::getNextLatchValues()
{
	next_values_.clear();
	next_values_.reserve(circuit_->num_latches);

	for (unsigned b = 0; b < circuit_->num_latches; ++b)
	{
		int next_state_var = Utils::readCnfValue(results_, circuit_->latches[b].next);
		next_values_.push_back(next_state_var);
	}

	return next_values_;
}

// -------------------------------------------------------------------------------------------
void SymbolicSimulator::setInputValues(const vector<int>& input_values)
{
	MASSERT(input_values.size() == circuit_->num_inputs, "Input vector has wrong length!");

	// set input values according to TestCase to TRUE or FALSE:
	for (unsigned cnt_i = 0; cnt_i < circuit_->num_inputs; ++cnt_i)
		results_[(circuit_->inputs[cnt_i].lit >> 1)] =
				(input_values[cnt_i] == AIG_TRUE) ? CNF_TRUE : CNF_FALSE;
}

// -------------------------------------------------------------------------------------------
void SymbolicSimulator::setResultValue(unsigned cnf_lit, int cnf_value)
{
	results_[cnf_lit] = cnf_value;
}

// -------------------------------------------------------------------------------------------
int SymbolicSimulator::getResultValue(unsigned cnf_lit)
{
	return results_[cnf_lit];
}

// -------------------------------------------------------------------------------------------
void SymbolicSimulator::initLatches()
{
	for (unsigned l = 0; l < circuit_->num_latches; ++l)
		results_[(circuit_->latches[l].lit >> 1)] = CNF_FALSE;

	time_index_ = 0;
}

int SymbolicSimulator::getAlarmValue()
{
	return Utils::readCnfValue(results_,
						circuit_->outputs[circuit_->num_outputs - 1].lit);
}
