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

#include <fstream>
#include <string>
#include <string.h>
#include <cppunit/TestAssert.h>


//for Console
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/Test.h>
#include <cppunit/TestPath.h>

#include <cppunit/extensions/TestFactoryRegistry.h>

#include <sstream>
#include <stdlib.h>

using namespace std;

CPPUNIT_NS::Test* allTests;

void printSubTests(CPPUNIT_NS::Test* test) {
  for (int i=0; i< test->getChildTestCount(); ++i) {
    cout << test->getChildTestAt(i)->getName() << endl;
  }
}

void printTests(bool suite=true) {
  CPPUNIT_NS::Test* allTests;
  allTests = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
  
  for (int i=0; i< allTests->getChildTestCount(); ++i) {
    if(suite) {
      cout << allTests->getChildTestAt(i)->getName() << endl;
    } else {
      printSubTests( allTests->getChildTestAt(i) );
    }
  }
  delete allTests;
}

CPPUNIT_NS::Test* selectedTests (int argc, char** argv) 
{
  allTests = CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest();
 
  if(argc == 1) {
    return allTests;
  }

  CPPUNIT_NS::TestSuite* testsToRun = new CPPUNIT_NS::TestSuite;

  for(int i = 1; i < argc; ++i)
  {
    //throws an exception, if test fails 
    //as 20100601
    testsToRun->addTest(allTests->findTest (argv[i]));
  }
  return testsToRun;
}

int withConsole(int argc, char** argv) 
{
  allTests = NULL;
  // Create the event manager and test controller
  CPPUNIT_NS::TestResult controller;

  // Add a listener that colllects test result
  CPPUNIT_NS::TestResultCollector result;
  controller.addListener( &result );        

  // Add a listener that print dots as test run.
  CPPUNIT_NS::BriefTestProgressListener progress;
  controller.addListener( &progress );      

  // Add the top suite to the test runner
  // create a single memory leak to allow deletion of the whole test suite
  CPPUNIT_NS::TestRunner* runner = new CPPUNIT_NS::TestRunner;
  runner->addTest( selectedTests(argc,argv) );
  runner->run( controller );

  // Print test in a compiler compatible format.
  CPPUNIT_NS::CompilerOutputter outputter( &result, std::cerr );
  outputter.write(); 

  std::ofstream file( "testresults-cppunit.xml" );
  CPPUNIT_NS::XmlOutputter xml( &result, file );
  xml.write();
  file.close();

  delete allTests;
  return result.wasSuccessful() ? 0 : 1;
}

void assert_abort() {
  CPPUNIT_ASSERT(false);
}

int main( int argc, char** argv )
{
  if (argc > 1) {
    if (strcmp(argv[1],"--list") == 0) {
      printTests();
      return 0;
    } else if (strcmp(argv[1],"--list-single") == 0) {
      printTests(false);
      return 0;
    }
  }
  
  return withConsole(argc, argv);
}

