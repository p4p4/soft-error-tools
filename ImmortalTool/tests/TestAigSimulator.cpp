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

#include <vector>

#include "TestAigSimulator.h"
#include "../src/AigSimulator.h"

extern "C"
{
#include "aiger.h"
}

using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION (TestAigSimulator);

// -------------------------------------------------------------------------------------------
void TestAigSimulator::setUp()
{
	//setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestAigSimulator::tearDown()
{
	//define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestAigSimulator::test1_sim_combinatoric_circuit()
{

	// read file:
	aiger* aig_input = readAigerFile("inputs/C17_orig.aig");
	AigSimulator sim(aig_input);

	int expected_results[32][2] =
	{
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 1, 0 },
	{ 1, 0 },
	{ 0, 1 },
	{ 0, 1 },
	{ 0, 1 },
	{ 0, 1 },
	{ 0, 1 },
	{ 0, 0 },
	{ 1, 1 },
	{ 1, 0 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 0, 0 },
	{ 1, 1 },
	{ 1, 0 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 0, 0 },
	{ 1, 1 },
	{ 1, 0 } };

	vector<int> inputs(5);
	for (unsigned int i = 0; i < 32; i++)
	{
		inputs[4] = i & 1;
		inputs[3] = (i & 2) >> 1;
		inputs[2] = (i & 4) >> 2;
		inputs[1] = (i & 8) >> 3;
		inputs[0] = (i & 16) >> 4;

		sim.simulateOneTimeStep(inputs);

		// check outputs:
		vector<int> outputs = sim.getOutputs();

		for (unsigned int out_ctr = 0; out_ctr < outputs.size(); out_ctr++)
		{
			CPPUNIT_ASSERT(outputs[out_ctr] == expected_results[i][out_ctr]);
		}
	}

}

aiger* TestAigSimulator::readAigerFile(char* path)
{
	// read file:
	aiger* aig_input = aiger_init();
	const char *read_err = aiger_open_and_read_from_file(aig_input, path);

	if (read_err != NULL)
	{
		CPPUNIT_FAIL("Error: Could not open AIGER file");
	}

	return aig_input;
}

void TestAigSimulator::compareOutputVector(int* c_array,
		std::vector<int> c_vector)
{
	for (unsigned int out_ctr = 0; out_ctr < c_vector.size(); out_ctr++)
	{
		CPPUNIT_ASSERT(c_vector[out_ctr] == c_array[out_ctr]);
	}
}

void TestAigSimulator::test2_sim_combinatoric_circuit_with_aigsim_inputfile()
{
	// read circuit:
	aiger* aig_input = readAigerFile("inputs/C17_orig.aig");
	AigSimulator sim(aig_input);

	// read testcase-file
	sim.setTestcase("inputs/C17_orig.aigsiminput");

	int expected_results[32][2] =
	{
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 },
	{ 1, 0 },
	{ 1, 0 },
	{ 0, 1 },
	{ 0, 1 },
	{ 0, 1 },
	{ 0, 1 },
	{ 0, 1 },
	{ 0, 0 },
	{ 1, 1 },
	{ 1, 0 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 0, 0 },
	{ 1, 1 },
	{ 1, 0 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 1, 1 },
	{ 0, 0 },
	{ 1, 1 },
	{ 1, 0 } };

	for (unsigned int i = 0; i < 32; i++)
	{
		bool success = sim.simulateOneTimeStep();
		CPPUNIT_ASSERT(success == true);

		// check outputs:
		vector<int> outputs = sim.getOutputs();

		for (unsigned int out_ctr = 0; out_ctr < outputs.size(); out_ctr++)
		{
			CPPUNIT_ASSERT(outputs[out_ctr] == expected_results[i][out_ctr]);
		}
	}


	// there are only 32 input-vectors, after that, simulateOneTimeStep() must return false
	bool success = sim.simulateOneTimeStep();
	CPPUNIT_ASSERT(success == false);


}
