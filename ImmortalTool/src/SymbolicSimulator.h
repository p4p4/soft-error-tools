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
/// @file SymbolicSimulator.h
/// @brief Contains the declaration of the class SymbolicSimulator.
// -------------------------------------------------------------------------------------------

#ifndef SymbolicSimulator_H__
#define SymbolicSimulator_H__

#include "defines.h"

#include "SatSolver.h"

struct aiger;

// -------------------------------------------------------------------------------------------
///
/// @class SymbolicSimulator
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class SymbolicSimulator
{
	public:

// -------------------------------------------------------------------------------------------
///
/// @brief Creates a new AigerSimulator instance to a given circuit.
	SymbolicSimulator(aiger* circuit, SatSolver* solver, int& next_free_cnf_var_reference);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
	virtual ~SymbolicSimulator();


// -------------------------------------------------------------------------------------------
//
/// @brief Simpulates one Timestep using an input-vector
///
/// Computes all outputs of a circuit according to the provided input-vector
/// and the current latch values
///
/// @param input_values The input vector
	void simulateOneTimeStep(const vector<int> &input_values);

// -------------------------------------------------------------------------------------------
//
/// @brief Simpulates one Timestep using an input-vector and a state-vector
///
/// Computes all outputs of a circuit according to the provided input-vector
/// and the *provided* latch values
///
/// @param input_values The input vector
/// @param latch_values The state vector containing the values of the latches
	void simulateOneTimeStep(const vector<int> &input_values, const vector<int> &latch_values);

// -------------------------------------------------------------------------------------------
//
/// @brief Simpulates one Timestep
///
/// Computes all outputs of a circuit according to the current latch values and
/// the current input-values (if any).
/// Attention: 	If the circuit_ has inputs, call setInputValues() beforehand, or use one of
/// 						the	other simulateOneTimeStep() functions which take input values as argument
///
	void simulateOneTimeStep();

	void setInputValues(const vector<int> &input_values);

	void setResultValue(unsigned cnf_lit, int cnf_value);
	int getResultValue(unsigned cnf_lit);

	int getAlarmValue();



// -------------------------------------------------------------------------------------------
//
/// @brief Switch to next State: Latch-outputs get their next-state values
///
/// Computes all outputs of a circuit according to the current input-vector
/// and the current latch values
///
	void switchToNextState();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a string representation of the current state
///
/// The returned storing has the same format as an output-line of the AigSim tool.
/// It is a line containing the following:
/// - latch values
/// - input values
/// - output values
/// - latch next state values
///
/// @return A string representation of the current state
	string getStateString();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a string representation of the current state (including intermediate values)
///
/// The (long!) string contains the following:
/// - latch values
/// - input values
/// - *** input and output values of AND gates ***
/// - output values
/// - latch next state values
///
/// @return A string representation of the current state
	string getVerboseStateString();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a vector of all the output-values
///
///
/// @return vector containing the values of the outputs
	const vector<int> &getOutputValues();

	const vector<int> &getLatchValues();

	const vector<int> &getNextLatchValues();



	protected:

	vector<int> output_values_;
	vector<int> latch_values_;
	vector<int> next_values_;

// -------------------------------------------------------------------------------------------
///
/// @brief The circuit to simulate in AIGER representation
///
	aiger* circuit_;

	SatSolver* solver_;

	int& next_free_cnf_var_;


// -------------------------------------------------------------------------------------------
///
/// @brief Array storing the current values for each variable
///
	vector<int> results_;

// -------------------------------------------------------------------------------------------
///
/// @brief The current time-step. Only used for getVerboseString() and for debugging
///
	size_t time_index_;

// -------------------------------------------------------------------------------------------
///
/// @brief (re)sets the latch values to Zero
///
	void initLatches();


	private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	SymbolicSimulator(const SymbolicSimulator &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	SymbolicSimulator& operator=(const SymbolicSimulator &other);

};

#endif // SymbolicSimulator_H__
