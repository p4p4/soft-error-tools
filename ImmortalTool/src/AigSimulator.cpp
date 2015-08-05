// ----------------------------------------------------------------------------
// Copyright (c) 2013-2014 by Graz University of Technology and
//                            Johannes Kepler University Linz
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


#include "AigSimulator.h"

extern "C" {
  #include "aiger.h"
}

// -------------------------------------------------------------------------------------------
AigSimulator::AigSimulator(aiger* circuit) : time_index_(0)
{
  circuit_ = circuit;
  results_ = new int[circuit_->maxvar + 1];
  results_[0] = 0;
  results_[1] = 1;

  // initialize latches (if any) with FALSE
	for(size_t cnt = 0; cnt < circuit_->num_latches; ++cnt)
	{
		results_[aiger_lit2var(circuit_->latches[cnt].lit)] = 0;
	}
}

// -------------------------------------------------------------------------------------------
AigSimulator::~AigSimulator()
{
	delete results_;
}


void AigSimulator::simulateOneTimeStep(const vector<int> &input_values)
{
	MASSERT(input_values.size() == circuit_->num_inputs, "Wrong test case provided!");

	// copy inputs into results_
	for(size_t cnt = 0; cnt < circuit_->num_inputs; ++cnt)
	{
		results_[aiger_lit2var(circuit_->inputs[cnt].lit)] = input_values[cnt];
	}

	// compute AND outputs
  // initialize latches (if any) with FALSE
	for(size_t cnt = 0; cnt < circuit_->num_ands; ++cnt)
	{
		int rhs0_val = results_[aiger_lit2var(circuit_->ands[cnt].rhs0)];
		int rhs1_val = results_[aiger_lit2var(circuit_->ands[cnt].rhs1)];
		results_[aiger_lit2var(circuit_->ands[cnt].lhs)] = rhs0_val & rhs1_val;
	}

	// print values of outputs
//	for(size_t cnt = 0; cnt < circuit_->num_outputs; ++cnt)
//	{
//		cout << results_[aiger_lit2var(circuit_->outputs[cnt].lit)] << endl;
//	}


	time_index_++;
}
