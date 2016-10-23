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

#ifndef CPP_UNIT_TestAigSimulator_H__
#define CPP_UNIT_TestAigSimulator_H__


#include <vector>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "../src/AigSimulator.h"

struct aiger;

// -------------------------------------------------------------------------------------------
///
/// @class TestAigSimulator
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class TestAigSimulator : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(TestAigSimulator);
  CPPUNIT_TEST(test1_sim_combinatoric_circuit);
  CPPUNIT_TEST(test2_sim_combinatoric_circuit_with_aigsim_inputfile);
  CPPUNIT_TEST(test3_circuit_with_latches_compare_w_aigersim_outputfile);
  CPPUNIT_TEST(test4_reuse_aigsim_object);
  CPPUNIT_TEST(test5_simulate_with_provided_state);
  CPPUNIT_TEST(test6);
  CPPUNIT_TEST(test7);
  CPPUNIT_TEST_SUITE_END();

public:

// -------------------------------------------------------------------------------------------
///
/// @brief Initializes the object under test.
  void setUp();

// -------------------------------------------------------------------------------------------
///
/// @brief Shuts down the object under test.
  void tearDown();

  aiger* readAigerFile(string path);

  void compareOutputVector(int* c_array, std::vector<int> c_vector);

  void AigSimDiff(aiger* circuit, string aigsim_input_file, string aigsim_output_file);
  void AigSimDiff(AigSimulator& sim, string aigsim_input_file, string aigsim_output_file);


protected:

// -------------------------------------------------------------------------------------------
///
/// @brief Simulates a combinatoric circuit (5 inputs, 2 outputs, no latches)
///
/// uses all possible 32 input vectors and compares the results with expected results.
/// benchmark: IWLS_2002_AIG/LGSynth89/C17_orig.aig
///
  void test1_sim_combinatoric_circuit();

// -------------------------------------------------------------------------------------------
///
/// @brief Same as test1, but reads inputs from aigsim file
///
/// Simulates a combinatoric circuit (5 inputs, 2 outputs, no latches)
/// uses all possible 32 input vectors and compares the results with expected results.
/// benchmark: IWLS_2002_AIG/LGSynth89/C17_orig.aig
///
  void test2_sim_combinatoric_circuit_with_aigsim_inputfile();

  void test3_circuit_with_latches_compare_w_aigersim_outputfile();

  void test4_reuse_aigsim_object();

  void test5_simulate_with_provided_state();

  void test6();

// -------------------------------------------------------------------------------------------
///
/// @brief latches as output bug
///
/// the aigsim tool from the AIGER library has a bug with the tested circuit.
/// This test executes the circuit-simulation with our own AigSimulator class, which
/// should not have this bug.
  void test7();

};

#endif // CPP_UNIT_TestAigSimulator_H__
