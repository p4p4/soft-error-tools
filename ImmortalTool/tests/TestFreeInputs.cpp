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
#include "../src/SymbTimeAnalysis.h"
#include "../src/SymbTimeLocationAnalysis.h"
#include "../src/SimulationBasedAnalysis.h" // for comparison
#include "../src/Utils.h"
#include "../src/Options.h"
#include "../src/Logger.h"

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

	SymbTimeLocationAnalysis sta(circuit, num_err_latches, mode);
	sta.findVulnerabilities(tc_files);
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

	SymbTimeAnalysis sta(circuit, num_err_latches, mode);
	sta.findVulnerabilities(tc_files);
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
		uint64_t foo = 123;
		Options::instance().setUnsatCoreInterval(0);
		vector<string> tc_files;
		tc_files.push_back("inputs/35_bit_input_1");

		SymbTimeLocationAnalysis sta(circuit, 2, SymbTimeLocationAnalysis::FREE_INPUTS);
		sta.findVulnerabilities(tc_files);
		const set<unsigned> &vulnerabilities = sta.getVulnerableElements();

		// DEBUG: print the vulnerable latches
		for (set<unsigned>::iterator it = vulnerabilities.begin();
				it != vulnerabilities.end(); ++it)
		{
			cout << "  Latch " << *it << endl;
		}
		cout << "number of detected vulnerabilities: " << vulnerabilities.size() << endl;

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
		uint64_t foo = 123;
		Options::instance().setUnsatCoreInterval(0);
		vector<string> tc_files;
		tc_files.push_back("inputs/35_bit_input_1");

		SymbTimeAnalysis sta(circuit, 2, SymbTimeAnalysis::FREE_INPUTS);
		sta.findVulnerabilities(tc_files);
		const set<unsigned> &vulnerabilities = sta.getVulnerableElements();

		// DEBUG: print the vulnerable latches
		for (set<unsigned>::iterator it = vulnerabilities.begin();
				it != vulnerabilities.end(); ++it)
		{
			cout << "  Latch " << *it << endl;
		}
		cout << "number of detected vulnerabilities: " << vulnerabilities.size() << endl;

		aiger_reset(circuit);
}
