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

#include "TestDefinitelyProtected.h"

extern "C" {
#include "aiger.h"
};
#include "cuddObj.hh"

#include "../src/DefinitelyProtected.h"
#include "../src/Utils.h"
#include "../src/Logger.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestDefinitelyProtected);

// -------------------------------------------------------------------------------------------
void TestDefinitelyProtected::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestDefinitelyProtected::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestDefinitelyProtected::test1()
{
	aiger* circuit = Utils::readAiger("inputs/toggle.perfect.aag");
	DefinitelyProtected dp(circuit, 1);

	dp.findDefinitelyProtected_1();
//  CPPUNIT_FAIL("test not implemented");
//  CPPUNIT_ASSERT(1 == 1);
//  CPPUNIT_ASSERT_MESSAGE("error occurred", 1 == 0);
}


