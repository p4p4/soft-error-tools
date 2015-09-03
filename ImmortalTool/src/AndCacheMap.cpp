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
/// @file AndCacheMap.cpp
/// @brief Contains the definition of the class AndCacheMap.
// -------------------------------------------------------------------------------------------


#include "AndCacheMap.h"

// -------------------------------------------------------------------------------------------
AndCacheMap::AndCacheMap(SatSolver* solver) : solver_(solver)
{
}

// -------------------------------------------------------------------------------------------
AndCacheMap::~AndCacheMap()
{
}

// -------------------------------------------------------------------------------------------
int AndCacheMap::addAndGate(int left, int right, int& next_free_cnf_var)
{

	if(left > right)
	{
		int tmp = left;
		left = right;
		right = tmp;
	}

	uint64_t comb = static_cast<uint64_t>(left) << 32;
	comb |=  static_cast<uint64_t>(right) & 0x00000000FFFFFFFFULL;


	pair<uint64_t, int> new_cache_item(comb, next_free_cnf_var);
	pair<map<uint64_t, int>::iterator, bool> found = cache_.insert(new_cache_item);
	if(found.second) // we do not have such an AND gate yet
	{

		int res = next_free_cnf_var++;
//		solver_->addVarToKeep(res);

		// Do Sat-Solver calls
		// res == rhs1_cnf_value & rhs0_cnf_value:
		// Step 1: (rhs1_cnf_value == false) -> (res == false)
		solver_->incAdd2LitClause(left, -res);
		// Step 2: (rhs0_cnf_value == false) -> (res == false)
		solver_->incAdd2LitClause(right, -res);
		// Step 3: (rhs0_cnf_value == true && rhs1_cnf_value == true)
		//   -> (res == true)
		solver_->incAdd3LitClause(-left, -right, res);

		return res;
	}
	//else  we already have this AND gate
//	cout << "in cache: " << found.first->second << endl;
	return found.first->second;
}

void AndCacheMap::clearCache()
{
	cache_.clear();
}
