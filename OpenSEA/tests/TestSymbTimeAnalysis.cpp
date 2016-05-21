// ----------------------------------------------------------------------------
// Copyright (c) 2013-2014 by Graz University of Technology and
//                            Johannes Kepler University Linz
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
// For more information about this software see
//   <http://www.iaik.tugraz.at/content/research/design_verification/others/>
// or email the authors directly.
//
// ----------------------------------------------------------------------------

#include "TestSymbTimeAnalysis.h"
#include "../src/SimulationBasedAnalysis.h" // for comparison
#include "../src/Utils.h"
#include "../src/Logger.h"
#include "../src/TestCaseProvider.h"

extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION (TestSymbTimeAnalysis);

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::setUp()
{
	//setup for testcases
	Logger::instance().disable(Logger::LOG);
}

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::tearDown()
{
	//define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::checkVulnerabilities(string path_to_aiger_circuit,
		vector<string> tc_files, set<unsigned> should_be_vulnerable,
		int num_err_latches, int mode)
{

	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open " + path_to_aiger_circuit, circuit != 0);

	SymbTimeAnalysis sta(circuit, num_err_latches, mode);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sta.analyze(tcs);
	const set<unsigned> &vulnerabilities = sta.getDetectedLatches();

	// DEBUG: print the vulnerable latches
//	for (set<unsigned>::iterator it = vulnerabilities.begin();
//			it != vulnerabilities.end(); ++it)
//	{
//		cout << "  Latch " << *it << endl;
//	}

	aiger_reset(circuit);

	CPPUNIT_ASSERT_MESSAGE("circuit was: " + path_to_aiger_circuit,
			vulnerabilities == should_be_vulnerable);
}

void TestSymbTimeAnalysis::compareWithSimulation(string path_to_aiger_circuit,
		int num_tc, int num_timesteps, int num_err_latches, int mode)
{
	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open " + path_to_aiger_circuit, circuit != 0);

	srand(0xCAFECAFE);
	SymbTimeAnalysis sta(circuit, num_err_latches, mode);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().generateRandomTestCases(num_tc, num_timesteps);
	sta.analyze(tcs);
	const set<unsigned> &symb_vulnerabilities = sta.getDetectedLatches();

	srand(0xCAFECAFE); // SymbTimeAnalysis and SimulationBasedAnalysis must have same "random" inputs
	SimulationBasedAnalysis sba(circuit, num_err_latches);
	sba.analyze(tcs);
	const set<unsigned> &sim_vulnerabilities = sba.getDetectedLatches();
	L_INF("test: " << path_to_aiger_circuit);
	L_INF(
			"vulnerabilities found with SIMULATION="<<sim_vulnerabilities.size() <<", with SYMBTIME="<<symb_vulnerabilities.size());

	for (set<unsigned>::iterator it = symb_vulnerabilities.begin();
			it != symb_vulnerabilities.end(); ++it)
	{
		L_INF("[symb]Latch " << *it);
	}
	for (set<unsigned>::iterator it = sim_vulnerabilities.begin();
			it != sim_vulnerabilities.end(); ++it)
	{
		L_INF("[sim]Latch " << *it);
	}

	aiger_reset(circuit);

	CPPUNIT_ASSERT_MESSAGE(path_to_aiger_circuit,
			sim_vulnerabilities == symb_vulnerabilities);
}


// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::test4_analysis_w_1_extra_latch()
{

	// Paths to TestCase files
	// A TestCase file contains vectors of input values
	vector<string> tc_files;
	tc_files.push_back("inputs/3b");
//	tc_files.push_back("inputs/3_bit_input_1");
//	tc_files.push_back("inputs/3_bit_input_2");
//	tc_files.push_back("inputs/3_bit_input_3");
//	tc_files.push_back("inputs/3_bit_input_4");
//	tc_files.push_back("inputs/3_bit_input_5");

	//-------------------------------------------
	// test 4a: 3 of 3 latches protected
//	Logger::instance().enable(Logger::DBG);
	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilities("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::NAIVE);
//	Logger::instance().disable(Logger::DBG);
	//-------------------------------------------
	// test 4b: 2 of 3 latches protected

	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilities("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::NAIVE);

	//-------------------------------------------
	// test 4c: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::NAIVE);

	//-------------------------------------------
	// test 4d: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0, SymbTimeAnalysis::NAIVE);

}

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::test5_analysis_w_2_extra_latch()
{
	// Paths to TestCase files
	// A TestCase file contains vectors of input values
	vector<string> tc_files;
	tc_files.push_back("inputs/3_bit_input_1");
	tc_files.push_back("inputs/3_bit_input_2");
	tc_files.push_back("inputs/3_bit_input_3");
	tc_files.push_back("inputs/3_bit_input_4");
	tc_files.push_back("inputs/3_bit_input_5");

	set<unsigned> should_be_vulnerable;
	should_be_vulnerable.insert(10); // Latch 10 in vulnerable
	checkVulnerabilities("inputs/toggle2l.aag", tc_files, should_be_vulnerable,
			2, SymbTimeAnalysis::NAIVE);
}


// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::test7_compare_with_simulation_1()
{
	Logger::instance().enable(Logger::INF);
	Logger::instance().disable(Logger::INF);
	compareWithSimulation("inputs/toggle.perfect.aag", 1, 2, 1,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/toggle.1vulnerability.aag", 1, 2, 1,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/toggle.2vulnerabilities.aag", 1, 2, 1,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/toggle.3vulnerabilities.aag", 1, 2, 0,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/iwls02texasa.2vul.1l.aag", 5, 5, 1,
			SymbTimeAnalysis::NAIVE); // TODO: 0 vulnerabilities?
	compareWithSimulation("inputs/ex5.2vul.1l.aig", 5, 5, 1,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/ex5.2vul.2l.aig", 5, 5, 2,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/beecount-synth.2vul.1l.aig", 2, 5, 1,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/s27.1vul.1l", 2, 1, 1,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/shiftreg.2vul.1l.aig", 1, 2, 1,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/traffic-synth.5vul.1l.aig", 5, 14, 1,
			SymbTimeAnalysis::NAIVE);
	compareWithSimulation("inputs/s5378.50percent.aag", 3, 5, 2,
			SymbTimeAnalysis::NAIVE); // 164 Latches!
}

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::test8_symbolic_simulation_basic()
{


	// Paths to TestCase files
	// A TestCase file contains vectors of input values
	vector<string> tc_files;
	tc_files.push_back("inputs/3b");
	tc_files.push_back("inputs/3_bit_input_1");
//	tc_files.push_back("inputs/3_bit_input_2");
//	tc_files.push_back("inputs/3_bit_input_3");
//	tc_files.push_back("inputs/3_bit_input_4");
//	tc_files.push_back("inputs/3_bit_input_5");

	//-------------------------------------------
	// test 8c: 3 of 3 latches protected
	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilities("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::SYMBOLIC_SIMULATION);

	//-------------------------------------------
	// test 8d: 2 of 3 latches protected
	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilities("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::SYMBOLIC_SIMULATION);

	//-------------------------------------------
	// test 8e: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::SYMBOLIC_SIMULATION);

	//-------------------------------------------
	// test 8f: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0, SymbTimeAnalysis::SYMBOLIC_SIMULATION);

}

void TestSymbTimeAnalysis::test10_symbolic_simulation_compare_w_simulation()
{
//	Logger::instance().enable(Logger::INF);
//	Logger::instance().enable(Logger::DBG);

	compareWithSimulation("inputs/traffic-synth.5vul.1l.aig", 8, 2, 1,
			SymbTimeAnalysis::SYMBOLIC_SIMULATION);

	compareWithSimulation("inputs/shiftreg.3vul.0l.aig", 1, 1, 0,
			SymbTimeAnalysis::SYMBOLIC_SIMULATION);

	// TODO: check this out. symb[naive] = 3, sim= 1:
//	compareWithSimulation("inputs/shiftreg.3vul.0l.aig", 1, 1, 0, SymbTimeAnalysis::NAIVE);

	compareWithSimulation("inputs/s27.1vul.1l", 2, 1, 1,
			SymbTimeAnalysis::SYMBOLIC_SIMULATION);
}
