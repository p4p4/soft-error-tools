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
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file CnfUtils.cpp
/// @brief Contains the definition of the class CnfUtils.
// -------------------------------------------------------------------------------------------


#include "CnfUtils.h"

void CnfUtils::generateVectorIsDifferentClause(vector<int>& compare_a, vector<int>& compare_b,
		vector<int>& result_clause, int& next_free_cnf_var, SatSolver* solver)
{
	for (unsigned i = 0; i < compare_a.size(); i++)
	{

		if (compare_a[i] == CNF_TRUE && compare_b[i] == CNF_TRUE)
			continue;
		else if (compare_a[i] == CNF_FALSE && compare_b[i] == CNF_FALSE)
			continue;
		else if (compare_a[i] == CNF_TRUE)
			result_clause.push_back(-compare_b[i]);
		else if (compare_a[i] == CNF_FALSE)
			result_clause.push_back(compare_b[i]);
		else if (compare_b[i] == CNF_TRUE)
			result_clause.push_back(-compare_a[i]);
		else if (compare_b[i] == CNF_FALSE)
			result_clause.push_back(compare_a[i]);
		else if (compare_b[i] != compare_a[i]) // both symbolic but not equal
		{
			// c <=> a XOR b:
			// (~c | a | b) &  (~c | ~b | ~a) & (~a | b | c) & (~b | a | c)
			int out_is_diff = next_free_cnf_var++;
			solver->addVarToKeep(out_is_diff);
			result_clause.push_back(out_is_diff);
			solver->incAdd3LitClause(-out_is_diff, compare_a[i], compare_b[i]);
			solver->incAdd3LitClause(-out_is_diff, -compare_a[i], -compare_b[i]);
			solver->incAdd3LitClause(out_is_diff, -compare_a[i], compare_b[i]);
			solver->incAdd3LitClause(out_is_diff, compare_a[i], -compare_b[i]);
		}
	}
}
