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

#include "TestSimulationBasedAnalysis.h"
#include "../src/Logger.h"
#include "../src/TestCaseProvider.h"

extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION (TestSimulationBasedAnalysis);

// -------------------------------------------------------------------------------------------
void TestSimulationBasedAnalysis::setUp()
{
	//setup for testcases
	Logger::instance().disable(Logger::LOG);
}

// -------------------------------------------------------------------------------------------
void TestSimulationBasedAnalysis::tearDown()
{
	//define here post processing steps
}

// -------------------------------------------------------------------------------------------
aiger* TestSimulationBasedAnalysis::readAigerFile(string path)
{
	// read file:
	aiger* aig_input = aiger_init();
	const char *read_err = aiger_open_and_read_from_file(aig_input, path.c_str());

	if (read_err != NULL)
	{
		CPPUNIT_FAIL("Error: Could not open AIGER file");
	}

	return aig_input;
}

// -------------------------------------------------------------------------------------------
void TestSimulationBasedAnalysis::checkVulnerabilities(
		string path_to_aiger_circuit, vector<string> tc_files,
		set<unsigned> should_be_vulnerable, int num_err_latches)
{

	aiger* circuit = readAigerFile(path_to_aiger_circuit);
	SimulationBasedAnalysis sba(circuit, num_err_latches);
	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	sba.analyze(tcs);
	const set<unsigned> &vulnerabilities = sba.getVulnerableElements();

	// DEBUG: print the vulnerable latches
//	for (set<unsigned>::iterator it = vulnerabilities.begin();
//			it != vulnerabilities.end(); ++it)
//	{
//		cout << "  Latch " << *it << endl;
//	}

	CPPUNIT_ASSERT(vulnerabilities == should_be_vulnerable);
}

// -------------------------------------------------------------------------------------------
void TestSimulationBasedAnalysis::test1_simulation_analysis_w_1_extra_latch()
{

	// Paths to TestCase files
	// A TestCase file contains vectors of input values
	vector<string> tc_files;
	tc_files.push_back("inputs/3_bit_input_1");
	tc_files.push_back("inputs/3_bit_input_2");
	tc_files.push_back("inputs/3_bit_input_3");
	tc_files.push_back("inputs/3_bit_input_4");
	tc_files.push_back("inputs/3_bit_input_5");

	//-------------------------------------------
	// test 1a: 3 of 3 latches protected

	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilities("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1);

	//-------------------------------------------
	// test 1b: 2 of 3 latches protected

	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilities("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1);

	//-------------------------------------------
	// test 1c: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1);

	//-------------------------------------------
	// test 1d: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0);

}

void TestSimulationBasedAnalysis::test2_simulation_analysis_w_2_extra_latch()
{
	// Paths to TestCase files
	// A TestCase file contains vectors of input values
	vector<string> tc_files;
	tc_files.push_back("inputs/3_bit_input_1");
	tc_files.push_back("inputs/3_bit_input_2");
	tc_files.push_back("inputs/3_bit_input_3");
	tc_files.push_back("inputs/3_bit_input_4");
	tc_files.push_back("inputs/3_bit_input_5");

	//-------------------------------------------
	// test 1a: 3 of 3 latches protected

	set<unsigned> should_be_vulnerable;
	should_be_vulnerable.insert(10); // Latch 10 in vulnerable
	checkVulnerabilities("inputs/toggle2l.aag", tc_files, should_be_vulnerable,
			1);
}
