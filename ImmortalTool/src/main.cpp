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

	srand(time(0)); // seed with current time (used for random TestCases)
	//----------------------------------------------------------------------------
	L_LOG("File: " << Options::instance().getAigInFileNameOnly());

	BackEnd* error_analysis = Options::instance().getBackEnd();
	int tc_mode = Options::instance().getTestcaseMode();
	switch (tc_mode)
	{
		case Options::TC_RANDOM:
		{
			error_analysis->findVulnerabilities(Options::instance().getNumTestcases(),
					Options::instance().getLenRandTestcases());
			break;
		}
		case Options::TC_FILES:
		{
			error_analysis->findVulnerabilities(Options::instance().getPathsToTestcases());
			break;
		}
		default: MASSERT(false, "wrong/no testcase-mode provided");
	}
	error_analysis->findVulnerabilities(1, 10); // TODO modes

	const set<unsigned> &vulnerabilities = error_analysis->getVulnerableElements();

	L_LOG("#Vulnerabilities found: " << vulnerabilities.size());
//	for (set<unsigned>::iterator it = vulnerabilities.begin();
//			it != vulnerabilities.end(); ++it)
//	{
//		L_DBG("  Latch " << *it);
//	}

	//----------------------------------------------------------------------------
	double cpu_time = Stopwatch::getCPUTimeSec(start_time);
	size_t real_time = Stopwatch::getRealTimeSec(start_time);
	L_LOG(
			"Overall execution time: " << cpu_time << " sec CPU time, "<< real_time << " sec real time.");
	return 0;
}

