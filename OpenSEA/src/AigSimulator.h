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
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file AigSimulator.h
/// @brief Contains the declaration of the class AigSimulator.
// -------------------------------------------------------------------------------------------

#ifndef AigSimulator_H__
#define AigSimulator_H__

#include "defines.h"

struct aiger;

// -------------------------------------------------------------------------------------------
///
/// @class AigSimulator
/// @brief Simulates an AIGER circuit with concrete input-values
///
/// @author Patrick Klampfl
/// @version 1.2.0
class AigSimulator
{
	public:

// -------------------------------------------------------------------------------------------
///
/// @brief Creates a new AigerSimulator instance to a given circuit.
	AigSimulator(aiger* circuit);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
	virtual ~AigSimulator();


// -------------------------------------------------------------------------------------------
//
/// @brief Parses an AigSim input file and sets the Testcase accordingly.
///
/// A testcase is a set of input-vectors
///
/// @param path_to_aigsim_input The path to the AigSim input file
	void setTestcase(string path_to_aigsim_input);

// -------------------------------------------------------------------------------------------
//
/// @brief Sets the Testcase.
///
/// A testcase is a set of input-vectors
///
/// @param testcase The test case
	void setTestcase(const vector<vector<int> > &testcase);

// -------------------------------------------------------------------------------------------
//
/// @brief Simpulates one Timestep. Testcase has to be provided beforehand!
///
/// Computes all outputs of a circuit according to the current input-vector
/// and the current latch values
///
	bool simulateOneTimeStep();

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
/// @brief Switch to next State: Latch-outputs get their next-state values
///
/// Computes all outputs of a circuit according to the current input-vector
/// and the current latch values
///
	void switchToNextState();

	void flipValue(unsigned aiger_lit);

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
	vector<int> getOutputs();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a vector of all the latch-values
///
	vector<int> getLatchValues();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a vector of all the next-state latch-values
///
	vector<int> getNextLatchValues();

// -------------------------------------------------------------------------------------------
///
/// @brief (re)sets the latch values to Zero
///
	void initLatches();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a reference to the provided test-case
///
	const TestCase& getTestcase() const
	{
		return testcase_;
	}


	protected:

// -------------------------------------------------------------------------------------------
///
/// @brief The circuit to simulate in AIGER representation
///
	aiger* circuit_;

// -------------------------------------------------------------------------------------------
///
/// @brief The TestCase containing a list of input-vectors
///
	TestCase &testcase_;
	TestCase testcase_empty_; // empty TC

// -------------------------------------------------------------------------------------------
///
/// @brief Array storing the current values [0 or 1] for each variable
///
	int* results_;

// -------------------------------------------------------------------------------------------
///
/// @brief The current time-step. Only used for getVerboseString() and for debugging
///
	size_t time_index_;


	private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	AigSimulator(const AigSimulator &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	AigSimulator& operator=(const AigSimulator &other);

};

#endif // AigSimulator_H__
