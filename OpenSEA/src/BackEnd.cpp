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
/// @file BackEnd.cpp
/// @brief Contains the definition of the class BackEnd.
// -------------------------------------------------------------------------------------------
#include <algorithm>

#include "BackEnd.h"
#include "Utils.h"
#include "ErrorTraceManager.h"
#include "Options.h"
#include "Logger.h"

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
BackEnd::BackEnd(aiger* circuit, int num_err_latches, int mode, aiger* environment_model) :
		circuit_(circuit), environment_model_(environment_model), mode_(mode),
		num_err_latches_(num_err_latches)
{
	MASSERT(!environment_model || environment_model->num_outputs >= circuit->num_outputs - 1, "Error: Environment model has too few outputs!");
}

// -------------------------------------------------------------------------------------------
BackEnd::~BackEnd()
{
	// nothing to be done
}

// -------------------------------------------------------------------------------------------
const set<unsigned>& BackEnd::getVulnerableElements() const
{
	return detected_latches_;
}

unsigned int BackEnd::getNumberOfErrors()
{
	return detected_latches_.size();
}

void BackEnd::printResults()
{
	L_LOG("#Errors found: " << getNumberOfErrors());
	if (Options::instance().isUseDiagnosticOutput())
	{
		ErrorTraceManager::instance().printErrorTraces();
	}
}

void BackEnd::setEnvironmentModel(aiger* environmentModel)
{
	environment_model_ = environmentModel;
}

void BackEnd::storeResultingLatches()
{
	ofstream out_file;

	out_file.open(Options::instance().getLatchesResultPath().c_str());
			MASSERT(out_file,
					"could not write latches result file: " + Options::instance().getLatchesResultPath())

	for(set<unsigned>::iterator it = detected_latches_.begin(); it != detected_latches_.end(); ++it)
	{
		out_file << *it << endl;
	}

	out_file.close();

}
