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

#ifndef CPP_UNIT_TestSimulationBasedAnalysis_H__
#define CPP_UNIT_TestSimulationBasedAnalysis_H__


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "../src/AigSimulator.h"
#include "../src/SimulationBasedAnalysis.h"

struct aiger;


// -------------------------------------------------------------------------------------------
///
/// @class TestSimulationBasedAnalysis
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class TestSimulationBasedAnalysis : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(TestSimulationBasedAnalysis);
  CPPUNIT_TEST(test1_simulation_analysis_w_1_extra_latch);
  CPPUNIT_TEST(test2_simulation_analysis_w_2_extra_latch);
  CPPUNIT_TEST_SUITE_END();

public:

// -------------------------------------------------------------------------------------------
///
/// @brief Initializes the object under test.
  void setUp();

// -------------------------------F------------------------------------------------------------
///
/// @brief Shuts down the object under test.
  void tearDown();

  aiger* readAigerFile(char* path);

  void checkVulnerabilities(char* path_to_aiger_circuit, vector<char*> tc_files, set<unsigned> should_be_vulnerable, int num_err_latches);

protected:

// -------------------------------------------------------------------------------------------
///
/// @brief Tests the found vulnerabilities of a circuit, which is protected with 1 extra latch
  void test1_simulation_analysis_w_1_extra_latch();

// -------------------------------------------------------------------------------------------
///
/// @brief Tests the found vulnerabilities of a circuit, which is protected with 2 extra latches
  void test2_simulation_analysis_w_2_extra_latch();

};

#endif // CPP_UNIT_TestSimulationBasedAnalysis_H__
