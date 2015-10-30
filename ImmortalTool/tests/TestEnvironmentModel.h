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

#ifndef CPP_UNIT_TestEnvironmentModel_H__
#define CPP_UNIT_TestEnvironmentModel_H__

#include "../src/BackEnd.h"
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

// -------------------------------------------------------------------------------------------
///
/// @class TestEnvironmentModel
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class TestEnvironmentModel: public CPPUNIT_NS::TestFixture
{

	void basic_test_1(std::string backend, int mode);

	void basic_test_2(std::string backend, int mode);

	BackEnd* getBackend(const std::string& backend_name, int mode, aiger* circuit);

CPPUNIT_TEST_SUITE(TestEnvironmentModel);
	CPPUNIT_TEST(test1_sim_basic_1);
	CPPUNIT_TEST(test2_sim_basic_2);
	CPPUNIT_TEST(test3_sta0_basic_1);
	CPPUNIT_TEST(test4_sta0_basic_2);
	CPPUNIT_TEST(test5_sta1_basic_1);
	CPPUNIT_TEST(test6_sta1_basic_2);
	CPPUNIT_TEST(test7_sta2_basic_1);
	CPPUNIT_TEST(test8_sta2_basic_2);
	CPPUNIT_TEST_SUITE_END()
	;

public:

// -------------------------------------------------------------------------------------------
///
/// @brief Initializes the object under test.
	void setUp();

// -------------------------------------------------------------------------------------------
///
/// @brief Shuts down the object under test.
	void tearDown();

protected:

// -------------------------------------------------------------------------------------------
///
/// @brief Basic env test 1, sim
	void test1_sim_basic_1();

// -------------------------------------------------------------------------------------------
///
/// @brief Basic env test 2, sim
	void test2_sim_basic_2();

// -------------------------------------------------------------------------------------------
///
/// @brief Basic env test 1, sta0
	void test3_sta0_basic_1();

// -------------------------------------------------------------------------------------------
///
/// @brief Basic env test 2, sta0
	void test4_sta0_basic_2();

// -------------------------------------------------------------------------------------------
///
/// @brief Basic env test 1, sta1
	void test5_sta1_basic_1();

// -------------------------------------------------------------------------------------------
///
/// @brief Basic env test 2, sta1
	void test6_sta1_basic_2();

// -------------------------------------------------------------------------------------------
///
/// @brief Basic env test 1, sta2
	void test7_sta2_basic_1();

// -------------------------------------------------------------------------------------------
///
/// @brief Basic env test 2, sta2
	void test8_sta2_basic_2();

};

#endif // CPP_UNIT_TestEnvironmentModel_H__
