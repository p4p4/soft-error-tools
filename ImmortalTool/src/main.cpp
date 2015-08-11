//============================================================================
// Name        : Main.cpp
// Author      : Patrick Klampfl
// Version     : 0.1, 08.2015
//============================================================================
#include "CNF.h"
#include "Stopwatch.h"
#include "Options.h"
#include "defines.h"
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

	srand(time(0));
	//----------------------------------------------------------------------------
	Logger::instance().enable(Logger::DBG);
	// Playground starts here
	L_LOG("File: " << Options::instance().getAigInFileNameOnly());

	BackEnd* sea = Options::instance().getBackEnd();
	sea->findVulnerabilities(10,10); // TODO


	// Playground ends here
	//----------------------------------------------------------------------------

	double cpu_time = Stopwatch::getCPUTimeSec(start_time);
	size_t real_time = Stopwatch::getRealTimeSec(start_time);
	L_LOG("Overall execution time: " << cpu_time << " sec CPU time.");
	L_LOG("Overall execution time: " << real_time << " sec real time.");
	return 0;
}

