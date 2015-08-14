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

#ifndef CPP_UNIT_TestSymbTimeAnalysis_H__
#define CPP_UNIT_TestSymbTimeAnalysis_H__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "../src/SymbTimeAnalysis.h"

struct aiger;

// -------------------------------------------------------------------------------------------
///
/// @class TestSymbTimeAnalysis
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class TestSymbTimeAnalysis: public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE (TestSymbTimeAnalysis);
	CPPUNIT_TEST (test1_one_latch);
	CPPUNIT_TEST (test2_one_latch_one_and);
	CPPUNIT_TEST (test3_two_latches);
  CPPUNIT_TEST (test4_analysis_w_1_extra_latch);
  CPPUNIT_TEST (test5_analysis_w_2_extra_latch);
	CPPUNIT_TEST (test6_analysis_w_random_inputs);
	CPPUNIT_TEST (test7_analysis_big_w_random_inputs);
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

	void checkVulnerabilities(string path_to_aiger_circuit,
			vector<string> tc_files, set<unsigned> should_be_vulnerable,
			int num_err_latches);

	protected:

	void test1_one_latch();
	void test2_one_latch_one_and();
	void test3_two_latches();
	// -------------------------------------------------------------------------------------------
	///
	/// @brief Tests the found vulnerabilities of a circuit, which is protected with 1 extra latch
	void test4_analysis_w_1_extra_latch();

	// -------------------------------------------------------------------------------------------
	///
	/// @brief Tests the found vulnerabilities of a circuit, which is protected with 2 extra latches
	void test5_analysis_w_2_extra_latch();

	// -------------------------------------------------------------------------------------------
	///
	/// @brief Searches for vulnerabilities in a circuit using random input vectors
	void test6_analysis_w_random_inputs();

	// -------------------------------------------------------------------------------------------
	///
	/// @brief Searches for vulnerabilities in a *bigger* circuit using random input vectors
	void test7_analysis_big_w_random_inputs();

};

#endif // CPP_UNIT_TestSymbTimeAnalysis_H__
