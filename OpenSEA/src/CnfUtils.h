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
/// @file CnfUtils.h
/// @brief Contains the declaration of the class CnfUtils.
// -------------------------------------------------------------------------------------------

#ifndef CnfUtils_H__
#define CnfUtils_H__

#include "defines.h"
#include "SatSolver.h"
#include "CNF.h"

// -------------------------------------------------------------------------------------------
///
/// @class CnfUtils
/// @brief contains helper functions for working with CNFs
///
/// @author Patrick Klampfl
/// @version 1.2.0
class CnfUtils
{
public:
	static void generateVectorIsDifferentClause(vector<int> &compare_a, vector<int> &compare_b,
			vector<int> &result_clause, int &next_free_cnf_var, SatSolver* solver);

	//CNF tseitinXor(int lhs, int rhs0, int rhs1) TODO add helper functions like this

protected:

private:

	CnfUtils();
	virtual ~CnfUtils();

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	CnfUtils(const CnfUtils &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	CnfUtils& operator=(const CnfUtils &other);

};

#endif // CnfUtils_H__
