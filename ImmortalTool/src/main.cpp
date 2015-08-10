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
	//----------------------------------------------------------------------------
	// Playground starts here
	CNF cnf;
	LingelingApi ling;
	cout << "empty cnf satisfiable: " << ling.isSat(cnf) << endl;

	cnf.add1LitClause(0);
	cout << "cnf with 0 clause: " << ling.isSat(cnf) << endl;

	// read file:
	aiger* circuit = aiger_init();
//	const char *read_err = aiger_open_and_read_from_file(circuit,
//			"tests/inputs/minmax2_weak.aag");

	const char *read_err = aiger_open_and_read_from_file(circuit,
			"tests/inputs/minmax2_perfect.aag");

	if (read_err != NULL)
	{
		return -1;
	}
	L_LOG("file opened");

	SimulationBasedAnalysis sba(circuit,2);
	vector<char*> tc_files;
	tc_files.push_back("tests/inputs/5_bit_input");
	sba.findVulnerabilities(tc_files);
	const set<unsigned> &vulnerabilities = sba.getVulnerableElements();

	L_LOG("Number of vulnerabilities: " << vulnerabilities.size() << ":");
	for (set<unsigned>::iterator it = vulnerabilities.begin();
			it != vulnerabilities.end(); ++it)
	{
		L_LOG("  Latch " << *it);
	}

	// Playground ends here
	//----------------------------------------------------------------------------

	double cpu_time = Stopwatch::getCPUTimeSec(start_time);
	size_t real_time = Stopwatch::getRealTimeSec(start_time);
	L_LOG("Overall execution time: " << cpu_time << " sec CPU time.");
	L_LOG("Overall execution time: " << real_time << " sec real time.");
	return 0;
}

