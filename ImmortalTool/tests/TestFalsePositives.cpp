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

#include "TestFalsePositives.h"
#include "../src/FalsePositives.h"
#include "../src/Utils.h"
#include "../src/Logger.h"
#include "../src/defines.h"

extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestFalsePositives);

// -------------------------------------------------------------------------------------------
void TestFalsePositives::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestFalsePositives::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestFalsePositives::test1()
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.protected.aag");
	FalsePositives falsepos(circuit,1);

	vector<TestCase> tc;
	Utils::generateRandomTestCases(tc,1,5,circuit->num_inputs);

	CPPUNIT_ASSERT(!falsepos.findFalsePositives_1b(tc));

//	sta.findVulnerabilities(1, 3);
//	CPPUNIT_ASSERT(sta.getVulnerableElements().size() == 0);
//	aiger_reset(circuit);
}

void TestFalsePositives::test2()
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.fp.aag");
	FalsePositives falsepos(circuit,1);

	vector<TestCase> tc;
	Utils::generateRandomTestCases(tc,1,5,circuit->num_inputs);

	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b(tc));
}
