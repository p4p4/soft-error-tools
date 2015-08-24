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

#include "TestSymbTimeLocationAnalysis.h"
#include "../src/SymbTimeLocationAnalysis.h"
#include "../src/SimulationBasedAnalysis.h" // for comparison
#include "../src/Utils.h"
#include "../src/Logger.h"

extern "C"
{
#include "aiger.h"
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestSymbTimeLocationAnalysis);

// -------------------------------------------------------------------------------------------
void TestSymbTimeLocationAnalysis::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestSymbTimeLocationAnalysis::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestSymbTimeLocationAnalysis::test1()
{
	aiger* circuit = Utils::readAiger("inputs/one_latch.protected.aag");
	SymbTimeLocationAnalysis sta(circuit, 1, SymbTimeLocationAnalysis::STANDARD);
	sta.findVulnerabilities(1, 3);
//	Logger::instance().disable(Logger::DBG);
	CPPUNIT_ASSERT(sta.getVulnerableElements().size() == 0);
	aiger_reset(circuit);

	aiger* circuit2 = Utils::readAiger("inputs/one_latch.unprotected.aag");
	SymbTimeLocationAnalysis sta2(circuit2, 0, SymbTimeLocationAnalysis::STANDARD);
	sta2.findVulnerabilities(1, 3);
	CPPUNIT_ASSERT(sta2.getVulnerableElements().size() == 1);
	aiger_reset(circuit2);
}


