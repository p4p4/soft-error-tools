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

#include "@TestclassName@.h"

CPPUNIT_TEST_SUITE_REGISTRATION(@TestclassName@);

// -------------------------------------------------------------------------------------------
void @TestclassName@::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void @TestclassName@::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void @TestclassName@::test1()
{
  CPPUNIT_FAIL("test not implemented");
  CPPUNIT_ASSERT(1 == 1);
  CPPUNIT_ASSERT_MESSAGE("error occurred", 1 == 0);
}


