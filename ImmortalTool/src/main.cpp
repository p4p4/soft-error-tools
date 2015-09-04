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
#include "ErrorTraceManager.h"
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
		error_analysis->findVulnerabilities(Options::instance().getNumTestcases(),
				Options::instance().getLenRandTestcases());
		break;
	}
	case Options::TC_FILES:
	{
		error_analysis->findVulnerabilities(Options::instance().getPathsToTestcases());
		break;
	}
	default:
		MASSERT(false, "wrong/no testcase-mode provided")
		;
	}

	const set<unsigned> &vulnerabilities = error_analysis->getVulnerableElements();

	L_LOG("#Vulnerabilities found: " << vulnerabilities.size());
//	for (set<unsigned>::iterator it = vulnerabilities.begin();
//			it != vulnerabilities.end(); ++it)
//	{
//		L_DBG("  Latch " << *it);
//	}

	if (Options::instance().isUseDiagnosticOutput())
	{
		vector<ErrorTrace*> &et = ErrorTraceManager::instance().error_traces_;
		cout << "--------------------------------------------" << endl;
		for (unsigned i = 0; i < et.size(); i++)
		{
			unsigned timestep = et[i]->error_timestep_;
			cout << "Latch: " << et[i]->latch_index_ << " flipped at " << et[i]->flipped_timestep_
					<< endl;
			cout << "Error happened at timestep " << timestep << endl;

			Utils::logPrint(et[i]->output_is_, "output was");
			Utils::logPrint(et[i]->output_shouldbe_, "should have been");
			cout << "input vectors:" << endl;
			for (unsigned j = 0; j <= timestep; j++)
			{
				Utils::logPrint(et[i]->input_trace_[j], "step ");
			}
			cout << "--------------------------------------------" << endl;
		}
	}

	//----------------------------------------------------------------------------
	double cpu_time = Stopwatch::getCPUTimeSec(start_time);
	size_t real_time = Stopwatch::getRealTimeSec(start_time);
	L_LOG(
			"Overall execution time: " << cpu_time << " sec CPU time, "<< real_time << " sec real time.");
	return 0;
}

