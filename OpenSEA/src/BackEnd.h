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
/// @file BackEnd.h
/// @brief Contains the declaration of the class BackEnd.
// -------------------------------------------------------------------------------------------

#ifndef BackEnd_H__
#define BackEnd_H__

#include "defines.h"

struct aiger;
// -------------------------------------------------------------------------------------------
///
/// @class BackEnd
/// @brief An interface for the back-ends.
///
/// This class provides an interface for all back-ends. Every back-end implements a different
/// Soft-Error-Analysis algorithm. This class is abstract, i.e., objects of this class cannot
/// be instantiated. Instantiate one of the derived classes instead.
///
/// @author Patrick Klampfl
/// @version 1.2.0
class BackEnd
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	BackEnd(aiger* circuit, int num_err_latches, int mode, aiger* environment_model = 0);

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
	virtual ~BackEnd();

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the set of latches detected by analyze()
///
/// @return The set of latches detected by analyze()
	const set<unsigned>& getDetectedLatches() const;

	virtual void printResults();

	virtual void analyze() = 0;

// -------------------------------------------------------------------------------------------
///
/// @brief allows to set an environment-model
///
/// an environment-model is an aiger-circuit with the same number of inputs and outputs as the
/// original circuit. These outputs define when an output of the original may be (ir)relevant.
///
/// @param environmentModel the environment-model
	void setEnvironmentModel(aiger* environmentModel);

	void storeResultingLatches();

protected:
// -------------------------------------------------------------------------------------------
///
/// @brief the circuit to analyze
	aiger* circuit_;

	aiger* environment_model_;

// -------------------------------------------------------------------------------------------
///
/// @brief the mode (if there are more than one version and/or optimizations to enable)
	int mode_;

// -------------------------------------------------------------------------------------------
///
/// @brief the list of vulnerabilities / protected latches / false positives found so for
	set<unsigned> detected_latches_;


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
	BackEnd(const BackEnd &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
	BackEnd& operator=(const BackEnd &other);

};

#endif // BackEnd_H__
