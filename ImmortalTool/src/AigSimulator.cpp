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
/// @file AigSimulator.cpp
/// @brief Contains the definition of the class AigSimulator.
// -------------------------------------------------------------------------------------------

#include <fstream>

#include "AigSimulator.h"
#include "Logger.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
AigSimulator::AigSimulator(aiger* circuit) :
		time_index_(0)
{
	circuit_ = circuit;
	results_ = new int[circuit_->maxvar + 1];
	results_[0] = 0;
	results_[1] = 1;

	init();
}

// -------------------------------------------------------------------------------------------
AigSimulator::~AigSimulator()
{
	delete results_;
}

// -------------------------------------------------------------------------------------------
void AigSimulator::setTestcase(string path_to_aigsim_input)
{
	init();

	ifstream infile(path_to_aigsim_input.c_str());
	string input_vector_line;
	while (infile >> input_vector_line)
	{
		MASSERT(input_vector_line.length() == circuit_->num_inputs,
				"corrupt aigsim-file (does not match number of inputs)!");

		vector<int> input_vector;
		for (int i = 0; i < input_vector_line.length(); i++)
		{
			if (input_vector_line.c_str()[i] == '0')
			{
				input_vector.push_back(0);
			}
			else if (input_vector_line.c_str()[i] == '1')
			{
				input_vector.push_back(1);
			}
			else
			{
				MASSERT(false, "corrupt aigsim-file (unexpected character)!");
			}
		}
		testcase_.push_back(input_vector);
	}
}

// -------------------------------------------------------------------------------------------
void AigSimulator::setTestcase(const vector<vector<int> >& testcase)
{
	MASSERT(testcase.size() > 0 && testcase[0].size() == circuit_->num_inputs,
			"Wrong test case provided!");
	init();
	testcase_ = testcase;
}

// -------------------------------------------------------------------------------------------
bool AigSimulator::simulateOneTimeStep()
{
	if (testcase_.empty())
	{
		L_ERR(
				"No TCs provided, use AigSimulator::simulateOneTimeStep(const vector<int> &input_values) instead");
		return false; // TODO: MASSERT?
	}

	if (time_index_ >= testcase_.size())
	{
		return false;
	}

	simulateOneTimeStep(testcase_[time_index_]);
	return true;
}

// -------------------------------------------------------------------------------------------
void AigSimulator::simulateOneTimeStep(const vector<int> &input_values)
{
	MASSERT(input_values.size() == circuit_->num_inputs,
			"Wrong test case provided!");

	// copy inputs into results_
	for (size_t cnt = 0; cnt < circuit_->num_inputs; ++cnt)
	{
		results_[aiger_lit2var(circuit_->inputs[cnt].lit)] = input_values[cnt];
	}

	// compute AND outputs
	for (size_t cnt = 0; cnt < circuit_->num_ands; ++cnt)
	{
		int rhs0_val = results_[aiger_lit2var(circuit_->ands[cnt].rhs0)];
		if (circuit_->ands[cnt].rhs0 % 2 != 0)
		{
			rhs0_val = aiger_not(rhs0_val);
		}
		int rhs1_val = results_[aiger_lit2var(circuit_->ands[cnt].rhs1)];
		if (circuit_->ands[cnt].rhs1 % 2 != 0)
		{
			rhs1_val = aiger_not(rhs1_val);
		}

		results_[aiger_lit2var(circuit_->ands[cnt].lhs)] = rhs0_val & rhs1_val;
	}

	time_index_++;
}

// -------------------------------------------------------------------------------------------
void AigSimulator::simulateOneTimeStep(const vector<int>& input_values,
		const vector<int>& latch_values)
{
	MASSERT(input_values.size() == circuit_->num_inputs,
			"Wrong test case (number of inputs) provided!");

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
string AigSimulator::getStateString()
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
	const vector<int>& out_values = getOutputs();
	for (size_t cnt = 0; cnt < circuit_->num_outputs; ++cnt)
	{
		str << out_values[cnt];
	}

	if (circuit_->num_outputs > 0 && circuit_->num_latches > 0)
	{
		str << " ";
	}

	// Latch next state values (.next)
	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		str << results_[aiger_lit2var(circuit_->latches[cnt].next)];
	}

	return str.str();
}

// -------------------------------------------------------------------------------------------
string AigSimulator::getVerboseStateString()
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
		str << "latch: " << circuit_->latches[cnt].lit << "(next="
				<< circuit_->latches[cnt].next << ")" << "=´"
				<< results_[aiger_lit2var(circuit_->latches[cnt].lit)] << "´" << endl;
	}

	for (size_t cnt = 0; cnt < circuit_->num_ands; ++cnt)
	{
		str << "AND: " << circuit_->ands[cnt].lhs << "=" << circuit_->ands[cnt].rhs0
				<< "AND" << circuit_->ands[cnt].rhs1 << "=´"
				<< results_[aiger_lit2var(circuit_->ands[cnt].lhs)] << "´" << endl;
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
			str << aiger_not(results_[aiger_lit2var(circuit_->outputs[cnt].lit)])
					<< "´" << endl;
		}

	}

	return str.str();
}

// -------------------------------------------------------------------------------------------
void AigSimulator::switchToNextState()
{
	vector<int> latch_results(circuit_->num_latches);

	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		latch_results[cnt] = results_[aiger_lit2var(circuit_->latches[cnt].next)];

		if (circuit_->latches[cnt].next % 2 == 1)
		{
			latch_results[cnt] = aiger_not(latch_results[cnt]);
		}
	}

	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		results_[aiger_lit2var(circuit_->latches[cnt].lit)] = latch_results[cnt];
	}
}

// -------------------------------------------------------------------------------------------
vector<int> AigSimulator::getOutputs()
{
	vector<int> outputs(circuit_->num_outputs);

	for (size_t cnt = 0; cnt < circuit_->num_outputs; ++cnt)
	{

		if (circuit_->outputs[cnt].lit % 2 == 1)
		{
			outputs[cnt] = aiger_not(
					results_[aiger_lit2var(circuit_->outputs[cnt].lit)]);
		}
		else
		{
			outputs[cnt] = results_[aiger_lit2var(circuit_->outputs[cnt].lit)];
		}
	}
	return outputs;
}

// -------------------------------------------------------------------------------------------
vector<int> AigSimulator::getLatchValues()
{
	vector<int> latches(circuit_->num_latches);

	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{

		// TODO: check if a latch output can really be inverted
//		if (circuit_->latches[cnt].lit % 2 == 1)
//		{
//			latches[cnt] = aiger_not(
//					results_[aiger_lit2var(circuit_->latches[cnt].lit)]);
//		}
//		else
//		{
			latches[cnt] = results_[aiger_lit2var(circuit_->latches[cnt].lit)];
//		}
	}
	return latches;
}


void AigSimulator::init()
{
	// initialize latches (if any) with FALSE
	for (size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		results_[aiger_lit2var(circuit_->latches[cnt].lit)] =
				circuit_->latches[cnt].reset;
		results_[aiger_lit2var(circuit_->latches[cnt].next)] = 0;
	}
}
