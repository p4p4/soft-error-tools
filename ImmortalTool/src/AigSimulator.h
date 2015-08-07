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
/// @brief TODO
///
/// @author TODO
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
	void setTestcase(char* path_to_aigsim_input);

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
	const vector<int>& getOutputs() const;

	protected:
	aiger* circuit_;
	vector<vector<int> > testcase_;
	int* results_;
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
