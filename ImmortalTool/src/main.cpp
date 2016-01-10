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

//============================================================================
// Name        : Main.cpp
// Author      : Patrick Klampfl
// Version     : 0.1, 08.2015
//============================================================================
#include "CNF.h"
#include "Stopwatch.h"
#include "Options.h"
#include "defines.h"
#include "Utils.h"
#include "Logger.h"
#include "LingelingApi.h"
#include "SimulationBasedAnalysis.h"
#include "Utils.h"

extern "C"
{
#include "aiger.h"
}

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
	PointInTime start_time = Stopwatch::start();
	bool quit = Options::instance().parse(argc, argv);
	if (quit)
		return 0;

	//----------------------------------------------------------------------------
	BackEnd* error_analysis = Options::instance().getBackEnd();
	L_LOG(
			"Back-End: " << Options::instance().getBackEndName() << ", mode = " << Options::instance().getBackEndMode());
	int tc_mode = Options::instance().getTestcaseMode();
	switch (tc_mode)
	{
	case Options::TC_RANDOM:
	{
		error_analysis->analyzeWithRandomTestCases(Options::instance().getNumTestcases(),
				Options::instance().getLenRandTestcases());
		break;
	}
	case Options::TC_FILES:
	{
		error_analysis->analyze(Options::instance().getPathsToTestcases());
		break;
	}
	case Options::TC_MC:
	{
		error_analysis->analyzeModelChecking(Options::instance().getLenRandTestcases());
		break;
	}
	default:
		MASSERT(false, "wrong/no testcase-mode provided")
		;
	}

	L_LOG("#Errors found: " << error_analysis->getNumberOfErrors());
//	for (set<unsigned>::iterator it = vulnerabilities.begin();
//			it != vulnerabilities.end(); ++it)
//	{
//		L_DBG("  Latch " << *it);
//	}

	if (Options::instance().isUseDiagnosticOutput())
	{
		error_analysis->printErrorTraces();
	}

	//----------------------------------------------------------------------------
	double cpu_time = Stopwatch::getCPUTimeSec(start_time);
	size_t real_time = Stopwatch::getRealTimeSec(start_time);
	L_LOG(
			"Overall execution time: " << cpu_time << " sec CPU time, "<< real_time << " sec real time.");
	return 0;
}

