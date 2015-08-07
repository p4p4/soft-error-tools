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
	CNF cnf;
	LingelingApi ling;
	cout << "empty cnf satisfiable: " << ling.isSat(cnf) << endl;

	cnf.add1LitClause(0);
	cout << "cnf with 0 clause: " << ling.isSat(cnf) << endl;




	//----------------------------------------------------------------------------

	double cpu_time = Stopwatch::getCPUTimeSec(start_time);
	size_t real_time = Stopwatch::getRealTimeSec(start_time);
	L_LOG("Overall execution time: " << cpu_time << " sec CPU time.");
	L_LOG("Overall execution time: " << real_time << " sec real time.");
	return 0;
}

