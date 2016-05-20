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
#include "../src/TestCaseProvider.h"


extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestFalsePositives);

// -------------------------------------------------------------------------------------------
void TestFalsePositives::setUp()
{
	Logger::instance().disable(Logger::DBG);
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestFalsePositives::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestFalsePositives::test1_no_alarm()
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.protected.aag");
	FalsePositives falsepos(circuit,1);

	vector<TestCase> tc;
	Utils::generateRandomTestCases(tc,1,5,circuit->num_inputs);

	CPPUNIT_ASSERT(!falsepos.findFalsePositives_1b(tc));
	CPPUNIT_ASSERT(!falsepos.findFalsePositives_2b(tc));
	CPPUNIT_ASSERT(!falsepos.findFalsePositives_1b_free_inputs(tc));
	CPPUNIT_ASSERT(!falsepos.findFalsePositives_2b_free_inputs(tc));

	aiger_reset(circuit);
}

void TestFalsePositives::test2_alarm_without_error()
{

	aiger* circuit = Utils::readAiger("inputs/one_latch.fp.aag");
	FalsePositives falsepos(circuit,0);

	vector<TestCase> tc;
	Utils::generateRandomTestCases(tc,1,5,circuit->num_inputs);

	// Alarm should be raised without an error:
	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b(tc));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b(tc));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b_free_inputs(tc));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b_free_inputs(tc));

	aiger_reset(circuit);
}

void TestFalsePositives::test3()
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.fp2.aag");
	FalsePositives falsepos(circuit,0);


	// TC alarm values = 0,0,0
	vector<int> input_vector_false;
	input_vector_false.push_back(0);

	TestCase tc;
	tc.push_back(input_vector_false);
	tc.push_back(input_vector_false);
	tc.push_back(input_vector_false);

	vector<TestCase> tcs;
	tcs.push_back(tc);


	CPPUNIT_ASSERT(!falsepos.findFalsePositives_1b(tcs));
	CPPUNIT_ASSERT(!falsepos.findFalsePositives_2b(tcs));
	CPPUNIT_ASSERT(!falsepos.findFalsePositives_1b_free_inputs(tcs));
	CPPUNIT_ASSERT(!falsepos.findFalsePositives_2b_free_inputs(tcs));

	aiger_reset(circuit);
}




void TestFalsePositives::test4()
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.fp2.aag");
	FalsePositives falsepos(circuit,0);

	// TC alarm values = 0,0,1
	vector<int> input_vector_false;
	input_vector_false.push_back(0);
	vector<int> input_vector_true;
	input_vector_true.push_back(1);

	TestCase tc;
	tc.push_back(input_vector_false);
	tc.push_back(input_vector_false);
	tc.push_back(input_vector_true);

	vector<TestCase> tcs;
	tcs.push_back(tc);

	// Alarm should be raised without an error:
	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b(tcs));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b(tcs));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b_free_inputs(tcs));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b_free_inputs(tcs));

	aiger_reset(circuit);
}

void TestFalsePositives::test5_irrelevant_latches()
{
	aiger* circuit = Utils::readAiger("inputs/irrelevant_latches.prot.aag");
	FalsePositives falsepos(circuit,0);

	vector<TestCase> tcs;
		Utils::generateRandomTestCases(tcs,1,6,circuit->num_inputs);

	// Alarm should be raised
	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b(tcs));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b(tcs));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b_free_inputs(tcs));
	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b_free_inputs(tcs));
	aiger_reset(circuit);

}

void TestFalsePositives::test6_irrelevant_latches_delayOneTimestep()
{

	aiger* circuit = Utils::readAiger("inputs/irrelevant_latches.prot.delay1.aag");
	FalsePositives falsepos(circuit,0);

	vector<TestCase> tcs;
	Utils::generateRandomTestCases(tcs,1,6,circuit->num_inputs);

	// Alarm should be raised
	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b(tcs));
	checkIrrelevantLatchesDelayOneTraces(falsepos.getSuperfluous());
	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b(tcs));
	checkIrrelevantLatchesDelayOneTraces(falsepos.getSuperfluous());
	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b_free_inputs(tcs));
	checkIrrelevantLatchesDelayOneTraces(falsepos.getSuperfluous());
	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b_free_inputs(tcs));
	checkIrrelevantLatchesDelayOneTraces(falsepos.getSuperfluous());
	aiger_reset(circuit);

}

void TestFalsePositives::checkIrrelevantLatchesDelayOneTraces(vector<SuperfluousTrace*> sftrace)
{
	CPPUNIT_ASSERT(sftrace.size() == 25);

	for(vector<SuperfluousTrace*>::iterator it = sftrace.begin(); it != sftrace.end(); ++it)
	{

		SuperfluousTrace* trace = *it;

		if(trace->component_ < 42)
			CPPUNIT_ASSERT(trace->alarm_timestep_ == trace->flip_timestep_ + 1);
		else // latch 42 = alarm output. used to delay the alarm value for 1 timestep
			CPPUNIT_ASSERT(trace->alarm_timestep_ == trace->flip_timestep_);

		if(trace->component_ == 4) // 2 cycles necessary to let the (false) error go away for the first latch
		{
			CPPUNIT_ASSERT(trace->alarm_timestep_ + 2 == trace->error_gone_timestep_);
		}
		else // just one ts for the other latches
		{
			CPPUNIT_ASSERT(trace->alarm_timestep_ + 1 == trace->error_gone_timestep_);
		}
	}
}

// alarm can only be true if the input value is also true
void TestFalsePositives::test7_free_inputs()
{
	aiger* circuit = Utils::readAiger("inputs/irrelevant_latches.prot.delay1.useinput.aag");

	FalsePositives falsepos(circuit,0);

	// TC: 6 Timesteps with open input value
	vector<int> input_vector_open;
	input_vector_open.push_back(2);
	TestCase tc;
	tc.push_back(input_vector_open);
	tc.push_back(input_vector_open);
	tc.push_back(input_vector_open);
	tc.push_back(input_vector_open);
	tc.push_back(input_vector_open);
	tc.push_back(input_vector_open);
	vector<TestCase> tcs;
	tcs.push_back(tc);


	CPPUNIT_ASSERT(falsepos.findFalsePositives_1b_free_inputs(tcs));
	checkTest7Traces(falsepos.getSuperfluous());

	CPPUNIT_ASSERT(falsepos.findFalsePositives_2b_free_inputs(tcs));
	checkTest7Traces(falsepos.getSuperfluous());

	aiger_reset(circuit);
}

void TestFalsePositives::checkTest7Traces(vector<SuperfluousTrace*> sftrace)
{
	checkIrrelevantLatchesDelayOneTraces(sftrace); // basic circuit is equal to test6

	// the one and only input must be true whenever the alarm is raised
	for(vector<SuperfluousTrace*>::iterator it = sftrace.begin(); it != sftrace.end(); ++it)
	{
		SuperfluousTrace* trace = *it;
		unsigned alarm_ts = trace->alarm_timestep_;
		CPPUNIT_ASSERT(trace->testcase_[alarm_ts][0] == AIG_TRUE);
	}

}

void TestFalsePositives::test8_environment_input_model()
{
	aiger* circuit = Utils::readAiger("inputs/irrelevant_latches.prot.delay1.useinput.aag");
	aiger* environment = Utils::readAiger("inputs/env_buffer.aag");

	vector<unsigned> modes_to_test;
	modes_to_test.push_back(FalsePositives::SYMB_TIME_INPUTS);
	modes_to_test.push_back(FalsePositives::SYMB_TIME_LOCATION_INPUTS);

	for(unsigned i = 0; i < modes_to_test.size(); i++)
	{
		FalsePositives falsepos(circuit,modes_to_test[i]);
		falsepos.setEnvironmentModel(environment);

		// a) let the inputs value be selected by the SAT-solver for 3 timesteps
		TestCaseProvider::instance().setCircuit(circuit);
		vector<TestCase> tcs =  TestCaseProvider::instance().generateMcTestCase(3);
		falsepos.analyze(tcs);

		CPPUNIT_ASSERT(falsepos.getSuperfluous().size() > 0);

		falsepos.printResults();

		// b) the input is hardcoded to 0, which makes it
		//    irrelevant according to the environment
		//    model.

		InputVector invalid_input_vector;
		invalid_input_vector.push_back(AIG_FALSE); // this makes the CNF of the env UNSAT

		TestCase tc;
		tc.push_back(invalid_input_vector);
		tc.push_back(invalid_input_vector);
		tc.push_back(invalid_input_vector);

		vector<TestCase> testcases;
		testcases.push_back(tc);

		falsepos.analyze(testcases);

		CPPUNIT_ASSERT(falsepos.getSuperfluous().size() == 0);
	}




	aiger_reset(circuit);
	aiger_reset(environment);
}
