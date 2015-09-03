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
/// @file MiniSatApi.h
/// @brief Contains the declaration of the class MiniSatApi.
// -------------------------------------------------------------------------------------------

#ifndef MiniSatApi_H__
#define MiniSatApi_H__

#include "defines.h"
#include "SatSolver.h"

namespace Minisat
{
  class Solver;
}

// -------------------------------------------------------------------------------------------
///
/// @class MiniSatApi
/// @brief Interfaces the MiniSat SAT-solver via its API.
///
/// This class represents an interface to the SAT-solver Minisat (see
/// http://minisat.se). It is a concrete implementation of the SatSolver
/// interface. For a given CNF, this class is able to determine
/// satisfiability. Furthermore, in case of satisfiability, it can extract satisfying
/// assignments. In case of unsatisfiability, it can compute an unsatisfiable core. It can be
/// used in two different ways. In the incremental usage scenario, all information the solver
/// has learned so far is retained. Methods for incremental solving start with 'inc'. Other
/// methods (like #isSat() or #isSatModelOrCore()) instantiate a fresh solver instance for
/// every call.
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.2.0
class MiniSatApi : public SatSolver
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// @param rand_models A flag indicating if satisfying assignments should be randomized.
///        This is done in a post-processing step (values are flipped randomly and then we
///        check if this still constitutes a satisfying assignment). This is expensive.
///        If this parameter is skipped, then satisfying assignments are not randomized.
/// @param min_cores A flag indicating if unsatisfiable cores returned by the solver should
///        be minimized further by trying to drop one literal after the other. This makes the
///        calls slower but produces potentially smaller cubes.
  MiniSatApi(bool rand_models = false, bool min_cores = true);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~MiniSatApi();

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a CNF is satisfiable.
///
/// This method is not incremental. At aver call to this method, a new solver instance is
/// created and deleted afterwards. This method does not interfere with an incremental
/// session that may be open in parallel.
///
/// @param cnf The CNF formula for which we want to know if it is satisfiable.
/// @return True in case of satisfiability, false otherwise.
  virtual bool isSat(const CNF &cnf);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a CNF is satisfiable and extracts a model or an unsatisfiable core.
///
/// This method is not incremental. At aver call to this method, a new solver instance is
/// created and deleted afterwards. This method does not interfere with an incremental
/// session that may be open in parallel.
/// This method checks if the passed CNF is satisfiable given that all literals passed in
/// the vector 'assumptions' is true. If this is the case, then a satisfying assignment will
/// be written into model_or_core. You can specify which variables you want to have in the
/// satisfying assignment using the vars_of_interest vector. The reason is that you CNF may
/// contain thousands of temporary variables stemming from some Tseitin encoding, but
/// usually one in only interested in the value of a few variables. In case of
/// unsatisfiability, an unsatisfiable core is stored in model_or_core. The unsatisfiable
/// core is a subset of the literals passed in 'assumptions' such that the CNF is still
/// unsatisfiable when these literals hold.
///
/// @param cnf The CNF formula for which we want to know if it is satisfiable (in conjunction
///        with the assumptions).
/// @param assumptions A vector of literals. These literals are conjuncted to the CNF before
///        solving. If you do not have any assumptions but want to decide the satisfiability
///        of the cnf only, then simply leave this vector empty.
/// @param vars_of_interest The variables for which you want to have a value in case of
///        satisfiability.
/// @param model_or_core An empty vector. Depending on the outcome of the call, either a
///        satisfying assignment (a cube over the variables passed in vars_of_interest) or an
///        unsatisfiable core (a subset of the literals passed in 'assumptions') will be
///        written into this vector.
/// @return True in case of satisfiability (of the CNF conjuncted with all assumptions),
///         false otherwise.
  virtual bool isSatModelOrCore(const CNF &cnf,
                        const vector<int> &assumptions,
                        const vector<int> &vars_of_interest,
                        vector<int> &model_or_core);

// -------------------------------------------------------------------------------------------
///
/// @brief Starts a new incremental session.
///
/// Every instance of this class can have at most one open incremental session.
/// If there is an old open incremental session it will be closed before. An incremental
/// session allows you to execute sequences of calls, where the information the solver
/// learned in previous calls is retained and may speedup later calls. You can add new
/// clauses between calls, but you cannot remove clauses.
///
/// @param vars_to_keep A set of variables the solver should not optimize away. Clauses added
///        later in an incremental session can only talk about variables contained in this
///        vector. The MiniSatApi actually completely ignores these variables. However, other
///        SatSolver implementations require this information, so it must be provided in any
///        case (otherwise we could not exchange one SatSolver implementation by another one).
/// @param use_push A hint to the solver if you are ever going to use #incPush() or
///        #incPop(). This information is particularly important for the MiniSatApi (other
///        implementations of the SatSolver interface may ignore this information). The reason
///        is that MiniSat does not support push and pop natively. Also, one cannot easily
///        copy the entire solver. Therefore, we do a workaround to support push and pop.
///        However, this workaround is expensive and we can safe some computation time if we
///        skip it when we will not need it.
  virtual void startIncrementalSession(const vector<int> &vars_to_keep,
                                       bool use_push = true);

  virtual void addVarsToKeep(const vector<int> &vars_to_keep);

  virtual void addVarToKeep(int var_to_keep);

// -------------------------------------------------------------------------------------------
///
/// @brief Deletes the solver instance that is used in the incremental session.
  virtual void clearIncrementalSession();

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new CNF to the current incremental session.
///
/// @pre #startIncrementalSession() must have been called before.
/// @param cnf The CNF to add to the currently open incremental session. If this method is
///        called after solving for the first time, be sure that the passed CNF talks only
///        about variables that have been mentioned in vars_to_keep when calling
///        #startIncrementalSession(). Actually, the implementation in THIS class does not
///        care, but other implementations of the SatSolver interface do. Hence, you should
///        not violate this property to have the solver instances exchangeable.
  virtual void incAddCNF(const CNF &cnf);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new clause to the current incremental session.
///
/// @pre #startIncrementalSession() must have been called before.
/// @param clause The clause (a disjunction of literals) to add (conjunct) to the currently
///        open incremental session. If this method is called after solving for the first
///        time, be sure that the passed clause talks only about variables that have been
///        mentioned in vars_to_keep when calling #startIncrementalSession().
///        Actually, the implementation in THIS class does not
///        care, but other implementations of the SatSolver interface do. Hence, you should
///        not violate this property to have the solver instances exchangeable.
  virtual void incAddClause(const vector<int> &clause);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new unit clause to the current incremental session.
///
/// @pre #startIncrementalSession() must have been called before.
/// @param lit The (one and only) literal of the unit clause to add to the currently
///        open incremental session. If this method is called after solving for the first
///        time, be sure that the passed literal talks about a variables that have been
///        mentioned in vars_to_keep when calling #startIncrementalSession().
///        Actually, the implementation in THIS class does not
///        care, but other implementations of the SatSolver interface do. Hence, you should
///        not violate this property to have the solver instances exchangeable.
  virtual void incAddUnitClause(int lit);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new clause consisting of 2 literals to the current incremental session.
///
/// @pre #startIncrementalSession() must have been called before.
/// @param lit1 The first literal of the clause to add to the currently
///        open incremental session. If this method is called after solving for the first
///        time, be sure that the passed literal talks about a variables that have been
///        mentioned in vars_to_keep when calling #startIncrementalSession().
///        Actually, the implementation in THIS class does not
///        care, but other implementations of the SatSolver interface do. Hence, you should
///        not violate this property to have the solver instances exchangeable.
/// @param lit2 The second literal of the clause to add to the currently open incremental
///        session (must be contained in vars_to_keep as well).
  virtual void incAdd2LitClause(int lit1, int lit2);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new clause consisting of 3 literals to the current incremental session.
///
/// @pre #startIncrementalSession() must have been called before.
/// @param lit1 The first literal of the clause to add to the currently
///        open incremental session. If this method is called after solving for the first
///        time, be sure that the passed literal talks about a variables that have been
///        mentioned in vars_to_keep when calling #startIncrementalSession().
///        Actually, the implementation in THIS class does not
///        care, but other implementations of the SatSolver interface do. Hence, you should
///        not violate this property to have the solver instances exchangeable.
/// @param lit2 The second literal of the clause to add to the currently open incremental
///        session (must be contained in vars_to_keep as well).
/// @param lit3 The third literal of the clause to add to the currently open incremental
///        session (must be contained in vars_to_keep as well).
  virtual void incAdd3LitClause(int lit1, int lit2, int lit3);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new clause consisting of 4 literals to the current incremental session.
///
/// @pre #startIncrementalSession() must have been called before.
/// @param lit1 The first literal of the clause to add to the currently
///        open incremental session. If this method is called after solving for the first
///        time, be sure that the passed literal talks about a variables that have been
///        mentioned in vars_to_keep when calling #startIncrementalSession().
///        Actually, the implementation in THIS class does not
///        care, but other implementations of the SatSolver interface do. Hence, you should
///        not violate this property to have the solver instances exchangeable.
/// @param lit2 The second literal of the clause to add to the currently open incremental
///        session (must be contained in vars_to_keep as well).
/// @param lit3 The third literal of the clause to add to the currently open incremental
///        session (must be contained in vars_to_keep as well).
/// @param lit4 The fourth literal of the clause to add to the currently open incremental
///        session (must be contained in vars_to_keep as well).
  virtual void incAdd4LitClause(int lit1, int lit2, int lit3, int lit4);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds a new cube to the current incremental session.
///
/// That is, all literals of the cube are added as unit clauses to the current incremental
/// session. For instance, if the cube [2, -4, 8] is passed as argument, then this method
/// adds the three unit clauses [2], [-4], [8].
///
/// @pre #startIncrementalSession() must have been called before.
/// @param cube The cube (a conjunction of literals) to add (conjunct) to the currently
///        open incremental session. All literals of this cube will be added as unit clauses.
///        If this method is called after solving for the first time, be sure that the passed
///        clause talks only about variables that have been mentioned in vars_to_keep when
///        calling #startIncrementalSession().
///        Actually, the implementation in THIS class does not
///        care, but other implementations of the SatSolver interface do. Hence, you should
///        not violate this property to have the solver instances exchangeable.
  virtual void incAddCube(const vector<int> &cube);

// -------------------------------------------------------------------------------------------
///
/// @brief Adds the negation of a given cube (which is a clause) to the incremental session.
///
/// For instance, if the cube [2, -4, 8] is passed as argument, then this method adds the
/// clauses [-2, 4, -8] to the current incremental session.
///
/// @pre #startIncrementalSession() must have been called before.
/// @param cube The cube (a conjunction of literals) to negate and add (conjunct) to the
///        currently open incremental session.
///        If this method is called after solving for the first time, be sure that the passed
///        cube talks only about variables that have been mentioned in vars_to_keep when
///        calling #startIncrementalSession().
///        Actually, the implementation in THIS class does not
///        care, but other implementations of the SatSolver interface do. Hence, you should
///        not violate this property to have the solver instances exchangeable.
  virtual void incAddNegCubeAsClause(const vector<int> &cube);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a the CNF in the incremental session is satisfiable.
///
/// @pre #startIncrementalSession() must have been called before.
/// @return True in case of satisfiability, false otherwise.
  virtual bool incIsSat();

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a the CNF in the incremental session is satisfiable under assumptions.
///
/// This method checks if the CNF in the incremental session is satisfiable given that all
/// literals passed in the vector 'assumptions' are true. The assumptions are not persistently
/// added to the CNF of the incremental session.
///
/// @pre #startIncrementalSession() must have been called before.
/// @param assumptions A vector of literals. This method then checks if the CNF of the
///        incremental session is satisfiable given that all literals passed in this vector
///        are true. The assumptions are not persistently added to the CNF.
///        The assumptions must be a subset of the vars_to_keep passed to
///        #startIncrementalSession().
/// @return True in case of satisfiability, false otherwise.
  virtual bool incIsSat(const vector<int> &assumptions);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks the CNF in the incremental session and computes a model or unsat core.
///
/// This method checks if the CNF of the incremental session is satisfiable given that all
/// literals passed in the vector 'assumptions' is true. If this is the case, then a
/// satisfying assignment will be written into model_or_core. You can specify which variables
/// you want to have in the satisfying assignment using the vars_of_interest vector. The
/// reason is that your CNF may contain thousands of temporary variables stemming from some
/// Tseitin encoding, but usually one in only interested in the value of a few variables. In
/// case of unsatisfiability, an unsatisfiable core is stored in model_or_core. The
/// unsatisfiable core is a subset of the literals passed in 'assumptions' such that the CNF
/// is still unsatisfiable when these literals hold.
///
/// @param assumptions A vector of literals. This method then checks if the CNF of the
///        incremental session is satisfiable given that all literals passed in this vector
///        are true. The assumptions are not persistently added to the CNF.
///        The assumptions must be a subset of the vars_to_keep passed to
///        #startIncrementalSession().
/// @param vars_of_interest The variables for which you want to have a value in case of
///        satisfiability.
///        vars_of_interest must be a subset of the vars_to_keep passed to
///        #startIncrementalSession().
/// @param model_or_core An empty vector. Depending on the outcome of the call, either a
///        satisfying assignment (a cube over the variables passed in vars_of_interest) or an
///        unsatisfiable core (a subset of the literals passed in 'assumptions') will be
///        written into this vector.
/// @return True in case of satisfiability (of the CNF of the incremental session in
///         conjunction conjuncted with all assumptions), false otherwise.
  virtual bool incIsSatModelOrCore(const vector<int> &assumptions,
                                   const vector<int> &vars_of_interest,
                                   vector<int> &model_or_core);

// -------------------------------------------------------------------------------------------
///
/// @brief Computes a satisfying assignment or core using additional assumptions.
///
/// In contrast to the previous method, this one allows to define two sets of assumptions.
/// The core_assumptions are used as basis for extracting an unsatisfiable core in case of
/// unsatisfiability. The more_assumptions are assumed but not considered for the computation
/// of unsatisfiable cores. This is convenient to avoid calls to #incPush() and #incPop()
/// when temporarily working under assumptions that should not be minimized for the
/// unsatisfiable core.
///
/// @param core_assumptions A vector of literals. This method then checks if the CNF of the
///        incremental session is satisfiable given that all literals passed in this vector
///        and all literals passed in more_assumptions are true. However, for computing the
///        the unsatisfiable core, only the core_assumptions will be minimized and the
///        more_assumptions stay as they are. That is, in case of unsatisfiability,
///        model_or_core will contain a subset X of the core_assumptions such that
///        X & more_assumptions & incremental_cnf is still unsatisfiable.
///        core_assumptions must be a subset of the vars_to_keep passed to
///        #startIncrementalSession().
/// @param more_assumptions A vector of assumptions that are assumed, but not minimized when
///        an unsatisfiable core is computed. That is, in case of unsatisfiability,
///        model_or_core will contain a subset X of the core_assumptions such that
///        X & more_assumptions & incremental_cnf is still unsatisfiable.
///        more_assumptions must be a subset of the vars_to_keep passed to
///        #startIncrementalSession().
/// @param vars_of_interest The variables for which you want to have a value in case of
///        satisfiability.
///        vars_of_interest must be a subset of the vars_to_keep passed to
///        #startIncrementalSession().
/// @param model_or_core An empty vector. Depending on the outcome of the call, either a
///        satisfying assignment (a cube over the variables passed in vars_of_interest) or an
///        unsatisfiable core (a subset of the literals passed in 'core_assumptions') will be
///        written into this vector.
/// @return True in case of satisfiability (of the CNF of the incremental session in
///         conjunction conjuncted with all assumptions), false otherwise.
  virtual bool incIsSatModelOrCore(const vector<int> &core_assumptions,
                                   const vector<int> &more_assumptions,
                                   const vector<int> &vars_of_interest,
                                   vector<int> &model_or_core);

// -------------------------------------------------------------------------------------------
///
/// @brief Stores the current state of the incremental session on a stack.
///
/// The state can be restored later by calling #incPop().
/// MiniSat does not support push and pop natively. Also, MiniSat does not allow to copy a
/// a solver instance. Hence, we do a workaround: we create a new solver instance and feed it
/// with the CNF that has been fed into the original solver instance. Clauses learned by the
/// original solver are not transferred. For this to work, we track all clauses that are
/// added to the original solver. This means some computational overhead. We can save this
/// overhead if we tell #startIncrementalSession() that #incPush() will never be called.
///
/// @todo Try to improve this workaround with activation variables or by extracting the
///       learned clauses from the original solver instance or by implementing a
///       copy-function for MiniSat solver instances.
  virtual void incPush();

// -------------------------------------------------------------------------------------------
///
/// @brief Restores the incremental session back to the point where #incPush() was called.
  virtual void incPop();

protected:

// -------------------------------------------------------------------------------------------
///
/// @brief A stack of incremental session contexts.
///
/// The incremental session is store on a stack to support #incPush() and #incPop(). The
/// current session is always at the top of the stack (the end of the list).
  list<Minisat::Solver*> incr_stack_;

// -------------------------------------------------------------------------------------------
///
/// @brief A CNFs that are stored in the incremental session contexts.
///
/// This is used for our workaround to support #incPush() and #incPop(). The
/// current CNF is always at the top of the stack (the end of the list).
  list<CNF*> incr_stack_cnfs_;

// -------------------------------------------------------------------------------------------
///
/// @brief A flag indicating if #incPush() and #incPop() are ever going to be used.
///
/// If this flag is set to false, we do not track the CNFs in the incremental solver
/// instances in #incr_stack_cnfs_.
  bool use_push_;

// -------------------------------------------------------------------------------------------
///
/// @typedef list<Minisat::Solver*>::const_iterator StackConstIter
/// @brief A type for a const-iterator over the incremental session stack.
  typedef list<Minisat::Solver*>::const_iterator StackConstIter;

// -------------------------------------------------------------------------------------------
///
/// @typedef list<Minisat::Solver*>::iterator StackIter
/// @brief A type for an iterator over the incremental session stack.
  typedef list<Minisat::Solver*>::iterator StackIter;

// -------------------------------------------------------------------------------------------
///
/// @typedef list<CNF*>::const_iterator StackCNFConstIter
/// @brief A type for a const-iterator over the CNFs of the incremental sessions.
  typedef list<CNF*>::const_iterator StackCNFConstIter;

// -------------------------------------------------------------------------------------------
///
/// @typedef list<CNF*>::iterator StackCNFIter;
/// @brief A type for an iterator over the CNFs of the incremental sessions.
  typedef list<CNF*>::iterator StackCNFIter;


private:

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  MiniSatApi(const MiniSatApi &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  MiniSatApi& operator=(const MiniSatApi &other);

};

#endif // MiniSatApi_H__
