// ----------------------------------------------------------------------------
// Copyright (c) 2013-2015 by Graz University of Technology and
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
/// @file CNF.h
/// @brief Contains the declaration of the class CNF.
// -------------------------------------------------------------------------------------------

#ifndef CNF_H__
#define CNF_H__

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <fstream>
#include <algorithm>
#include <limits>
#include <exception>

using namespace std;

// -------------------------------------------------------------------------------------------
///
/// @def MASSERT(condition, message)
/// @brief A macro for an assertion associated with an error message.
///
/// If the condition evaluates to false, the error message is printed and the program
/// terminates.
///
/// @param condition The condition of the assertion. If this condition evaluates to false,
///        then the error message is printed.
/// @param message The error message to be printed upon assertion violation.
#define MASSERT(condition, message)                                          \
{                                                                            \
  if(!(condition))                                                           \
  {                                                                          \
    std::cerr << __FILE__ << " (" << __LINE__ << ") : " << message << endl;  \
    abort();                                                                 \
  }                                                                          \
}

// -------------------------------------------------------------------------------------------
///
/// @class CNF
/// @brief Represents a propositional formula in Conjunctive Normal Form.
///
/// This class also provides many methods for manipulating CNFs conveniently.
///
/// A Conjunctive Normal Form formula is a conjunction of clauses. Each clause is a
/// disjunction of literals. A literal is a Boolean variable or its negation. Internally,
/// every literal is represented as integer number (0 is not used). Every clause is
/// a vector of literals. A CNF is a list of clauses. This representation was chosen for
/// pragmatic reasons: we often manipulate clauses, so we chose a simple representation which
/// can be manipulated efficiently. The CNF is a list (and no vector) of clauses because
/// we often append clauses, which can be inefficient with vectors. Furthermore, we usually
/// do not need random access to individual clauses.
///
/// Other options would be (a) to sort the literals in the clauses, (b) represent clauses as
/// sets, (c) represent a CNF as set of sets of integers, etc. So far, we considered these
/// options an over-kill because the time for manipulating CNFs is negligable compared to
/// SAT- or QBF-solving time.
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.2.0
class CNF
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// After construction, the CNF represents the empty set of clauses, i.e., the formula TRUE.
  CNF();

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor from DIMACS file.
///
/// @param filename The name (including path and extension) of the DIMACS file containing the
///        definition of the CNF.
  CNF(const string &filename);

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// Makes a (deep) copy of the CNF.
///
/// @param other The source for creating the (deep) copy.
  CNF(const CNF &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// Makes this object a deep copy of another object.
///
/// @param other The source for creating the (deep) copy.
/// @return The result of the assignment, i.e, *this.
  virtual CNF& operator=(const CNF &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~CNF();

// -------------------------------------------------------------------------------------------
///
/// @brief Clears the CNF, i.e., sets the CNF back to TRUE (the empty set of clauses).
  void clear();

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new clause to the CNF.
///
/// @param clause The new clause to add.
  void addClause(const vector<int> &clause);

// -------------------------------------------------------------------------------------------
///
/// @brief Conjuncts the CNF with the negation of a given clause (which is a cube then).
///
/// For instance, if the clause [2, -4, 8] is passed as argument, then this method adds the
/// the three unit clauses [-2], [4], [-8] to the CNF.
///
/// @param clause The clause which's negation should be conjuncted to the CNF.
  void addNegClauseAsCube(const vector<int> &clause);

// -------------------------------------------------------------------------------------------
///
/// @brief Conjuncts the CNF with a given cube.
///
/// This means: every literal of the cube is added as a unit clause to the CNF.
/// For instance, if the cube [2, -4, 8] is passed as argument, then this method adds the
/// three unit clauses [2], [-4], [8] to the CNF.
///
/// @param cube The new cube to add.
  void addCube(const vector<int> &cube);

// -------------------------------------------------------------------------------------------
///
/// @brief Conjuncts the CNF with the negation of a given cube (which is a clause then).
///
/// For instance, if the cube [2, -4, 8] is passed as argument, then this method adds the
/// clauses [-2, 4, -8] to the CNF.
///
/// @param cube The cube which's negation should be conjuncted to the CNF.
  void addNegCubeAsClause(const vector<int> &cube);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new clause to the CNF, and then simplifies the CNF.
///
/// The only simplification we do is to remove all clauses which form a superset of the
/// newly added clause.
///
/// @param clause The new clause to add.
/// @return True if some clauses were removed due to simplification, false otherwise.
  bool addClauseAndSimplify(const vector<int> &clause);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a unit clause to the CNF.
///
/// @param lit1 The (one and only) literal of the unit clause to add.
  void add1LitClause(int lit1);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a clause consisting of two literal to the CNF.
///
/// @param lit1 The first literal of the clause to add.
/// @param lit2 The second literal of the clause to add.
  void add2LitClause(int lit1, int lit2);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a clause consisting of three literal to the CNF.
///
/// @param lit1 The first literal of the clause to add.
/// @param lit2 The second literal of the clause to add.
/// @param lit3 The third literal of the clause to add.
  void add3LitClause(int lit1, int lit2, int lit3);


// -------------------------------------------------------------------------------------------
///
/// @brief Adds a clause consisting of four literal to the CNF.
///
/// @param lit1 The first literal of the clause to add.
/// @param lit2 The second literal of the clause to add.
/// @param lit3 The third literal of the clause to add.
/// @param lit4 The fourth literal of the clause to add.
  void add4LitClause(int lit1, int lit2, int lit3, int lit4);

// -------------------------------------------------------------------------------------------
///
/// @brief Removes and returns the smallest clause (least number of literals).
///
/// It is guaranteed that the removed clause is among the clauses with the lowest cardinality.
/// If there are more such clauses, an arbitrary one is chosen.
///
/// @return The clause that has been removed from the CNF.
  vector<int> removeSmallest();

// -------------------------------------------------------------------------------------------
///
/// @brief Removes and returns some clause.
///
/// @return The clause that has been removed from the CNF.
  vector<int> removeSomeClause();

// -------------------------------------------------------------------------------------------
///
/// @brief Conjuncts the CNF with another CNF.
///
/// That is, this method appends all clauses of the other CNF to this one.
///
/// @param cnf The CNF that shall be conjuncted to this CNF.
  void addCNF(const CNF& cnf);

// -------------------------------------------------------------------------------------------
///
/// @brief Swaps the content of this CNF with the passed set of clauses.
///
/// This is cheaper than copying or assigning.
///
/// @param clauses The clause set to swap content with.
  void swapWith(list<vector<int> > &clauses);

// -------------------------------------------------------------------------------------------
///
/// @brief Swaps the content of this CNF with the passed CNF.
///
/// This is cheaper than copying or assigning.
///
/// @param other The cnf to swap content with.
  void swapWith(CNF &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the number of clauses in this CNF.
///
/// @return The number of clauses in this CNF.
  size_t getNrOfClauses() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the sum of the sizes of all clauses in the CNF.
///
/// That is, if a literal is contained in several clauses then it is counted several times.
/// This is a measure of the size of the CNF.
///
/// @return The sum of the sizes of all clauses in the CNF.
  size_t getNrOfLits() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a string representation of the CNF in DIMACS format.
///
/// The string is returned in DIMACS format, i.e., you can pass this string directly on to
/// SAT- and QBF-solvers.
/// DIMACS format means: every clause is printed as the list of its literals. Clauses are
/// terminated by the special number '0' (which cannot be a literal).
///
/// @return A string representation of the CNF in DIMACS format.
  string toString() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Stores the CNF into a file in DIMACS format.
///
/// The CNF is stored in DIMACS format, i.e., you can pass this file directly on to
/// SAT-solvers.
/// DIMACS format means: every clause is printed as the list of its literals. Clauses are
/// terminated by the special number '0' (which cannot be a literal).
///
/// @param filename The name of the file to write to.
  void saveToFile(const string &filename) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the clauses of the CNF as list of vectors.
///
/// @return The clauses of the CNF as list of vectors.
  const list<vector<int> >& getClauses() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Simplifies the CNF syntactically.
///
/// The only simplification we do is to remove all clauses which form a superset of other
/// clauses.
  void simplify();

// -------------------------------------------------------------------------------------------
///
/// @brief Removes all duplicates from the CNF.
///
/// This method does even more: it transforms the CNF into a set of sets, and back.
/// This means that all clauses and literals in clauses will be sorted according to a certain
/// criterion after executing this method.
/// The reason for this method is: sometimes we would actually prefer a representation of
/// clauses as sets of sets.
  void removeDuplicates();

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if the CNF is satisfied by a certain variable assignment.
///
/// This is a purely syntactic check, i.e., no SAT-solver is involved.
/// The variable assignment is passed as a cube with the following meaning: variables
/// occurring positively in the cube are set to TRUE, variables occurring in negative
/// polarity are set to FALSE.
///
/// @param cube The variable assignment in form of a cube: variables occurring positively in
///        the cube are interpreted as TRUE, variables occurring in negative polarity are
///        considered to be FALSE.
/// @return True if the CNF is satisfied by the passed variable assignment, false otherwise.
  bool isSatBy(const vector<int> &cube) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Replaces a variable in the CNF by true or false.
///
/// @param var The variable to replace.
/// @param value The truth value of this variable.
  void setVarValue(int var, bool value);

// -------------------------------------------------------------------------------------------
///
/// @brief Simplifies the CNF by propagating unit clauses and pure literals until fixpoint.
///
/// @todo So far, only unit clause propagation has been implemented. Pure literals still
///       need to be done.
///
/// @param keep A list of CNF variables. All CNF variables contained in the list will be
///        kept assigned by the CNF (in a unit clause).
  void doPureAndUnit(const vector<int> &keep);

// -------------------------------------------------------------------------------------------
///
/// @brief Renames all variables in a CNF.
///
/// @param rename_map A map containing the new names of the variables. Every occurrence of
///        variable i is replaced by rename_map[i]. If a certain variable i should not be
///        renamed, then set rename_map[i]=i. This rename_map must contain a new name for
///        EVERY variable that may appear in the CNF.
  void renameVars(const vector<int> &rename_map);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns true if this CNF contains a certain variable, and false otherwise.
///
/// @param var The variable to check for.
/// @return True if this CNF contains var, and false otherwise.
  bool contains(int var) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the set of all variables that occur in the CNF.
///
/// @return The set of all variables that occur in the CNF.
  vector<int> getVars() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Adds all variables that occur in the CNF to the given set.
///
/// @param var_set The set to which all variables in the CNF should be appended to.
  void appendVarsTo(set<int> &var_set) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Compares two CNFs syntactically.
///
/// This is a purely syntactical check, i.e., no SAT-solving is involved.
/// If this method returns true, then the two CNFs are definitely equivalent. If it returns
/// False, it could still be that they are equivalent.
///
/// @param other The other CNF to compare with.
/// @return True if the two CNFs are syntactically equivalent, False otherwise. If false
///         is returned, then the two CNFs can still be equivalent.
  bool operator==(const CNF &other) const;

// -------------------------------------------------------------------------------------------
///
/// @typedef list<vector<int> >::const_iterator ClauseConstIter
/// @brief A type for a const-iterator over the clauses.
  typedef list<vector<int> >::const_iterator ClauseConstIter;

// -------------------------------------------------------------------------------------------
///
/// @typedef list<vector<int> >::iterator ClauseIter
/// @brief A type for an iterator over the clauses.
  typedef list<vector<int> >::iterator ClauseIter;

protected:

// -------------------------------------------------------------------------------------------
///
/// @brief The CNF represented as list of clauses (in turn being vectors of literals).
  list<vector<int> > clauses_;

};

#endif // CNF_H__
