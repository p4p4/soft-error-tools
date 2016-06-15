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
//	Logger::instance().enable(Logger::DBG);
}

// -------------------------------------------------------------------------------------------
void TestDefinitelyProtected::tearDown()
{
  //define here post processing steps
	Logger::instance().disable(Logger::DBG);
}

// -------------------------------------------------------------------------------------------
void TestDefinitelyProtected::test1()
{
	unsigned last_mode = 3;


	aiger* circuit = Utils::readAiger("inputs/toggle.perfect.aag");
	for (unsigned mode = 0; mode <= last_mode; mode++)
	{
		L_DBG(endl << endl << "mode = " << mode)
		DefinitelyProtected dp(circuit, 0, mode);
		dp.analyze();
		cout << dp.getDetectedLatches().size() << endl;
		CPPUNIT_ASSERT(dp.getDetectedLatches().size() == 4);
	}
	aiger_reset(circuit);

	circuit = Utils::readAiger("inputs/toggle.1vulnerability.aag");
	for (unsigned mode = 0; mode <= last_mode; mode++)
	{
		L_DBG(endl << endl << "mode = " << mode)
		DefinitelyProtected dp(circuit, 0, mode);
		dp.analyze();
		CPPUNIT_ASSERT(dp.getDetectedLatches().size() == 3);
	}
	aiger_reset(circuit);

	circuit = Utils::readAiger("inputs/toggle.3vulnerabilities.aag");
	for (unsigned mode = 0; mode <= last_mode; mode++)
	{
		L_DBG(endl << endl << "mode = " << mode)
		DefinitelyProtected dp(circuit, 0, mode);
		dp.analyze();
		CPPUNIT_ASSERT(dp.getDetectedLatches().size() == 0);
	}
	aiger_reset(circuit);

	//	aiger* circuit = Utils::readAiger("inputs/toggle.2vulnerabilities.aag");

}


