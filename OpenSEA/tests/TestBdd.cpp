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

#include "../src/Utils.h"


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

void TestBdd::getSatAss(const Cudd &cudd, const BDD &bdd, std::vector<int> &dest) const
{
    CPPUNIT_ASSERT(!bdd.IsZero());
    dest.clear();
    DdNode *currentNode = bdd.getNode();
    DdNode *constantOne = cudd.bddOne().getNode();
    DdNode *constantZero = cudd.bddZero().getNode();
    while (currentNode != constantOne) {
        CPPUNIT_ASSERT(currentNode != constantZero);
        int index = Cudd_NodeReadIndex(currentNode);
        if (Cudd_IsComplement(currentNode)) {
            if (Cudd_E(currentNode)==constantOne) {
                // Must not take else branch
                dest.push_back(index);
                currentNode = Cudd_Not(Cudd_T(currentNode));
            } else {
                dest.push_back(-index);
                currentNode = Cudd_Not(Cudd_E(currentNode));
            }
        } else {
            if (Cudd_E(currentNode)==constantZero) {
                // Must not take else branch
                dest.push_back(index);
                currentNode = Cudd_T(currentNode);
            } else {
                dest.push_back(-index);
                currentNode = Cudd_E(currentNode);
            }
        }
    }
}

// -------------------------------------------------------------------------------------------
void TestBdd::test1()
{
	Cudd cudd;
	cudd.AutodynEnable(CUDD_REORDER_SIFT);

	CPPUNIT_ASSERT(cudd.bddZero() == ~ cudd.bddOne());

	std::cout << std::endl;

	BDD foo = cudd.bddVar(2);
	BDD bar = cudd.bddVar(3);

	BDD res = foo & bar;
	std::vector<int> sat_ass;
	getSatAss(cudd, res, sat_ass);
	std::cout << "Result of getSatAss" << std::endl;
	for (size_t i = 0; i < sat_ass.size(); ++i)
		std::cout << sat_ass[i] << ", ";
	std::cout << std::endl;

	Utils::debugPrint(sat_ass, "sat ass: ");
	char sat_ass_string[4];
	res.PickOneCube(sat_ass_string);
	// prints a value vor bddVar 0,1,2,3,4,...
	// value 2 means: irrelevant
	// value 1 means: true
	// value 0 means: false
	std::cout << "Result of PickOneCube (2 means: irrelevant)" << std::endl;
	for (size_t i = 0; i < 4; ++i)
		std::cout << "  var " << i << " has value " << static_cast<int>(sat_ass_string[i])
				<< std::endl;

	res = foo & bar;
	res.PrintMinterm();
	CPPUNIT_ASSERT(!res.IsZero());

	res = foo & ~bar;
	res.PrintMinterm();
	CPPUNIT_ASSERT(!res.IsZero());

	res = ~foo & bar;
	res.PrintMinterm();
	CPPUNIT_ASSERT(!res.IsZero());

	res = ~foo & ~bar;
	res.PrintMinterm();
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
void TestBdd::test2_generate_cj_BDDs()
{
	Cudd cudd;

	int num_of_cj_signals = 4;
	map<int, int> latch_to_cj; // dummy cj literals

	for(int i = 0; i < num_of_cj_signals; i++)
		latch_to_cj[i] = i;


	map<int, int>::iterator map_iter;
	map<int, int>::iterator map_iter2;

	map<int, BDD> cj_to_BDD_signal;

	// --------------------------------------------------------------------------------------------------
	// generate 1 hot encoding:
	for (map_iter = latch_to_cj.begin(); map_iter != latch_to_cj.end(); map_iter++) // for each latch:
	{
		BDD real_c_signal = cudd.bddOne();
		for (map_iter2 = latch_to_cj.begin(); map_iter2 != latch_to_cj.end(); map_iter2++) // for current latch, go over all latches
		{
			if (map_iter == map_iter2) // the one and only cj-signal which can be true for this signal
				real_c_signal = real_c_signal & cudd.bddVar(map_iter2->second);
			else
				real_c_signal = real_c_signal & ~cudd.bddVar(map_iter2->second);
		}
		cj_to_BDD_signal[map_iter->second] = real_c_signal;
	}


	// --------------------------------------------------------------------------------------------------
	// test 1 hot encoding for correctness
	char sat_ass_string[num_of_cj_signals];
	for(int i = 0; i < num_of_cj_signals; i++)
	{
		cj_to_BDD_signal[i].PickOneCube(sat_ass_string);
		for (size_t j = 0; j < num_of_cj_signals; j++)
		{
			std::cout << "c[" << i <<"]  var " << j << " has value " << static_cast<int>(sat_ass_string[j]) << std::endl;
			CPPUNIT_ASSERT(i == j ? sat_ass_string[j] == 1 : sat_ass_string[j] == 0);
		}

	}


}


// -------------------------------------------------------------------------------------------
void TestBdd::test3_modify_BDD_signal()
{
	Cudd cudd;

	// input signals
	BDD in_0 = cudd.bddVar(0);
	BDD in_1 = cudd.bddVar(1);
	BDD in_2 = cudd.bddVar(2);

	// output = in_0 & in_1 & in_2
	BDD intermediate_result = in_0 & in_1;
	BDD output = intermediate_result & in_2;

	// check assignment
	char sat_ass_string[3];
	output.PickOneCube(sat_ass_string);
	for (size_t j = 0; j < 3; j++)
	{
		std::cout << "var " << j << " has value " << static_cast<int>(sat_ass_string[j]) << std::endl;
		CPPUNIT_ASSERT(sat_ass_string[j] == 1);
	}

	// modify intermediate result
	intermediate_result = in_0 & ~in_1;

	/*  // or this way?
	BDD new_intermediate_result = in_0 & ~in_1;
	vector<BDD> oldVar, newVar;
	oldVar.push_back(intermediate_result);
	newVar.push_back(new_intermediate_result);
	output.SwapVariables(oldVar,newVar);
	*/


	// check SAT assignment again:
	output.PickOneCube(sat_ass_string);
	for (size_t j = 0; j < 3; j++)
	{
		std::cout << "var " << j << " has value " << static_cast<int>(sat_ass_string[j]) << std::endl;
	}
	CPPUNIT_ASSERT(sat_ass_string[1] == 0); // TODO fails! modification of intermediate result does not affect the output!
	// how can we achieve an updated behavior when modifying an intermediate node?

	// I need this as an equivalent of SAT-solver blocking clauses and when I add new f variables.
	// solution: use side-constraints


}


