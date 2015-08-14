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
#include "../src/Utils.h"
#include "../src/Logger.h"

extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestSymbTimeAnalysis);

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::checkVulnerabilities(
		string path_to_aiger_circuit, vector<string> tc_files,
		set<unsigned> should_be_vulnerable, int num_err_latches)
{


	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);

	CPPUNIT_ASSERT(circuit != 0);
	SymbTimeAnalysis sta(circuit, num_err_latches);
	sta.findVulnerabilities(tc_files);
	const set<unsigned> &vulnerabilities = sta.getVulnerableElements();

	// DEBUG: print the vulnerable latches
//	for (set<unsigned>::iterator it = vulnerabilities.begin();
//			it != vulnerabilities.end(); ++it)
//	{
//		cout << "  Latch " << *it << endl;
//	}

	CPPUNIT_ASSERT(vulnerabilities == should_be_vulnerable);
}




void TestSymbTimeAnalysis::test1_one_latch()
{

//	Logger::instance().enable(Logger::DBG);

	aiger* circuit = Utils::readAiger("inputs/one_latch.protected.aag");
	SymbTimeAnalysis sta(circuit, 1);
	sta.findVulnerabilities(1, 3);
	CPPUNIT_ASSERT(sta.getVulnerableElements().size() == 0);
	aiger_reset(circuit);

	aiger* circuit2 = Utils::readAiger("inputs/one_latch.unprotected.aag");
	SymbTimeAnalysis sta2(circuit2,0);
	sta2.findVulnerabilities(1, 3);
	CPPUNIT_ASSERT(sta2.getVulnerableElements().size() == 1);
	aiger_reset(circuit2);

}

void TestSymbTimeAnalysis::test2_one_latch_one_and()
{
//		Logger::instance().enable(Logger::DBG);

		aiger* circuit = Utils::readAiger("inputs/1latch_1and.protected.aag");
		SymbTimeAnalysis sta(circuit, 1);
		sta.findVulnerabilities(1, 3);
		CPPUNIT_ASSERT(sta.getVulnerableElements().size() == 0);
		aiger_reset(circuit);

//		Logger::instance().disable(Logger::DBG);
//		Logger::instance().enable(Logger::DBG);


		aiger* circuit2 = Utils::readAiger("inputs/1latch_1and.unprotected.aag");
		SymbTimeAnalysis sta2(circuit2,0);
		sta2.findVulnerabilities(1, 3);
		CPPUNIT_ASSERT(sta2.getVulnerableElements().size() == 1);
		aiger_reset(circuit2);
}

void TestSymbTimeAnalysis::test3_two_latches()
{
//		Logger::instance().enable(Logger::DBG);

		aiger* circuit = Utils::readAiger("inputs/two_latches.protected.aag");
		SymbTimeAnalysis sta(circuit, 1);
		sta.findVulnerabilities(1, 2);
		L_DBG("VULNERABLE size = " << sta.getVulnerableElements().size());
		CPPUNIT_ASSERT(sta.getVulnerableElements().size() == 0);
		aiger_reset(circuit);

		aiger* circuit2 = Utils::readAiger("inputs/two_latches.unprotected.aag");
		SymbTimeAnalysis sta2(circuit2,0);
		sta2.findVulnerabilities(1, 3);
		CPPUNIT_ASSERT(sta2.getVulnerableElements().size() == 2);
		aiger_reset(circuit2);
}

// -------------------------------------------------------------------------------------------
void TestSymbTimeAnalysis::test4_analysis_w_1_extra_latch()
{

	// Paths to TestCase files
	// A TestCase file contains vectors of input values
	vector<string> tc_files;
	tc_files.push_back("inputs/3b");
	tc_files.push_back("inputs/3_bit_input_1");
	tc_files.push_back("inputs/3_bit_input_2");
	tc_files.push_back("inputs/3_bit_input_3");
	tc_files.push_back("inputs/3_bit_input_4");
	tc_files.push_back("inputs/3_bit_input_5");

	//-------------------------------------------
	// test 4a: 3 of 3 latches protected

	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilities("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1);

	//-------------------------------------------
	// test 4b: 2 of 3 latches protected

	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilities("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1);

	//-------------------------------------------
	// test 4c: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1);

	//-------------------------------------------
	// test 4d: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0);

}

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

	//-------------------------------------------
	// test 1a: 3 of 3 latches protected

	set<unsigned> should_be_vulnerable;
	should_be_vulnerable.insert(10); // Latch 10 in vulnerable
	checkVulnerabilities("inputs/toggle2l.aag", tc_files, should_be_vulnerable,
			1);
}

void TestSymbTimeAnalysis::test6_analysis_w_random_inputs()
{
	// use constant seed, so that we don't have any non-determinism in the test-case ;-)
	srand(0xCAFECAFE);

	aiger* circuit = Utils::readAiger("inputs/toggle.2vulnerabilities.aag");
	SymbTimeAnalysis sta(circuit, 1);
	sta.findVulnerabilities(1, 5);
	const set<unsigned> &vulnerabilities = sta.getVulnerableElements();

	// DEBUG: print the vulnerable latches
//	cout << endl;
//		for (set<unsigned>::iterator it = vulnerabilities.begin();
//				it != vulnerabilities.end(); ++it)
//		{
//			cout << "  Latch " << *it << endl;
//		}

	set<unsigned> should_be_vulnerable; // empty
	should_be_vulnerable.insert(10);
	should_be_vulnerable.insert(12);
	CPPUNIT_ASSERT(vulnerabilities == should_be_vulnerable);
}

void TestSymbTimeAnalysis::test7_analysis_big_w_random_inputs()
{
	// use constant seed, so that we don't have any non-determinism in the test-case ;-)
	srand(0xCAFECAFE);

	// Test 4a:
	//	Source: IWLS_2002_AIG/LGSynth91/smlexamples/s5378_orig.aig:
	//	Latches: 164, Protected with AddParityTool: 50% (=82 Latches)
	//	Vulnerabilities: 82 Latches

	aiger* circuit = Utils::readAiger("inputs/s5378.50percent.aag");
	SymbTimeAnalysis sta(circuit, 2);
	sta.findVulnerabilities(12, 10); // 1 TC with 2 timesteps would also already work
	const set<unsigned> &vulnerabilities = sta.getVulnerableElements();

	cout <<  endl << "vulnerabilities found: " <<  vulnerabilities.size() << endl;
	CPPUNIT_ASSERT(vulnerabilities.size() == 82);


}
