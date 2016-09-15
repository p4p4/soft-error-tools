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
/// @file ErrorTraceManager.cpp
/// @brief Contains the definition of the class ErrorTraceManager.
// -------------------------------------------------------------------------------------------

#include "ErrorTraceManager.h"
#include "Options.h"
#include "Utils.h"
#include "AigSimulator.h"
#include <fstream>

ErrorTraceManager *ErrorTraceManager::instance_ = NULL;

// -------------------------------------------------------------------------------------------
ErrorTraceManager::ErrorTraceManager()
{
}

// -------------------------------------------------------------------------------------------
ErrorTraceManager::~ErrorTraceManager()
{
}

ErrorTraceManager& ErrorTraceManager::instance()
{
	if (instance_ == NULL)
		ErrorTraceManager::instance_ = new ErrorTraceManager;
	MASSERT(instance_ != NULL, "Could not create ErrorTraceManager instance.");
	return *instance_;
}

string ErrorTraceManager::errorTraceToString(ErrorTrace* et)
{
	ostringstream oss;

	unsigned timestep = et->error_timestep_;
	oss << "Latch: " << et->latch_index_;
	oss << " flipped at i=" << et->flipped_timestep_ << endl;
	aiger* circuit = Options::instance().getCircuit();
	for(unsigned i = 0; i < circuit->num_latches; i++)
	{
		if(circuit->latches[i].lit == et->latch_index_ && circuit->latches[i].name != 0)
		{
			oss << "Latch name: " << circuit->latches[i].name << endl;
			break;
		}
	}

	oss << "Error happened at timestep i=" << timestep << endl;

	AigSimulator sim(circuit);
	AigSimulator sim_ok(circuit);
	oss << "[SIM] i=?: state | inputs | outputs | next state" << endl;
	oss << "-------------------------------------------------" << endl; // todo: make fancy
	for (unsigned j = 0; j <= timestep; j++)
	{
		if (j == et->flipped_timestep_)
			sim.flipValue(et->latch_index_);

		sim.simulateOneTimeStep(et->input_trace_[j]);
		sim_ok.simulateOneTimeStep(et->input_trace_[j]);
		oss << "[ OK] i=" << j << ": " << sim_ok.getStateString() << endl;
		if (j >= et->flipped_timestep_)
		{
			oss << "[ERR] i=" << j << ": " << sim.getStateString();
			if (j == et->flipped_timestep_)
				oss << " <<< flipped in this state!";
			if (j == et->error_timestep_)
				oss << " <<< wrong output in this state!";
			oss << endl;
		}
		sim.switchToNextState();
		sim_ok.switchToNextState();
	}

	return oss.str();
}

void ErrorTraceManager::printErrorTraces()
{
	ostringstream oss;

	oss << "=================================================" << endl;
	for (unsigned i = 0; i < error_traces_.size(); i++)
	{
		oss << errorTraceToString(error_traces_[i]);
		oss << endl << endl;
		oss << "=================================================" << endl;
	}

	if (Options::instance().isDiagnosticOutputToFile())
	{
	  ofstream out_file;
	  out_file.open(Options::instance().getDiagnosticOutputPath().c_str());
		MASSERT(out_file,
				"could not write diagnostic output file: " + Options::instance().getDiagnosticOutputPath())

	  out_file << oss.str();

	  out_file.close();
	}
	else
	{
		cout << endl << endl;
		cout << oss.str();

	}
}
