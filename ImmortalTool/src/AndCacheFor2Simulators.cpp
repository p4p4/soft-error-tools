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
/// @file AndCacheFor2Simulators.cpp
/// @brief Contains the definition of the class AndCacheFor2Simulators.
// -------------------------------------------------------------------------------------------

#include "AndCacheFor2Simulators.h"
#include "Utils.h"

// -------------------------------------------------------------------------------------------
AndCacheFor2Simulators::AndCacheFor2Simulators(vector<int> &results1, vector<int> &results2,
		SatSolver* solver, int& next_free_cnf_var) :
		results1_(results1), results2_(results2), solver_(solver), next_free_cnf_var_(
				next_free_cnf_var)
{
}

// -------------------------------------------------------------------------------------------
AndCacheFor2Simulators::~AndCacheFor2Simulators()
{
}

void AndCacheFor2Simulators::addAndGate(int lhs_aig_lit, int rhs0_aig_lit, int rhs1_aig_lit)
{

	if ((results1_[rhs0_aig_lit >> 1] == results2_[rhs0_aig_lit >> 1])
			&& (results1_[rhs1_aig_lit >> 1] == results2_[rhs1_aig_lit >> 1]))
	{
		results2_[lhs_aig_lit >> 1] = results1_[lhs_aig_lit >> 1];
	}
	else
	{
//		cout << "cache miss" << endl;
		int res = next_free_cnf_var_++;
//		solver_->addVarToKeep(res);

		int rhs1_cnf_value1 = Utils::readCnfValue(results1_, rhs1_aig_lit);
		int rhs0_cnf_value1 = Utils::readCnfValue(results1_, rhs0_aig_lit);
		int rhs1_cnf_value2 = Utils::readCnfValue(results2_, rhs1_aig_lit);
		int rhs0_cnf_value2 = Utils::readCnfValue(results2_, rhs0_aig_lit);

// Do Sat-Solver calls
// res == rhs1_cnf_value & rhs0_cnf_value:
// Step 1: (rhs1_cnf_value == false) -> (res == false)
		solver_->incAdd2LitClause(rhs1_cnf_value2, -res);
		// Step 2: (rhs0_cnf_value == false) -> (res == false)
		solver_->incAdd2LitClause(rhs0_cnf_value2, -res);
		// Step 3: (rhs0_cnf_value == true && rhs1_cnf_value == true)
		//   -> (res == true)
		solver_->incAdd3LitClause(-rhs1_cnf_value2, -rhs0_cnf_value2, res);

		results2_[lhs_aig_lit >> 1] = res;
	}
}
