// ----------------------------------------------------------------------------
// Copyright (c) 2015 by Graz University of Technology
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
/// @file SymbTimeAnalysis.h
/// @brief Contains the declaration of the class SymbTimeAnalysis.
// -------------------------------------------------------------------------------------------

#ifndef SymbTimeAnalysis_H__
#define SymbTimeAnalysis_H__

#include "defines.h"
#include "BackEnd.h"
#include "SatSolver.h"
#include "AigSimulator.h"

// -------------------------------------------------------------------------------------------
///
/// @class SymbTimeAnalysis
/// @brief BackEnd that analyzes circuits for soft-errors. SAT based. point in time is symbolic
///
/// @author Patrick Klampfl
/// @version 1.2.0
class SymbTimeAnalysis: public BackEnd
{
	public:

	using BackEnd::analyze;

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	SymbTimeAnalysis(aiger* circuit, int num_err_latches, int mode = 1);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
	virtual ~SymbTimeAnalysis();

	enum AnalysisMode
	{
		NAIVE = 0, SYMBOLIC_SIMULATION = 1, FREE_INPUTS = 2
	};

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases
///
/// tries to find vulnerabilities using the provided TestCases
///
/// @param testcases a vector of TestCases.
/// @return TRUE if vulnerabilities were found.
	bool analyze(vector<TestCase> &testcases);
	void analyze();


	protected:

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases.
///	Uses the Symbolic time-steps algorithm.
///
/// This NAIVE mode creates and conjuncts a fresh copy of the T_err transition relation to F
/// at each time-step.
///
/// Algorithm ANALYZE1: Only the point in time is symbolic
///
/// @param testcase a vector of input vectors
	void Analyze1_naive(vector<TestCase> &testcases);

// -------------------------------------------------------------------------------------------
///
/// @brief tries to find vulnerabilities using the provided TestCases.
///	Uses the Symbolic time-steps algorithm.
///
/// SYMBOLIC_SIMULATION mode: instead of conjuncting a fresh copy of the T_err
/// transition relation to F, a symbolic simulation of the circuit is performed in order to
/// build the Error-transition-relation on the fly, which should have much fewer clauses
/// due to the fixed input-values.
///
/// Algorithm ANALYZE1: Only the point in time is symbolic
///
/// @param testcase a vector of input vectors
	void Analyze1_symb_sim(vector<TestCase> &testcases);

	void Analyze1_free_inputs(vector<TestCase> &testcases);

	void addErrorTrace(unsigned latch_aig, unsigned err_timestep, map<int, unsigned> &f_to_i,
			const vector<int> &model, const TestCase &tc, bool open_inputs = false);

// -------------------------------------------------------------------------------------------
///
/// @brief the Aiger Simulator instance
	AigSimulator* sim_;

// -------------------------------------------------------------------------------------------
///
/// @brief the Sat-Solver instance
	SatSolver* solver_;

// 0 = disabled, 1 = every iteration, 2 = every 2nd iteration, ...
	unsigned unsat_core_interval_;

	private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	SymbTimeAnalysis(const SymbTimeAnalysis &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	SymbTimeAnalysis& operator=(const SymbTimeAnalysis &other);


	vector<vector<int> > computeRelevantOutputs(TestCase& testcase);

	bool isARelevantOutputDifferent(vector<int>& out1, vector<int>& out2, vector<int>& out_is_relevant);
};

#endif // SymbTimeAnalysis_H__
