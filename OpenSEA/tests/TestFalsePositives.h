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

#ifndef CPP_UNIT_TestFalsePositives_H__
#define CPP_UNIT_TestFalsePositives_H__


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "../src/defines.h"
#include "../src/SuperFluousTrace.h"




// -------------------------------------------------------------------------------------------
///
/// @class TestFalsePositives
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class TestFalsePositives : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(TestFalsePositives);
  CPPUNIT_TEST(test1_no_alarm);
  CPPUNIT_TEST(test2_alarm_without_error);
  CPPUNIT_TEST(test3);
  CPPUNIT_TEST(test4);
  CPPUNIT_TEST(test5_irrelevant_latches);
  CPPUNIT_TEST(test6_irrelevant_latches_delayOneTimestep);
  CPPUNIT_TEST(test7_free_inputs);
  CPPUNIT_TEST(test8_environment_input_model);
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

protected:


  void test1_no_alarm();
  void test2_alarm_without_error();
  void test3();
  void test4();
  void test5_irrelevant_latches();
  void test6_irrelevant_latches_delayOneTimestep();
  void checkIrrelevantLatchesDelayOneTraces(vector<SuperfluousTrace*> sftrace);
  void test7_free_inputs();
  void checkTest7Traces(vector<SuperfluousTrace*> sftrace);
  void test8_environment_input_model();

};

#endif // CPP_UNIT_TestFalsePositives_H__
