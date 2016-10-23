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

#ifndef CPP_UNIT_TestDefinitelyProtected_H__
#define CPP_UNIT_TestDefinitelyProtected_H__


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>


// -------------------------------------------------------------------------------------------
///
/// @class TestDefinitelyProtected
/// @brief TODO
///
/// @author TODO
/// @version 1.2.0
class TestDefinitelyProtected : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE(TestDefinitelyProtected);
  CPPUNIT_TEST(test1);
  CPPUNIT_TEST(test_multi_step);
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

  unsigned start_mode_;
  unsigned last_mode_;

// -------------------------------------------------------------------------------------------
///
/// @brief Give brief description of test here.
  void test1();
  void test_multi_step();

};

#endif // CPP_UNIT_TestDefinitelyProtected_H__
