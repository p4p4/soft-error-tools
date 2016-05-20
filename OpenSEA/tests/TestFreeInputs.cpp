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

#include <stdint.h> // include this header for uint64_t
#include "TestFreeInputs.h"
#include "../src/BddAnalysis.h"
#include "../src/SymbTimeAnalysis.h"
#include "../src/SymbTimeLocationAnalysis.h"
#include "../src/SimulationBasedAnalysis.h" // for comparison
#include "../src/Utils.h"
#include "../src/Options.h"
#include "../src/Logger.h"
#include "../src/TestCaseProvider.h"


extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestFreeInputs);

// -------------------------------------------------------------------------------------------
void TestFreeInputs::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestFreeInputs::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestFreeInputs::checkVulnerabilitiesSTLA(string path_to_aiger_circuit,
		vector<string> tc_files, set<unsigned> should_be_vulnerable,
		int num_err_latches, int mode)
{

	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open " + path_to_aiger_circuit, circuit != 0);

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);

	SymbTimeLocationAnalysis sta(circuit, num_err_latches, mode);
	sta.analyze(tcs);
	const set<unsigned> &vulnerabilities = sta.getVulnerableElements();

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

// -------------------------------------------------------------------------------------------
void TestFreeInputs::checkVulnerabilitiesSTA(string path_to_aiger_circuit,
		vector<string> tc_files, set<unsigned> should_be_vulnerable,
		int num_err_latches, int mode)
{

	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open " + path_to_aiger_circuit, circuit != 0);

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);

	SymbTimeAnalysis sta(circuit, num_err_latches, mode);
	sta.analyze(tcs);
	const set<unsigned> &vulnerabilities = sta.getVulnerableElements();

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


void TestFreeInputs::checkVulnerabilitiesSIM(string path_to_aiger_circuit, vector<string> tc_files,
		set<unsigned> should_be_vulnerable, int num_err_latches, int mode)
{
	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open " + path_to_aiger_circuit, circuit != 0);

	SimulationBasedAnalysis sim(circuit, num_err_latches, mode);

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);

	sim.analyze(tcs);
	const set<unsigned> &vulnerabilities = sim.getVulnerableElements();

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

void TestFreeInputs::checkVulnerabilitiesBDD(string path_to_aiger_circuit, vector<string> tc_files,
		set<unsigned> should_be_vulnerable, int num_err_latches, int mode)
{
	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open " + path_to_aiger_circuit, circuit != 0);

	BddAnalysis bdd(circuit, num_err_latches, mode);

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);

	bdd.analyze(tcs);
	const set<unsigned> &vulnerabilities = bdd.getVulnerableElements();

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

// -------------------------------------------------------------------------------------------
void TestFreeInputs::test1()
{
	vector<string> tc_files;
	tc_files.push_back("inputs/3b_w_free_inputs");

	//-------------------------------------------
	// test 4a: 3 of 3 latches protected
//	Logger::instance().enable(Logger::DBG);
	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilitiesSTLA("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeLocationAnalysis::FREE_INPUTS);
	//-------------------------------------------
	// test 4b: 2 of 3 latches protected

	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilitiesSTLA("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeLocationAnalysis::FREE_INPUTS);

	//-------------------------------------------
	// test 4c: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilitiesSTLA("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeLocationAnalysis::FREE_INPUTS);

	//-------------------------------------------
	// test 4d: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilitiesSTLA("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0, SymbTimeLocationAnalysis::FREE_INPUTS);
}

void TestFreeInputs::test2()
{
	aiger* circuit = Utils::readAiger("inputs/s5378.50percent.aag");
	CPPUNIT_ASSERT_MESSAGE("can not open inputs/s5378.50percent.aag", circuit != 0);
	Options::instance().setUnsatCoreInterval(0);
	vector<string> tc_files;
	tc_files.push_back("inputs/35_bit_input_2");

	SymbTimeLocationAnalysis stla(circuit, 2, SymbTimeLocationAnalysis::FREE_INPUTS);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	stla.analyze(tcs);
	const set<unsigned> &vulnerabilities = stla.getVulnerableElements();

	// DEBUG: print the vulnerable latches
//		for (set<unsigned>::iterator it = vulnerabilities.begin();
//				it != vulnerabilities.end(); ++it)
//		{
//			cout << "  Latch " << *it << endl;
//		}
//	cout << "number of detected vulnerabilities: " << vulnerabilities.size() << endl;
	CPPUNIT_ASSERT(vulnerabilities.size() == 65);

	aiger_reset(circuit);
}

void TestFreeInputs::test3()
{
	vector<string> tc_files;
	tc_files.push_back("inputs/3b_w_free_inputs");

	//-------------------------------------------
	// test 4a: 3 of 3 latches protected
//	Logger::instance().enable(Logger::DBG);
	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilitiesSTA("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::FREE_INPUTS);
	//-------------------------------------------
	// test 4b: 2 of 3 latches protected

	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilitiesSTA("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::FREE_INPUTS);

	//-------------------------------------------
	// test 4c: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilitiesSTA("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1, SymbTimeAnalysis::FREE_INPUTS);

	//-------------------------------------------
	// test 4d: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilitiesSTA("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0, SymbTimeAnalysis::FREE_INPUTS);
}


void TestFreeInputs::test4()
{
	aiger* circuit = Utils::readAiger("inputs/s5378.50percent.aag");
	CPPUNIT_ASSERT_MESSAGE("can not open inputs/s5378.50percent.aag", circuit != 0);
	Options::instance().setUnsatCoreInterval(0);
	vector<string> tc_files;
	tc_files.push_back("inputs/35_bit_input_2");

	SymbTimeAnalysis sta(circuit, 2, SymbTimeAnalysis::FREE_INPUTS);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sta.analyze(tcs);
	const set<unsigned> &vulnerabilities = sta.getVulnerableElements();

	// DEBUG: print the vulnerable latches
//		for (set<unsigned>::iterator it = vulnerabilities.begin();
//				it != vulnerabilities.end(); ++it)
//		{
//			cout << "  Latch " << *it << endl;
//		}
//		cout << "number of detected vulnerabilities: " << vulnerabilities.size() << endl;
	CPPUNIT_ASSERT(vulnerabilities.size() == 65);

	aiger_reset(circuit);
}

void TestFreeInputs::test5_sta()
{
	aiger* circuit = Utils::readAiger("inputs/protected_IWLS_2005_AIG_s208.aig");
	CPPUNIT_ASSERT_MESSAGE("can not open inputs/protected_IWLS_2005_AIG_s208.aig", circuit != 0);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<string> tc_files;
	vector<TestCase> tcs;

	tc_files.push_back("inputs/s208_concreteunsat.input");
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	SymbTimeAnalysis sta(circuit, 2, SymbTimeAnalysis::FREE_INPUTS);
	sta.analyze(tcs);
	CPPUNIT_ASSERT(sta.getVulnerableElements().size() == 0);

	tc_files.clear();
	tc_files.push_back("inputs/s208_concretesat.input");
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	SymbTimeAnalysis sta2(circuit, 2, SymbTimeAnalysis::FREE_INPUTS);
	sta2.analyze(tcs);
	CPPUNIT_ASSERT(sta2.getVulnerableElements().size() == 1);

	tc_files.clear();
	tc_files.push_back("inputs/s208_open.input");
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	SymbTimeAnalysis sta3(circuit, 2, SymbTimeAnalysis::FREE_INPUTS);
	sta3.analyze(tcs);
	CPPUNIT_ASSERT(sta3.getVulnerableElements().size() == 1);

	aiger_reset(circuit);
}

void TestFreeInputs::test6_stla()
{
	aiger* circuit = Utils::readAiger("inputs/protected_IWLS_2005_AIG_s208.aig");
	CPPUNIT_ASSERT_MESSAGE("can not open inputs/protected_IWLS_2005_AIG_s208.aig", circuit != 0);
	vector<string> tc_files;

	tc_files.push_back("inputs/s208_concreteunsat.input");
	SymbTimeLocationAnalysis sta(circuit, 2, SymbTimeLocationAnalysis::FREE_INPUTS);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sta.analyze(tcs);
	CPPUNIT_ASSERT(sta.getVulnerableElements().size() == 0);

	tc_files.clear();
	tc_files.push_back("inputs/s208_concretesat.input");
	SymbTimeLocationAnalysis sta2(circuit, 2, SymbTimeLocationAnalysis::FREE_INPUTS);
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sta2.analyze(tcs);
	CPPUNIT_ASSERT(sta2.getVulnerableElements().size() == 1);

	tc_files.clear();
	tc_files.push_back("inputs/s208_open.input");
	SymbTimeLocationAnalysis sta3(circuit, 2, SymbTimeLocationAnalysis::FREE_INPUTS);
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sta3.analyze(tcs);
	CPPUNIT_ASSERT(sta3.getVulnerableElements().size() == 1);

	aiger_reset(circuit);
}

void TestFreeInputs::test7_sim()
{
	vector<string> tc_files;
	tc_files.push_back("inputs/3b_w_free_inputs");

	//-------------------------------------------
	// test 4a: 3 of 3 latches protected
//	Logger::instance().enable(Logger::DBG);
	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilitiesSIM("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1, SimulationBasedAnalysis::FREE_INPUTS);
	//-------------------------------------------
	// test 4b: 2 of 3 latches protected

	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilitiesSIM("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1, SimulationBasedAnalysis::FREE_INPUTS);

	//-------------------------------------------
	// test 4c: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilitiesSIM("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1, SimulationBasedAnalysis::FREE_INPUTS);

	//-------------------------------------------
	// test 4d: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilitiesSIM("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0, SimulationBasedAnalysis::FREE_INPUTS);
}

void TestFreeInputs::test8_sim()
{
	aiger* circuit = Utils::readAiger("inputs/s5378.50percent.aag");
	CPPUNIT_ASSERT_MESSAGE("can not open inputs/s5378.50percent.aag", circuit != 0);
	Options::instance().setUnsatCoreInterval(0);
	vector<string> tc_files;
	tc_files.push_back("inputs/35_bit_input_2");

	SimulationBasedAnalysis sim(circuit, 2, SimulationBasedAnalysis::FREE_INPUTS);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sim.analyze(tcs);
	const set<unsigned> &vulnerabilities = sim.getVulnerableElements();

	// DEBUG: print the vulnerable latches
//		for (set<unsigned>::iterator it = vulnerabilities.begin();
//				it != vulnerabilities.end(); ++it)
//		{
//			cout << "  Latch " << *it << endl;
//		}
//	cout << "number of detected vulnerabilities: " << vulnerabilities.size() << endl;
	CPPUNIT_ASSERT(vulnerabilities.size() == 65);

	aiger_reset(circuit);
}

void TestFreeInputs::test9_sim()
{
	aiger* circuit = Utils::readAiger("inputs/protected_IWLS_2005_AIG_s208.aig");
	CPPUNIT_ASSERT_MESSAGE("can not open inputs/protected_IWLS_2005_AIG_s208.aig", circuit != 0);
	vector<string> tc_files;

	tc_files.push_back("inputs/s208_concreteunsat.input");
	SimulationBasedAnalysis sim(circuit, 2, SimulationBasedAnalysis::FREE_INPUTS);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sim.analyze(tcs);
	CPPUNIT_ASSERT(sim.getVulnerableElements().size() == 0);

	tc_files.clear();
	tc_files.push_back("inputs/s208_concretesat.input");
	SimulationBasedAnalysis sim2(circuit, 2, SimulationBasedAnalysis::FREE_INPUTS);
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sim2.analyze(tcs);
	CPPUNIT_ASSERT(sim2.getVulnerableElements().size() == 1);

	tc_files.clear();
	tc_files.push_back("inputs/s208_open.input");
	SimulationBasedAnalysis sim3(circuit, 2, SimulationBasedAnalysis::FREE_INPUTS);
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sim3.analyze(tcs);
	CPPUNIT_ASSERT(sim3.getVulnerableElements().size() == 1);

	aiger_reset(circuit);
}

void TestFreeInputs::test10_bdd()
{
	vector<string> tc_files;
	tc_files.push_back("inputs/3b_w_free_inputs");

	//-------------------------------------------
	// test 4a: 3 of 3 latches protected
//	Logger::instance().enable(Logger::DBG);
	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilitiesBDD("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1, SimulationBasedAnalysis::FREE_INPUTS); 	// TODO: change MODES!!
	//-------------------------------------------
	// test 4b: 2 of 3 latches protected

	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilitiesBDD("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1, SimulationBasedAnalysis::FREE_INPUTS);

	//-------------------------------------------
	// test 4c: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilitiesBDD("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1, SimulationBasedAnalysis::FREE_INPUTS);

	//-------------------------------------------
	// test 4d: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilitiesBDD("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0, SimulationBasedAnalysis::FREE_INPUTS);
}

void TestFreeInputs::test11_bdd()
{
	aiger* circuit = Utils::readAiger("inputs/s5378.50percent.aag");
	CPPUNIT_ASSERT_MESSAGE("can not open inputs/s5378.50percent.aag", circuit != 0);
	Options::instance().setUnsatCoreInterval(0);
	vector<string> tc_files;
	tc_files.push_back("inputs/35_bit_input_2");

	BddAnalysis bdd(circuit, 2, BddAnalysis::C_F_BINARY_FREE_INPUTS);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	bdd.analyze(tcs);
	const set<unsigned> &vulnerabilities = bdd.getVulnerableElements();

	// DEBUG: print the vulnerable latches
//		for (set<unsigned>::iterator it = vulnerabilities.begin();
//				it != vulnerabilities.end(); ++it)
//		{
//			cout << "  Latch " << *it << endl;
//		}
//	cout << "number of detected vulnerabilities: " << vulnerabilities.size() << endl;
	CPPUNIT_ASSERT(vulnerabilities.size() == 65);

	aiger_reset(circuit);
}

void TestFreeInputs::test12_bdd()
{
	aiger* circuit = Utils::readAiger("inputs/protected_IWLS_2005_AIG_s208.aig");
	TestCaseProvider::instance().setCircuit(circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open inputs/protected_IWLS_2005_AIG_s208.aig", circuit != 0);
	vector<string> tc_files;

	tc_files.push_back("inputs/s208_concreteunsat.input");
	BddAnalysis sim(circuit, 2, SimulationBasedAnalysis::FREE_INPUTS); 	     // TODO: change MODES!!

	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sim.analyze(tcs);
	CPPUNIT_ASSERT(sim.getVulnerableElements().size() == 0);

	tc_files.clear();
	tc_files.push_back("inputs/s208_concretesat.input");
	BddAnalysis sim2(circuit, 2, SimulationBasedAnalysis::FREE_INPUTS);
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sim2.analyze(tcs);
	CPPUNIT_ASSERT(sim2.getVulnerableElements().size() == 1);

	tc_files.clear();
	tc_files.push_back("inputs/s208_open.input");
	BddAnalysis sim3(circuit, 2, SimulationBasedAnalysis::FREE_INPUTS);
	tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sim3.analyze(tcs);
	CPPUNIT_ASSERT(sim3.getVulnerableElements().size() == 1);

	aiger_reset(circuit);
}
