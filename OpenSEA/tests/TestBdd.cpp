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

#include "TestBdd.h"

extern "C" {
#include "aiger.h"
#include "cudd.h"
};

#include "cuddObj.hh"


#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION(TestBdd);

// -------------------------------------------------------------------------------------------
void TestBdd::setUp()
{
  //setup for testcases
}

// -------------------------------------------------------------------------------------------
void TestBdd::tearDown()
{
  //define here post processing steps
}

// -------------------------------------------------------------------------------------------
void TestBdd::test1()
{

	Cudd cudd;
	cudd.AutodynEnable(CUDD_REORDER_SIFT);

	std::cout << std::endl;

	BDD foo = cudd.bddVar(2);
	BDD bar = cudd.bddVar(3);

	BDD res = foo & bar;

	res = foo & bar;
	res.PrintMinterm();
	res.PrintCover();
	CPPUNIT_ASSERT(!res.IsZero());

	res = foo & ~bar;
	res.PrintMinterm();
	res.PrintCover();
	CPPUNIT_ASSERT(!res.IsZero());

	res = ~foo & bar;
	res.PrintMinterm();
	res.PrintCover();
	CPPUNIT_ASSERT(!res.IsZero());

	res = ~foo & ~bar;
	res.PrintMinterm();
	res.PrintCover();
	CPPUNIT_ASSERT(!res.IsZero());

	//-------------------
	DdNode* node = res.getNode();
	unsigned index = Cudd_NodeReadIndex(node);
	std::cout << "index " << index << std::endl;

	DdNode* thenNode = Cudd_T(node);
	std::cout << "index then " << Cudd_NodeReadIndex(thenNode) << std::endl;
	DdNode* elseNode = Cudd_E(node);
	std::cout << "index else " << Cudd_NodeReadIndex(elseNode) << std::endl;


	res = foo & ~foo;
	res.PrintMinterm();
	res.PrintCover();

	CPPUNIT_ASSERT(res.IsZero());


	// what is the equivalent to satsolver.isSat() for BDDs?
	// how to get a satisfying assignment?


}

// -------------------------------------------------------------------------------------------
void TestBdd::test2()
{
	Cudd cudd;
	DdManager* mgr = cudd.getManager();
}


