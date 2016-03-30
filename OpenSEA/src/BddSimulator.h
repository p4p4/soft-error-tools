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
/// @file BddSimulator.h
/// @brief Contains the declaration of the class BddSimulator.
// -------------------------------------------------------------------------------------------

#ifndef BddSimulator_H__
#define BddSimulator_H__

#include "defines.h"

extern "C" {
#include "aiger.h"
#include "cudd.h"
};
#include "cuddObj.hh"

// -------------------------------------------------------------------------------------------
///
/// @class BddSimulator
/// @brief This class can simulate aiger circuits. during the simulation, the circuit
/// is being converted to an unrolled transition-relation on the fly and added to an
/// incremental SAT-Solver session.
///
/// @author Patrick Klampfl
/// @version 1.2.0
class BddSimulator
{
	public:

// -------------------------------------------------------------------------------------------
///
/// @brief Creates a BddSimulator instance to a given circuit and an incr SAT session
	BddSimulator(aiger* circuit, const Cudd &cudd, int& next_free_cnf_var_reference);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
	virtual ~BddSimulator();

// -------------------------------------------------------------------------------------------
//
/// @brief Simulates one Timestep using an input-vector
///
/// Computes all outputs of a circuit according to the provided input-vector
/// and the current latch values
///
/// @param input_values The input vector
	void simulateOneTimeStep(const vector<int> &input_values);

// -------------------------------------------------------------------------------------------
//
/// @brief Simulates one Timestep using an input-vector and a state-vector
///
/// Computes all outputs of a circuit according to the provided input-vector
/// and the *provided* latch values
///
/// @param input_values The input vector
/// @param latch_values The state vector containing the values of the latches
	void simulateOneTimeStep(const vector<int> &input_values, const vector<int> &latch_values);

// -------------------------------------------------------------------------------------------
//
/// @brief Simulates one Timestep
///
/// Computes all outputs of a circuit according to the current latch values and
/// the current input-values (if any).
/// Attention: 	If the circuit_ has inputs, call setInputValues() beforehand, or use one of
/// 						the	other simulateOneTimeStep() functions which take input values as argument
///
	void simulateOneTimeStep();

// -------------------------------------------------------------------------------------------
//
/// @brief Sets the input values for the next simulation step
///
/// The next call of simulateOneTimeStep(void) will use the provided values for the simulation
///
/// @param input_values The input vector
	void setInputValues(const vector<int> &input_values);


	void setBddInputValues(const vector<BDD> &input_values_as_bdd);

// -------------------------------------------------------------------------------------------
//
/// @brief Sets a value for a given cnf-variable.
///
/// Set a value for a given variable. This can be useful to, for example, modify the circuit
/// araund the latches (if you add appropriate clauses to the given SAT- solver_ instance
/// (e.g. f variables in SymbTimeLocation Algorithm, f and c variables in SymbTimeLocationA.)
///
/// @param cnf_lit the variable where you want to set a value
/// @param cnf_value the value to store for the given cnf_lit variable
	void setResultValue(unsigned cnf_lit, BDD cnf_value);

// -------------------------------------------------------------------------------------------
//
/// @brief returns the value for a given cnf-variable.
///
/// @return the value (=result) of the simulation for a given cnf variable
	BDD getResultValue(unsigned cnf_lit);

// -------------------------------------------------------------------------------------------
//
/// @brief returns the value of the alarm output.
///
/// @return the value of the alarm output (in cnf-literal representation)
	BDD getAlarmValue();

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
/// @brief Returns a vector of all the output-values
///
/// @return vector containing the values of the outputs
	const vector<BDD> &getOutputValues();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a vector of all the latch-values
///
/// @return vector containing the values of the latches
	const vector<BDD> &getLatchValues();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a vector of all the next-latch-values
///
/// @return vector containing the next values of the latches
	const vector<BDD> &getNextLatchValues();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a vector of all the input-values
/// Undefined '?' inputs are replaced by fresh variables.
///
/// @return vector containing the input values.
	const vector<BDD> &getInputValues();

// -------------------------------------------------------------------------------------------
///
/// @brief (re)sets the latch values to Zero
///
	void initLatches();


// -------------------------------------------------------------------------------------------
///
/// @brief returns a list of newly created literals representing the free inputs
/// (free inputs are input values to a given time-step and input which has no given concrete
/// input value like 0,1. the SAT-Solver can choose the value instead).
///
	const vector<BDD>& getOpenInputVars() const;

	protected:

  static inline BDD readCnfValue(const vector<BDD> results, unsigned aigerlit)
  {
	return aigerlit & 1 ? ~results[(aigerlit >> 1)] : results[(aigerlit >> 1)];
  }

	const Cudd &cudd_;


// -------------------------------------------------------------------------------------------
///
/// @brief The circuit to simulate in AIGER representation
///
	aiger* circuit_;

// -------------------------------------------------------------------------------------------
///
/// @brief reference to the next free cnf-var. this is used for open input-values in TestCases
	int& next_free_cnf_var_;


// -------------------------------------------------------------------------------------------
///
/// @brief Array storing the current BDD for each variable
///
	vector<BDD> results_;


	private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	BddSimulator(const BddSimulator &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	BddSimulator& operator=(const BddSimulator &other);

};

#endif // BddSimulator_H__

