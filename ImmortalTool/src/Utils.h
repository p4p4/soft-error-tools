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
//   <http://www.iaik.tugraz.at/content/research/design_verification/demiurge/>
// or email the authors directly.
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file Utils.h
/// @brief Contains the declaration of the class Utils.
// -------------------------------------------------------------------------------------------

#ifndef Utils_H__
#define Utils_H__

#include "defines.h"

class CNF;


struct aiger;

// -------------------------------------------------------------------------------------------
///
/// @class Utils
/// @brief Contains utility functions that can be usful in various back-ends.
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.2.0
class Utils
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a cube (in form of a vector of literals) contains the initial state.
///
/// In our synthesis problems, there is exactly one initial state of the system (this is a
/// restriction imposed by the AIGER format which serves as input for our tool). Furthermore,
/// the initial state is always characterized by having all state bits set to FALSE. However,
/// this may change with future AIGER versions.
///
/// @param cube The cube (a conjuction of literals) in form of a vector of literals over the
///        current-state variables.
/// @return True if the cube contains the initial state (i.e., the initial state satisfies
///         the cube, i.e., the cube contains only negated literals). False otherwise.
  static bool containsInit(const vector<int> &cube);

// -------------------------------------------------------------------------------------------
///
/// @brief reads and parses an aigsim-file and stores (appends!) it as testcase
///
/// The stored testcase contains input vectors for each time-step of the simulation.
/// Attention: this function does NOT clear() the given TestCase &testcase.
///
/// @param path_to_aigsim_input the aigsim input file to parse
/// @param testcase a (potentially empty) TestCase data-structure
  static void parseAigSimFile(string path_to_aigsim_input, TestCase &testcase, unsigned number_of_inputs);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a cube (in form of a set of literals) contains the initial state.
///
/// In our synthesis problems, there is exactly one initial state of the system (this is a
/// restriction imposed by the AIGER format which serves as input for our tool). Furthermore,
/// the initial state is always characterized by having all state bits set to FALSE. However,
/// this may change with future AIGER versions.
///
/// @param cube The cube (a conjuction of literals) in form of a set of literals over the
///        current-state variables.
/// @return True if the cube contains the initial state (i.e., the initial state satisfies
///         the cube, i.e., the cube contains only negated literals). False otherwise.
  static bool containsInit(const set<int> &cube);


// -------------------------------------------------------------------------------------------
///
/// @brief Extracts certain literals from a cube or clause.
///
/// @param cube_or_clause The cube or clause to analyze.
/// @param vars The variables to extract from cube_or_clause.
/// @return All literals of cube_or_clause where the corresponding variable occurs in vars.
  static vector<int> extract(const vector<int> &cube_or_clause, const vector<int> &vars);

// -------------------------------------------------------------------------------------------
///
/// @brief Randomizes the order (not the value) of a given vector of integers.
///
/// @param vec The vector to randomize. This vector is modified in-place, i.e., after calling
///        this method, the order of the elements in the passed vector will be randomized.
  static void randomize(vector<int> &vec);

// -------------------------------------------------------------------------------------------
///
/// @brief Sorts a vector of integers in ascending order.
///
/// @param vec The vector to sort. This vector is modified in-place, i.e., after calling
///        this method, this vector will be sorted.
  static void sort(vector<int> &vec);

// -------------------------------------------------------------------------------------------
///
/// @brief Removes a certain element from a vector.
///
/// This method may change the order of the elements arbitrarily.
///
/// @param vec The vector from which the element should be removed.
/// @param elem The element to remove.
/// @return True if the element was removed, false otherwise.
  static bool remove(vector<int> &vec, int elem);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a vector contains a certain element.
///
/// @param vec The vector in which the element should be searched.
/// @param elem The element to search.
/// @return True if the vector contains the element, false otherwise.
  static bool contains(const vector<int> &vec, int elem);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if two vectors contain the same set of elements.
///
/// @param v1 The first vector for the comparison.
/// @param v2 The first vector for the comparison.
/// @param start_idx The start index for the comparison.
/// @return True if the two vectors contain the same set of elements.
  static bool eq(const vector<int> &v1, const vector<int> &v2, int start_idx = 0);

// -------------------------------------------------------------------------------------------
///
/// @brief Negates all literals in a cube or clause.
///
/// @param cube_or_clause The cube or clause in which all literals should be negated.
  static void negateLiterals(vector<int> &cube_or_clause);

// -------------------------------------------------------------------------------------------
///
/// @brief Replaces all current-state literals by their next-state copy.
///
/// @note The passed vector of literals must only contain current-state literals.
/// @param vec The vector of literals in which all current-state literals (it must only talk
///        about current-state literals) should be replaced by their next-state copy.
  static void swapPresentToNext(vector<int> &vec);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a vector and a set have an empty intersection.
///
/// @param x The vector to check.
/// @param y The set to check.
/// @return True if the intersection of the vector and the set is empty, false otherwise.
  static bool intersectionEmpty(const vector<int> &x, const set<int> &y);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if the intersection of two sets is empty.
///
/// @param x The first set for the check.
/// @param y The second set for the check.
/// @return True if the intersection is empty, false otherwise.
  static bool intersectionEmpty(const set<int> &x, const set<int> &y);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if one vector forms a subset of another one.
///
/// @param subset The potential subset in the check.
/// @param superset The potential superset in the check.
/// @return True if 'subset' is really a subset of 'superset', false otherwise.
  static bool isSubset(const vector<int> &subset, const vector<int> &superset);

// -------------------------------------------------------------------------------------------
///
/// @brief Compresses a state-CNF by removing clauses that are implied by others.
///
/// This is done by incremental SAT-solving and is usually quite fast (at least compared to
/// QBF solving).
///
/// @param cnf The CNF formula to compress.
/// @param hardcore Set this parameter to true if you do not only want to remove clauses but
///        also literals from clauses. This is more expensive, but can produce smaller CNF
///        representations.
/// @return True if the CNF was modified, false otherwise.
  static bool compressStateCNF(CNF &cnf, bool hardcore = false);

// -------------------------------------------------------------------------------------------
///
/// @brief Negates a CNF over the state-variables without introducing temporary variables.
///
/// The negation of the passed CNF is computed via computational learning. It uses a SAT
/// solver in an incremental fashion. This is more expensive than simply calling
/// cnf.negate(), but it does not introduce temporary variables, and hence may lead to a
/// smaller CNF.
///
/// @param cnf The CNF formula to negate.
  static void negateStateCNF(CNF &cnf);

// -------------------------------------------------------------------------------------------
///
/// @brief Negates a CNF by transforming into AIGER and back.
///
/// This method performs the following 3 steps to negate a CNF: (1) the CNF is transformed
/// into AIGER format, (2) optimized with ABC, and (3) transformed back into CNF. The new
/// CNF has one (fresh) temporary variable per AIGER gate.
///
/// @param cnf The CNF formula to negate.
  static void negateViaAig(CNF &cnf);

// -------------------------------------------------------------------------------------------
///
/// @brief Compresses a state-CNF by removing implied clauses and computes the next-state CNF.
///
/// This method is similar to #compressStateCNF(). The difference is that this method also
/// computes a compact representation of the next-state copy of passed state-CNF. When
/// compressing the next-state copy, clauses are removed if they are already implied by
/// existing clauses or the present-state copy of the CNF. That is, the compression of the
/// next-state copy is only valid if the current-state copy is going to be asserted in the
/// solver.
//
///
/// @param ps_cnf The CNF formula to compress. We assume that this CNF only talks about the
///        present-state variables. This CNF is compressed, similar as done by
///        #compressStateCNF().
/// @param ns_cnf An empty CNF. This CNF is filled with the compression of the next-state
///        copy of ps_cnf. The resulting ns_cnf is only valid under the assumption that
///        ps_cnf is asserted.
/// @param hardcore Set this parameter to true if you do not only want to remove clauses but
///        also literals from clauses. This is more expensive, but can produce smaller CNF
///        representations.
  static void compressNextStateCNF(CNF &ps_cnf, CNF &ns_cnf, bool hardcore = false);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the current memory usage as debug message in kB.
///
/// @return The current memory usage as debug message in kB.
  static size_t getCurrentMemUsage();

// -------------------------------------------------------------------------------------------
///
/// @brief Prints a vector of literals (mainly for debugging purposes).
///
///
/// @param vec The vector of literals to print.
/// @param prefix An optional prefix for the debug message.
  static void debugPrint(const vector<int> &vec, string prefix = "");

  static void logPrint(const vector<int> &vec, string prefix = "");

  static inline string vecToString(const vector<int>& vec)
  {
    ostringstream oss;
    oss << "[";
    for(size_t count = 0; count < vec.size(); ++count)
    {
      oss << vec[count];
      if(count != vec.size() - 1)
        oss << ", ";
    }
    oss << "]";
    return  oss.str();
  }

//  static void logPrint(const set<int> &set, string prefix = "");

// -------------------------------------------------------------------------------------------
///
/// @brief Checks a winning region for correctness.
///
/// The check is only done in debug-mode. In release-mode this method does nothing.
/// In debug-mode this method checks three properties of a winning region W.
/// <ol>
///  <li> I(x) => W(x): every initial state must be contained in the winning region
///  <li> W(x) => P(x): every state of the winning region must be safe
///  <li> forall x,i: exists c,x': W(x) => (T(x,i,c,x') & W(x')): from every state in the
///       winning region it must be possible to stay in the winning region by setting the
///       c-signals appropriately.
/// </ol>
/// These three properties are sufficient for turning the winning region into a circuit.
/// However, thise conditions are not necessary. E.g., if optimization RC is used by
/// LearnSynthQBFInd or LearnSynthSAT, the third property does not hold. Hence, these
/// classes have special methods to check such winning regions.
///
/// @param winning_region The winning region to check.
  static void debugCheckWinReg(const CNF &winning_region);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks a winning region and its negation for correctness.
///
/// Use this method if you have the negation of the winning region available. Otherwise, we
/// would end up with a double-negation, which is inefficient.
///
/// The check is only done in debug-mode. In release-mode this method does nothing.
/// In debug-mode this method checks three properties of a winning region W.
/// <ol>
///  <li> I(x) => W(x): every initial state must be contained in the winning region
///  <li> W(x) => P(x): every state of the winning region must be safe
///  <li> forall x,i: exists c,x': W(x) => (T(x,i,c,x') & W(x')): from every state in the
///       winning region it must be possible to stay in the winning region by setting the
///       c-signals appropriately.
/// </ol>
/// These three properties are sufficient for turning the winning region into a circuit.
/// However, thise conditions are not necessary. E.g., if optimization RC is used by
/// LearnSynthQBFInd or LearnSynthSAT, the third property does not hold. Hence, these
/// classes have special methods to check such winning regions.
///
/// @param winning_region The winning region to check.
/// @param neg_winning_region The negation of the winning region to check.
  static void debugCheckWinReg(const CNF &winning_region, const CNF &neg_winning_region);


  static inline int applyRen(const vector<int> &rename_map, int lit)
  {
  	return lit < 0 ? -rename_map[-lit] : rename_map[lit];
  }

  static inline int readCnfValue(const vector<int> results, unsigned aigerlit)
  {
  	return aigerlit & 1 ? -results[(aigerlit >> 1)] : results[(aigerlit >> 1)];
  }

  static aiger* readAiger(string path);

// -------------------------------------------------------------------------------------------
///
/// @brief Prints the current memory usage as debug message.
  static void debugPrintCurrentMemUsage();

protected:

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// The constructor is private and not implemented. Use the static methods.
  Utils();

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// The constructor is private and not implemented. Use the static methods.
  virtual ~Utils();

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  Utils(const Utils &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  Utils& operator=(const Utils &other);

};

#endif // Utils_H__
