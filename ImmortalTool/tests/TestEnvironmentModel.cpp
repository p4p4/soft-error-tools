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

#include "TestEnvironmentModel.h"
#include "../src/Logger.h"
#include "../src/AigSimulator.h"
#include "../src/SimulationBasedAnalysis.h"
#include "../src/Utils.h"

extern "C"
{
#include "aiger.h"
}


CPPUNIT_TEST_SUITE_REGISTRATION(TestEnvironmentModel);

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestEnvironmentModel::test1()
{
  aiger* circuit = Utils::readAiger("inputs/one_latch.unprotected.aag");
  aiger* environment = Utils::readAiger("inputs/one_latch.unprotected.aag");

  SimulationBasedAnalysis sba(circuit,0);
  sba.findVulnerabilities(1,5); // first without environment
  CPPUNIT_ASSERT(sba.getVulnerableElements().size() == 1);

  // in this test the output is alway false, i.e. we don't care about the output
  sba.setEnvironmentModel(environment);
  sba.findVulnerabilities(1,5);
  CPPUNIT_ASSERT(sba.getVulnerableElements().size() == 0);
}


