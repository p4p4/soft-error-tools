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

#include "TestEnvironmentModel.h"
#include "../src/Logger.h"
#include "../src/AigSimulator.h"
#include "../src/SimulationBasedAnalysis.h"
#include "../src/SymbTimeAnalysis.h"
#include "../src/SymbTimeLocationAnalysis.h"
#include "../src/Utils.h"
#include "../src/TestCaseProvider.h"

extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestEnvironmentModel);

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::setUp()
{
	//setup for testcases
//	Logger::instance().enable(Logger::ERR);
}

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::tearDown()
{
	//define here post processing steps
}

BackEnd* TestEnvironmentModel::getBackend(const std::string& backend_name, int mode,
		aiger* circuit)
{
	BackEnd* backend;
	if (backend_name == "sim")
	{
		backend = new SimulationBasedAnalysis(circuit, 0, mode);
	}
	else if (backend_name == "sta")
	{
		backend = new SymbTimeAnalysis(circuit, 0, mode);
	}
	else if (backend_name == "stla")
	{
		backend = new SymbTimeLocationAnalysis(circuit, 0, mode);
	}
	else
	{
		CPPUNIT_ASSERT_MESSAGE("wrong backend", false);
	}

	return backend;
}

void TestEnvironmentModel::basic_test_1(std::string backend_name, int mode)
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.unprotected.aag");
	aiger* environment = Utils::readAiger("inputs/env1.aag");

	TestCaseProvider::instance().setCircuit(circuit);
	vector<TestCase> tcs = TestCaseProvider::instance().generateRandomTestCases(1,5);
	if (backend_name == "sim")
	{
		SimulationBasedAnalysis* backend = new SimulationBasedAnalysis(circuit, 0, mode);
		backend->analyze(tcs);
		CPPUNIT_ASSERT(backend->getDetectedLatches().size() == 1);

		// in this test the output is always false, i.e. we don't care about the output
		backend->setEnvironmentModel(environment);
		backend->analyze(tcs);
		CPPUNIT_ASSERT(backend->getDetectedLatches().size() == 0);

		delete backend;
	}
	else if (backend_name == "sta")
	{
		SymbTimeAnalysis* backend = new SymbTimeAnalysis(circuit, 0, mode);
		backend->analyze(tcs);
		CPPUNIT_ASSERT(backend->getDetectedLatches().size() == 1);

		// in this test the output is always false, i.e. we don't care about the output
		backend->setEnvironmentModel(environment);
		backend->analyze(tcs);
		CPPUNIT_ASSERT(backend->getDetectedLatches().size() == 0);

		delete backend;
	}
	else if (backend_name == "stla")
	{
		SymbTimeLocationAnalysis* backend = new SymbTimeLocationAnalysis(circuit, 0, mode);
		backend->analyze(tcs);
		CPPUNIT_ASSERT(backend->getDetectedLatches().size() == 1);

		// in this test the output is always false, i.e. we don't care about the output
		backend->setEnvironmentModel(environment);
		backend->analyze(tcs);
		CPPUNIT_ASSERT(backend->getDetectedLatches().size() == 0);

		delete backend;
	}
	else
	{
		CPPUNIT_ASSERT_MESSAGE("wrong backend", false);
	}



	aiger_reset(circuit);
	aiger_reset(environment);
}

void TestEnvironmentModel::basic_test_2(std::string backend_name, int mode)
{
//	aiger* circuit = Utils::readAiger("inputs/1latch_1and.unprotected.aag");
//	aiger* environment;
//	BackEnd* backend = getBackend(backend_name, mode, circuit);
//	TestCase tc;
//	vector<TestCase> tcs;
//	vector<int> one;
//	one.push_back(1);
//	vector<int> zero;
//	zero.push_back(0);
//	// 2a) env = inverter, inputs = 1 1
//	environment = Utils::readAiger("inputs/inverter.unprot.aag");
//	backend->setEnvironmentModel(environment);
//	tc.push_back(one);
//	tc.push_back(one);
//	tcs.push_back(tc);
//	CPPUNIT_ASSERT(!backend->analyze(tcs));
//	// 2b) env = inverter, inputs = 1 1 0
//	tc.push_back(zero);
//	tcs.clear();
//	tcs.push_back(tc);
//	CPPUNIT_ASSERT(backend->analyze(tcs));
//	// 2c) env = buffer, inputs = 0 0
//	aiger_reset(environment);
//	environment = Utils::readAiger("inputs/buffer.unprot.aag");
//	backend->setEnvironmentModel(environment);
//	tc.clear();
//	tc.push_back(zero);
//	tc.push_back(zero);
//	tcs.clear();
//	tcs.push_back(tc);
//	CPPUNIT_ASSERT(!backend->analyze(tcs));
//	// 2d) env = buffer, inputs = 0 0 1
//	tc.push_back(one);
//	tcs.clear();
//	tcs.push_back(tc);
//	CPPUNIT_ASSERT(backend->analyze(tcs));
//
//	delete backend;
//	aiger_reset(circuit);
//	aiger_reset(environment);

	// TODO: repair TC
}

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::test1_sim_basic_1()
{
	basic_test_1("sim", 0);
}

void TestEnvironmentModel::test2_sim_basic_2()
{
	basic_test_2("sim", 0);
}

void TestEnvironmentModel::test3_sta0_basic_1()
{
	basic_test_1("sta", SymbTimeAnalysis::NAIVE);
}

void TestEnvironmentModel::test4_sta0_basic_2()
{
	basic_test_2("sta", SymbTimeAnalysis::NAIVE);
}

void TestEnvironmentModel::test5_sta1_basic_1()
{
	basic_test_1("sta", SymbTimeAnalysis::SYMBOLIC_SIMULATION);
}

void TestEnvironmentModel::test6_sta1_basic_2()
{
	basic_test_2("sta", SymbTimeAnalysis::SYMBOLIC_SIMULATION);
}

void TestEnvironmentModel::test7_sta2_basic_1()
{
	basic_test_1("sta", SymbTimeAnalysis::FREE_INPUTS);
}

void TestEnvironmentModel::test8_sta2_basic_2()
{
	basic_test_2("sta", SymbTimeAnalysis::FREE_INPUTS);
}

void TestEnvironmentModel::test9_stla0_basic_1()
{
	basic_test_1("stla", SymbTimeLocationAnalysis::STANDARD);
}

void TestEnvironmentModel::test10_stla0_basic_2()
{
	basic_test_2("stla", SymbTimeLocationAnalysis::STANDARD);
}

void TestEnvironmentModel::test11_stla1_basic_1()
{
	basic_test_1("stla", SymbTimeLocationAnalysis::FREE_INPUTS);
}

void TestEnvironmentModel::test12_stla1_basic_2()
{
	basic_test_2("stla", SymbTimeLocationAnalysis::FREE_INPUTS);
}

void TestEnvironmentModel::check_input_restrictions(std::string backend_name, int mode)
{
//	aiger* circuit = Utils::readAiger("inputs/toggle.3vulnerabilities.aag");
//	BackEnd* backend = getBackend(backend_name, mode, circuit);
//	aiger* environment = Utils::readAiger("inputs/env_3inputs_mustbe1.aag");
//	backend->setEnvironmentModel(environment);
//
//	// a) let all 3 inputs values be selected by the SAT-solver for 3 timesteps
//
//	TestCaseProvider::instance().setCircuit(circuit);
//	vector<TestCase> tcs =  TestCaseProvider::instance().generateMcTestCase(3);
//	backend->analyze(tcs);
//	CPPUNIT_ASSERT(backend->getVulnerableElements().size() > 0);
//
//
//	// b) let only 2 of 3 input values be selected by the MC
//	//    one input is hardcoded to 0, which makes all possible
//	//    input vectors to irrelevant according to the environment
//	//    model.
//
//	InputVector invalid_input_vector;
//	invalid_input_vector.push_back(LIT_FREE);
//	invalid_input_vector.push_back(LIT_FREE);
//	invalid_input_vector.push_back(AIG_FALSE); // this makes the CNF of the env UNSAT
//
//	TestCase tc;
//	tc.push_back(invalid_input_vector);
//	tc.push_back(invalid_input_vector);
//	tc.push_back(invalid_input_vector);
//
//	vector<TestCase> testcases;
//	testcases.push_back(tc);
//
//	backend->analyze(testcases);
//	CPPUNIT_ASSERT(backend->getVulnerableElements().size() == 0);
//
//	aiger_reset(circuit);
//	aiger_reset(environment);
//	delete backend;

	// TODO repair
}

void TestEnvironmentModel::test13_free_inputs()
{
	check_input_restrictions("sta", SymbTimeAnalysis::FREE_INPUTS);
	check_input_restrictions("stla", SymbTimeLocationAnalysis::FREE_INPUTS);

}

