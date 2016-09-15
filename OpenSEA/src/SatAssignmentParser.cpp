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
	lit_to_description_map_[literal] = description;
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
		/*bool assignmend_negated = false;
		map<int,string>::iterator it = lit_to_description_map_.find(assignment[i]);
		if (it == lit_to_description_map_.end()) {
			lit_to_description_map_.find(-assignment[i]);
			bool assignment_negated = true;
		}*/

		map<int,string>::iterator it = lit_to_description_map_.find(abs(assignment[i]));
		if (it != lit_to_description_map_.end()) {
			cout << it->first << "=" << ((assignment[i] > 0) ? 1 : 0) << " (" << it->second << ")" << endl;
		}
		else
		{
			cout << assignment[i] << "not found" << endl;
		}


	}
}
