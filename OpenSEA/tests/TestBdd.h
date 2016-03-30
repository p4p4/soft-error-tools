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

#ifndef CPP_UNIT_TestBdd_H__
#define CPP_UNIT_TestBdd_H__


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class Cudd;
class BDD;

// -------------------------------------------------------------------------------------------
///
/// @class TestBdd
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class TestBdd : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(TestBdd);
  CPPUNIT_TEST(test1);
  CPPUNIT_TEST(test2_generate_cj_BDDs);
  CPPUNIT_TEST(test3_modify_BDD_signal);
  CPPUNIT_TEST(test4_analysis_basic);
  CPPUNIT_TEST(test5_analysis_3latches);
  CPPUNIT_TEST(test6_analysis_w_1_extra_latch);
  CPPUNIT_TEST(test7_analysis_compare_with_simulation_1);
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

	void checkVulnerabilities(std::string path_to_aiger_circuit,
			std::vector<std::string> tc_files, std::set<unsigned> should_be_vulnerable,
			int num_err_latches, int mode = 0);

	void compareWithSimulation(std::string path_to_aiger_circuit, int num_tc,
			int num_timesteps, int num_err_latches,
			int mode = 0);

protected:

  void getSatAss(const Cudd &cudd, const BDD &bdd, std::vector<int> &dest) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Give brief description of test here.
  void test1();
  void test2_generate_cj_BDDs();
  void test3_modify_BDD_signal();

  void test4_analysis_basic();
  void test5_analysis_3latches();
  void test6_analysis_w_1_extra_latch();
  void test7_analysis_compare_with_simulation_1();

};

#endif // CPP_UNIT_TestBdd_H__
