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
#include "../src/Utils.h"

extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestEnvironmentModel);

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::setUp()
{
	//setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::tearDown()
{
	//define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::test1_sim_basic_1()
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.unprotected.aag");
	aiger* environment = Utils::readAiger("inputs/one_latch.unprotected.aag");

	SimulationBasedAnalysis sba(circuit, 0);
	sba.findVulnerabilities(1, 5); // first without environment
	CPPUNIT_ASSERT(sba.getVulnerableElements().size() == 1);

	// in this test the output is always false, i.e. we don't care about the output
	sba.setEnvironmentModel(environment);
	sba.findVulnerabilities(1, 5);
	CPPUNIT_ASSERT(sba.getVulnerableElements().size() == 0);
}

void TestEnvironmentModel::test2_sim_basic_2()
{
	aiger* circuit = Utils::readAiger("inputs/1latch_1and.unprotected.aag");
	aiger* environment;
	SimulationBasedAnalysis sba(circuit, 0);

	TestCase tc;
	vector<TestCase> tcs;


	vector<int> one;
	one.push_back(1);

	vector<int> zero;
	zero.push_back(0);

	// 2a) env = inverter, inputs = 1 1
	environment = Utils::readAiger("inputs/inverter.unprot.aag");
	sba.setEnvironmentModel(environment);
	tc.push_back(one);
	tc.push_back(one);
	tcs.push_back(tc);
	CPPUNIT_ASSERT(!sba.findVulnerabilities(tcs));

	// 2b) env = inverter, inputs = 1 1 0
	tc.push_back(zero);
	tcs.clear();
	tcs.push_back(tc);
	CPPUNIT_ASSERT(sba.findVulnerabilities(tcs));

	// 2c) env = buffer, inputs = 0 0
	environment = Utils::readAiger("inputs/buffer.unprot.aag");
	sba.setEnvironmentModel(environment);
	tc.clear();
	tc.push_back(zero);
	tc.push_back(zero);
	tcs.clear();
	tcs.push_back(tc);
	CPPUNIT_ASSERT(!sba.findVulnerabilities(tcs));

	// 2d) env = buffer, inputs = 0 0 1
	tc.push_back(one);
	tcs.clear();
	tcs.push_back(tc);
	CPPUNIT_ASSERT(sba.findVulnerabilities(tcs));
}
