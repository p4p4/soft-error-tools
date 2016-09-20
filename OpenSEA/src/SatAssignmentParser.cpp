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
/// @file SatAssignmentParser.cpp
/// @brief Contains the definition of the class SatAssignmentParser.
// -------------------------------------------------------------------------------------------


#include "SatAssignmentParser.h"

// -------------------------------------------------------------------------------------------
SatAssignmentParser::SatAssignmentParser()
{
}

// -------------------------------------------------------------------------------------------
SatAssignmentParser::~SatAssignmentParser()
{
}

void SatAssignmentParser::addLiteralOfInterest(int literal, string description)
{
	var_name_.push_back(description);
	vars_of_interrest_.push_back(literal);
}

void SatAssignmentParser::addVectorOfInterest(vector<int> literals, string description)
{
	for (unsigned i = 0; i < literals.size(); i++)
	{
		stringstream ss;
		ss << description << "<" << i << ">";
		addLiteralOfInterest(literals[i], ss.str());
	}
}

void SatAssignmentParser::parseAssignment(vector<int> assignment)
{
	for(unsigned i = 0; i < assignment.size(); i++) {
		bool result;
		if (vars_of_interrest_[i] < 0)
			result = assignment[i] < 0;
		else
			result = assignment[i] > 0;
		cout << var_name_[i] << " (" << vars_of_interrest_[i] << ") = " << result << endl;
	}
}

int SatAssignmentParser::findFirstPositiveAssignmentOfVector(vector<int> assignment,
		vector<int> literals)
{
	map<int,int> literal_to_assignment;

	for(unsigned i = 0; i < assignment.size(); i++)
	{
		literal_to_assignment[abs(assignment[i])] = assignment[i] > 0;
	}

	for(unsigned i = 0; i < literals.size(); i++)
	{
		if (literal_to_assignment[literals[i]] > 0)
			return literals[i];
	}

	return 0;
}
