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
/// @file SatAssignmentParser.h
/// @brief Contains the declaration of the class SatAssignmentParser.
// -------------------------------------------------------------------------------------------

#ifndef SatAssignmentParser_H__
#define SatAssignmentParser_H__

#include "defines.h"

// -------------------------------------------------------------------------------------------
///
/// @class SatAssignmentParser
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class SatAssignmentParser
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
  SatAssignmentParser();

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~SatAssignmentParser();

  void addLiteralOfInterest(int literal, string description);
  void addVectorOfInterest(vector<int> literals, string description);
  void parseAssignment(vector<int> assignment);
  int findFirstPositiveAssignmentOfVector(vector<int> assignment, vector<int> literals);

	vector<int> getVarsOfInterrest()
	{
		return vars_of_interrest_;
	}

	void reset()
	{
		var_name_.clear();
		vars_of_interrest_.clear();
	}

protected:
  vector<string> var_name_;
  vector<int> vars_of_interrest_;

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  SatAssignmentParser(const SatAssignmentParser &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  SatAssignmentParser& operator=(const SatAssignmentParser &other);

};

#endif // SatAssignmentParser_H__
