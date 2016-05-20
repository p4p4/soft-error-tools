// ----------------------------------------------------------------------------
// Copyright (c) 2015 by Graz University of Technology
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
//
// ----------------------------------------------------------------------------

#include "TestBdd.h"

extern "C" {
#include "aiger.h"
#include "cudd.h"
};
#include "cuddObj.hh"

#include "../src/Logger.h"
#include "../src/Utils.h"
#include "../src/BddAnalysis.h"
#include "../src/SimulationBasedAnalysis.h"
#include "../src/TestCaseProvider.h"

#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION(TestBdd);

// -------------------------------------------------------------------------------------------
void TestBdd::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestBdd::tearDown()
{
  //define here post processing steps
}

void TestBdd::getSatAss(const Cudd &cudd, const BDD &bdd, std::vector<int> &dest) const
{
    CPPUNIT_ASSERT(!bdd.IsZero());
    dest.clear();
    DdNode *currentNode = bdd.getNode();
    DdNode *constantOne = cudd.bddOne().getNode();
    DdNode *constantZero = cudd.bddZero().getNode();
    while (currentNode != constantOne) {
        CPPUNIT_ASSERT(currentNode != constantZero);
        int index = Cudd_NodeReadIndex(currentNode);
        if (Cudd_IsComplement(currentNode)) {
            if (Cudd_E(currentNode)==constantOne) {
                // Must not take else branch
                dest.push_back(index);
                currentNode = Cudd_Not(Cudd_T(currentNode));
            } else {
                dest.push_back(-index);
                currentNode = Cudd_Not(Cudd_E(currentNode));
            }
        } else {
            if (Cudd_E(currentNode)==constantZero) {
                // Must not take else branch
                dest.push_back(index);
                currentNode = Cudd_T(currentNode);
            } else {
                dest.push_back(-index);
                currentNode = Cudd_E(currentNode);
            }
        }
    }
}

// -------------------------------------------------------------------------------------------
void TestBdd::test1()
{
	Cudd cudd;
	cudd.AutodynEnable(CUDD_REORDER_SIFT);

	CPPUNIT_ASSERT(cudd.bddZero() == ~ cudd.bddOne());

	std::cout << std::endl;

	BDD foo = cudd.bddVar(2);
	BDD bar = cudd.bddVar(3);

	BDD res = foo & bar;
	std::vector<int> sat_ass;
	getSatAss(cudd, res, sat_ass);
	std::cout << "Result of getSatAss" << std::endl;
	for (size_t i = 0; i < sat_ass.size(); ++i)
		std::cout << sat_ass[i] << ", ";
	std::cout << std::endl;

	Utils::debugPrint(sat_ass, "sat ass: ");
	char sat_ass_string[4];
	res.PickOneCube(sat_ass_string);
	// prints a value vor bddVar 0,1,2,3,4,...
	// value 2 means: irrelevant
	// value 1 means: true
	// value 0 means: false
	std::cout << "Result of PickOneCube (2 means: irrelevant)" << std::endl;
	for (size_t i = 0; i < 4; ++i)
		std::cout << "  var " << i << " has value " << static_cast<int>(sat_ass_string[i])
				<< std::endl;

	res = foo & bar;
	res.PrintMinterm();
	CPPUNIT_ASSERT(!res.IsZero());

	res = foo & ~bar;
	res.PrintMinterm();
	CPPUNIT_ASSERT(!res.IsZero());

	res = ~foo & bar;
	res.PrintMinterm();
	CPPUNIT_ASSERT(!res.IsZero());

	res = ~foo & ~bar;
	res.PrintMinterm();
	CPPUNIT_ASSERT(!res.IsZero());

	//-------------------
	DdNode* node = res.getNode();
	unsigned index = Cudd_NodeReadIndex(node);
	std::cout << "index " << index << std::endl;

	DdNode* thenNode = Cudd_T(node);
	std::cout << "index then " << Cudd_NodeReadIndex(thenNode) << std::endl;
	DdNode* elseNode = Cudd_E(node);
	std::cout << "index else " << Cudd_NodeReadIndex(elseNode) << std::endl;


	res = foo & ~foo;
	res.PrintMinterm();
	res.PrintCover();

	CPPUNIT_ASSERT(res.IsZero());


	// what is the equivalent to satsolver.isSat() for BDDs?
	// how to get a satisfying assignment?


}

// -------------------------------------------------------------------------------------------
void TestBdd::test2_generate_cj_BDDs()
{
	Cudd cudd;

	int num_of_cj_signals = 4;
	map<int, int> latch_to_cj; // dummy cj literals

	for(int i = 0; i < num_of_cj_signals; i++)
		latch_to_cj[i] = i;


	map<int, int>::iterator map_iter;
	map<int, int>::iterator map_iter2;

	map<int, BDD> cj_to_BDD_signal;

	// --------------------------------------------------------------------------------------------------
	// generate 1 hot encoding:
	for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++) // for each latch:
	{
		BDD real_c_signal = cudd.bddOne();
		for (map_iter2 = latch_to_cj.begin(); map_iter2 != latch_to_cj.end(); map_iter2++) // for current latch, go over all latches
		{
			if (map_iter == map_iter2) // the one and only cj-signal which can be true for this signal
				real_c_signal = real_c_signal & cudd.bddVar(map_iter2->second);
			else
				real_c_signal = real_c_signal & ~cudd.bddVar(map_iter2->second);
		}
		cj_to_BDD_signal[map_iter->second] = real_c_signal;
	}


	// --------------------------------------------------------------------------------------------------
	// test 1 hot encoding for correctness
	char sat_ass_string[num_of_cj_signals];
	for(unsigned i = 0; i < num_of_cj_signals; i++)
	{
		cj_to_BDD_signal[i].PickOneCube(sat_ass_string);
		for (unsigned j = 0; j < num_of_cj_signals; j++)
		{
			std::cout << "c[" << i <<"]  var " << j << " has value " << static_cast<int>(sat_ass_string[j]) << std::endl;
			CPPUNIT_ASSERT(i == j ? sat_ass_string[j] == 1 : sat_ass_string[j] == 0);
		}

	}

	CPPUNIT_ASSERT(cudd.ReadSize() == 4);


}


// -------------------------------------------------------------------------------------------
void TestBdd::test3_modify_BDD_signal()
{
	Cudd cudd;

	// input signals
	BDD in_0 = cudd.bddVar(0);
	BDD in_1 = cudd.bddVar(1);
	BDD in_2 = cudd.bddVar(2);

	// output = in_0 & in_1 & in_2
	BDD intermediate_result = in_0 & in_1;
	BDD output = intermediate_result & in_2;

	// check assignment
	//char sat_ass_string[3];
	char* sat_ass_string = (char*) malloc(3);


	output.PickOneCube(sat_ass_string);
	for (size_t j = 0; j < 3; j++)
	{
		std::cout << "var " << j << " has value " << static_cast<int>(sat_ass_string[j]) << std::endl;
		CPPUNIT_ASSERT(sat_ass_string[j] == 1);
	}

	// modify intermediate result
	intermediate_result = in_0 & ~in_1;
	output = intermediate_result & in_2;

	/*  // or this way?
	BDD new_intermediate_result = in_0 & ~in_1;
	vector<BDD> oldVar, newVar;
	oldVar.push_back(intermediate_result);
	newVar.push_back(new_intermediate_result);
	output.SwapVariables(oldVar,newVar);
	*/


	// check SAT assignment again:
	output.PickOneCube(sat_ass_string);
	for (size_t j = 0; j < 3; j++)
	{
		std::cout << "var " << j << " has value " << static_cast<int>(sat_ass_string[j]) << std::endl;
	}
	CPPUNIT_ASSERT(sat_ass_string[1] == 0);


}

void TestBdd::test4_analysis_basic()
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.protected.aag");
	BddAnalysis bddAnalysis(circuit, 1, 0); // TODO change/add mode

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().generateRandomTestCases(1,3);
	bddAnalysis.analyze(tcs);

	CPPUNIT_ASSERT(bddAnalysis.getVulnerableElements().size() == 0);
	aiger_reset(circuit);

	aiger* circuit2 = Utils::readAiger("inputs/one_latch.unprotected.aag");
	BddAnalysis bddAnalysis2(circuit2, 0, 0);

	TestCaseProvider::instance().setCircuit(circuit2);
	vector<TestCase> tcs2 = TestCaseProvider::instance().generateRandomTestCases(1,3);
	bddAnalysis2.analyze(tcs2);

	CPPUNIT_ASSERT(bddAnalysis2.getVulnerableElements().size() == 1);
	aiger_reset(circuit2);
}

void TestBdd::test5_analysis_3latches()
{
	aiger* circuit = Utils::readAiger("inputs/two_latches.protected.aag");
	BddAnalysis bddAnalysis(circuit, 1, 0); // TODO Mode

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().generateRandomTestCases(1,2);
	bddAnalysis.analyze(tcs);

	//	L_DBG("VULNERABLE size = " << bddAnalysis.getVulnerableElements().size());
	CPPUNIT_ASSERT(bddAnalysis.getVulnerableElements().size() == 0);
	aiger_reset(circuit);

	aiger* circuit2 = Utils::readAiger("inputs/two_latches.unprotected.aag");
	BddAnalysis bddAnalysis2(circuit2, 0, 0);

	TestCaseProvider::instance().setCircuit(circuit2);
	vector<TestCase> tcs2 = TestCaseProvider::instance().generateRandomTestCases(1,3);
	bddAnalysis2.analyze(tcs2);

	CPPUNIT_ASSERT(bddAnalysis2.getVulnerableElements().size() == 2);
	aiger_reset(circuit2);
}

// -------------------------------------------------------------------------------------------
void TestBdd::checkVulnerabilities(string path_to_aiger_circuit,
		vector<string> tc_files, set<unsigned> should_be_vulnerable,
		int num_err_latches, int mode)
{

	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open " + path_to_aiger_circuit, circuit != 0);

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().readTestcasesFromFiles(tc_files);
	BddAnalysis bddAnalysis(circuit, num_err_latches, mode);
	bddAnalysis.analyze(tcs);
	const set<unsigned> &vulnerabilities = bddAnalysis.getVulnerableElements();

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

void TestBdd::compareWithSimulation(string path_to_aiger_circuit,
		int num_tc, int num_timesteps, int num_err_latches, int mode)
{
	aiger* circuit = Utils::readAiger(path_to_aiger_circuit);
	CPPUNIT_ASSERT_MESSAGE("can not open " + path_to_aiger_circuit, circuit != 0);

	srand(0xCAFECAFE);
	BddAnalysis bddAnalysis(circuit, num_err_latches, mode);

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().generateRandomTestCases(num_tc,num_timesteps);
	bddAnalysis.analyze(tcs);
	const set<unsigned> &symb_vulnerabilities = bddAnalysis.getVulnerableElements();

	SimulationBasedAnalysis sba(circuit, num_err_latches);
	sba.analyze(tcs);
	const set<unsigned> &sim_vulnerabilities = sba.getVulnerableElements();
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

	std::set<int> result;
	std::set_difference(sim_vulnerabilities.begin(), sim_vulnerabilities.end(), symb_vulnerabilities.begin(), symb_vulnerabilities.end(),
	    std::inserter(result, result.end()));
	std::cout << "result size:" << result.size() << std::endl;
	std::set<int>::iterator it;
	for (it = result.begin(); it != result.end(); ++it)
	{
		std::cout << "     VULNERABILITY latch DIFF: " << *it << std::endl;
	}

	std::cout << "sim: "<< sim_vulnerabilities.size() << " vs symb: " << symb_vulnerabilities.size() << std::endl;
	CPPUNIT_ASSERT(sim_vulnerabilities.size() == symb_vulnerabilities.size());
	CPPUNIT_ASSERT_MESSAGE(path_to_aiger_circuit,
			sim_vulnerabilities == symb_vulnerabilities);
}

void TestBdd::test6_analysis_w_1_extra_latch()
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
//	Logger::instance().enable(Logger::DBG);
	set<unsigned> should_be_vulnerable; // empty
	checkVulnerabilities("inputs/toggle.perfect.aag", tc_files,
			should_be_vulnerable, 1, 0);
//	Logger::instance().disable(Logger::DBG);
	//-------------------------------------------
	// test 4b: 2 of 3 latches protected

	should_be_vulnerable.insert(10); // 10 is vulnerable
	checkVulnerabilities("inputs/toggle.1vulnerability.aag", tc_files,
			should_be_vulnerable, 1, 0);

	//-------------------------------------------
	// test 4c: 1 of 3 latches protected
	should_be_vulnerable.insert(12); // 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.2vulnerabilities.aag", tc_files,
			should_be_vulnerable, 1, 0);

	//-------------------------------------------
	// test 4d: 0 of 3 latches protected
	should_be_vulnerable.insert(8); // 8, 10, 12 are vulnerable
	checkVulnerabilities("inputs/toggle.3vulnerabilities.aag", tc_files,
			should_be_vulnerable, 0, 0);
}

void TestBdd::test7_analysis_compare_with_simulation_1()
{

	compareWithSimulation("inputs/toggle.perfect.aag", 1, 2, 1,
			0);
	compareWithSimulation("inputs/toggle.1vulnerability.aag", 1, 2, 1,
			0);
	compareWithSimulation("inputs/toggle.2vulnerabilities.aag", 1, 2, 1,
			0);
//	Logger::instance().enable(Logger::INF);
	Logger::instance().disable(Logger::INF);
	compareWithSimulation("inputs/toggle.3vulnerabilities.aag", 1, 2, 0,
			0);
	compareWithSimulation("inputs/iwls02texasa.2vul.1l.aag", 5, 5, 1,
			0); // TODO: 0 vulnerabilities?
	compareWithSimulation("inputs/ex5.2vul.1l.aig", 5, 5, 1,
			0);
	compareWithSimulation("inputs/ex5.2vul.2l.aig", 5, 5, 2,
			0);
	compareWithSimulation("inputs/beecount-synth.2vul.1l.aig", 2, 5, 1,
			0);
	compareWithSimulation("inputs/s27.1vul.1l", 2, 1, 1,
			0);
	compareWithSimulation("inputs/shiftreg.2vul.1l.aig", 1, 2, 1,
			0);
	compareWithSimulation("inputs/traffic-synth.5vul.1l.aig", 5, 14, 1,
			0);
//	Logger::instance().enable(Logger::DBG);
	compareWithSimulation("inputs/s5378.50percent.aag", 3, 5, 2,
			0); // 164 Latches!
}
