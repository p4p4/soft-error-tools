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
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file DefinitelyProtected.h
/// @brief Contains the declaration of the class DefinitelyProtected.
// -------------------------------------------------------------------------------------------

#ifndef DefinitelyProtected_H__
#define DefinitelyProtected_H__

#include "defines.h"
#include "BackEnd.h"

struct aiger;

// -------------------------------------------------------------------------------------------
///
/// @class DefinitelyProtected
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class DefinitelyProtected : public BackEnd
{
public:
	using BackEnd::analyze;


	enum AnalysisMode
	{
		STANDARD = 0
	};
// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	DefinitelyProtected(aiger* circuit, int num_err_latches, int mode = 0);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
	virtual ~DefinitelyProtected();



	void analyze();
	bool findDefinitelyProtected_1();





protected:
	// -------------------------------------------------------------------------------------------
	///
	/// @brief the circuit to analyze
	aiger* circuit_;

	// -------------------------------------------------------------------------------------------
	///
	/// @brief the mode (if there are more than one version and/or optimizations to enable)
	int mode_;

	// -------------------------------------------------------------------------------------------
	///
	/// @brief the number of error-latches. Error latches are additional latches used for the
	/// protection circuit.
	unsigned num_err_latches_;



private:


// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
	DefinitelyProtected(const DefinitelyProtected &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	DefinitelyProtected& operator=(const DefinitelyProtected &other);

};

#endif // DefinitelyProtected_H__
