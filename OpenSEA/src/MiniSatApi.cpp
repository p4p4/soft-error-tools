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
/// @file MiniSatApi.cpp
/// @brief Contains the definition of the class MiniSatApi.
// -------------------------------------------------------------------------------------------

#include "MiniSatApi.h"
#include "core/Solver.h" // This is the MiniSat header file
#include "Utils.h"
#include "CNF.h"

using namespace Minisat;

// -------------------------------------------------------------------------------------------
MiniSatApi::MiniSatApi(bool rand_models, bool min_cores) :
            SatSolver(rand_models, min_cores),
            use_push_(true)
{
  // nothing to do
}

// -------------------------------------------------------------------------------------------
MiniSatApi::~MiniSatApi()
{
  clearIncrementalSession();
}

// -------------------------------------------------------------------------------------------
/// @brief Transforms a CNF literal to the corresponding literal used by MiniSat.
///
/// In our CNF we use -7 to denote the negation of the literal 7. MiniSat toggles the least
/// significant bit to encode negation. Hence we need utility functions to transform literals
/// between the two notations. This function is the inverse function of #m2c().
///
/// @param s The solver instance in which the literal should be used.
/// @param lit The CNF literal to transform into the corresponding literal used by MiniSat.
/// @return The corresponding literal used by MiniSat.
inline Lit c2m(Solver &s, int lit)
{
  int var = lit < 0 ? -lit : lit;
  while(var >= s.nVars())
    s.newVar();
  int minisat_lit = var + var;
  if(lit < 0)
    minisat_lit += 1;
  Lit p;
  p.x = minisat_lit;
  return p;
}

// -------------------------------------------------------------------------------------------
// @brief Transforms a MiniSat literal to the corresponding literal used in our CNFs.
///
/// In our CNF we use -7 to denote the negation of the literal 7. MiniSat toggles the least
/// significant bit to encode negation. Hence we need utility functions to transform literals
/// between the two notations. This function is the inverse function of #c2m().
///
/// @param lit The MiniSat literal to transform.
/// @return The corresponding CNF-literal.
inline int m2c(Lit lit)
{
  int l = lit.x >> 1;
  if(lit.x & 1)
    l = -l;
  return l;
}

// -------------------------------------------------------------------------------------------
///
/// @brief A helper to randomize a satisfying assignment after it has been computed.
///
/// This is done by flipping values of variables randomly and then checking if the modified
/// assignment is still satisfies the formula. This is expensive. However, changing the
/// decision heuristic to a random one seems even more expensive in first experiments.
/// This method is not declared in the header file because this would require to include all
/// the MiniSat internals in the header file.
///
/// @param solver The solver containing the CNF for which the satisfying assignment has been
///        computed.
/// @param assumptions Additional assumptions under which the CNF should be solved. This can
///        be an empty vector if there are no assumptions.
/// @param model The satisfying assignment that has been found and should now be randomized.
///        It will be modified in place, i.e., this vector will contain the randomized model
///        after this method is done.
void randModel(Solver &solver, const vec<Lit> &assumptions, vector<int> &model)
{
  vec<Lit> ass_mod(assumptions.size() + model.size());
  for(int ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    ass_mod[ass_cnt] = assumptions[ass_cnt];

  for(size_t cnt = 0; cnt < model.size(); ++cnt)
  {
    if(rand() % 2 == 0)
    {
      vector<int> mutated_model(model);
      mutated_model[cnt] = -mutated_model[cnt];
      for(size_t mod_cnt = 0; mod_cnt < mutated_model.size(); ++mod_cnt)
        ass_mod[mod_cnt + assumptions.size()] = c2m(solver, mutated_model[mod_cnt]);
      if(solver.solve(ass_mod))
        model = mutated_model;
    }
  }
}

// -------------------------------------------------------------------------------------------
bool MiniSatApi::isSat(const CNF &cnf)
{
  Solver solver;
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    vec<Lit> clause(it->size());
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      clause[lit_cnt] = c2m(solver, (*it)[lit_cnt]);
    solver.addClause(clause);
  }
  return solver.solve();
}

// -------------------------------------------------------------------------------------------
bool MiniSatApi::isSatModelOrCore(const CNF &cnf,
                                  const vector<int> &assumptions,
                                  const vector<int> &vars_of_interest,
                                  vector<int> &model_or_core)
{

  Solver solver;
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    vec<Lit> clause(it->size());
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      clause[lit_cnt] = c2m(solver, (*it)[lit_cnt]);
    solver.addClause(clause);
  }

  vec<Lit> ass(assumptions.size());
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    ass[ass_cnt] = c2m(solver, assumptions[ass_cnt]);

  bool sat = solver.solve(ass);
  if(sat)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(solver.modelValue(c2m(solver, vars_of_interest[var_cnt])) == l_True)
        model_or_core.push_back(vars_of_interest[var_cnt]);
      else
        model_or_core.push_back(-vars_of_interest[var_cnt]);
    }
    if(rand_models_)
      randModel(solver, ass, model_or_core);
    return true;
  }
  else
  {
    model_or_core.clear();
    model_or_core.reserve(solver.conflict.size());
    for(int ass_cnt = 0; ass_cnt < solver.conflict.size(); ++ass_cnt)
      model_or_core.push_back(m2c(solver.conflict[ass_cnt]));
    if(min_cores_)
    {
      vector<int> orig_core(model_or_core);
      for(size_t lit_cnt = 0; lit_cnt < orig_core.size(); ++lit_cnt)
      {
        vector<int> tmp(model_or_core);
        Utils::remove(tmp, orig_core[lit_cnt]);
        // TODO: make incremental:
        CNF min(cnf);
        for(size_t tmp_cnt = 0; tmp_cnt < tmp.size(); ++tmp_cnt)
          min.add1LitClause(tmp[tmp_cnt]);
        if(!isSat(min))
          model_or_core = tmp;
      }
    }
    return false;
  }
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::startIncrementalSession(const vector<int> &vars_to_keep,
                                         bool use_push)
{
  if(!incr_stack_.empty())
    clearIncrementalSession();
  use_push_ = use_push;
  incr_stack_.push_back(new Solver());
  if(use_push_)
    incr_stack_cnfs_.push_back(new CNF());
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::clearIncrementalSession()
{
  for(StackConstIter it = incr_stack_.begin(); it != incr_stack_.end(); ++it)
    delete *it;
  incr_stack_.clear();
  for(StackCNFIter it = incr_stack_cnfs_.begin(); it != incr_stack_cnfs_.end(); ++it)
    delete *it;
  incr_stack_cnfs_.clear();
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incAddCNF(const CNF &cnf)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  if(use_push_)
    incr_stack_cnfs_.back()->addCNF(cnf);
  Solver *solver = incr_stack_.back();
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    vec<Lit> clause(it->size());
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      clause[lit_cnt] = c2m(*solver, (*it)[lit_cnt]);
    solver->addClause(clause);
  }
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incAddClause(const vector<int> &clause)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  if(use_push_)
    incr_stack_cnfs_.back()->addClause(clause);
  Solver *solver = incr_stack_.back();
  vec<Lit> m_clause(clause.size());
  for(size_t lit_cnt = 0; lit_cnt < clause.size(); ++lit_cnt)
    m_clause[lit_cnt] = c2m(*solver, clause[lit_cnt]);
  solver->addClause(m_clause);
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incAddUnitClause(int lit)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  if(use_push_)
    incr_stack_cnfs_.back()->add1LitClause(lit);
  Solver *solver = incr_stack_.back();
  solver->addClause(c2m(*solver, lit));
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incAdd2LitClause(int lit1, int lit2)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  if(use_push_)
    incr_stack_cnfs_.back()->add2LitClause(lit1, lit2);
  Solver *solver = incr_stack_.back();
  solver->addClause(c2m(*solver, lit1), c2m(*solver, lit2));
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incAdd3LitClause(int lit1, int lit2, int lit3)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  if(use_push_)
    incr_stack_cnfs_.back()->add3LitClause(lit1, lit2, lit3);
  Solver *solver = incr_stack_.back();
  solver->addClause(c2m(*solver, lit1), c2m(*solver, lit2), c2m(*solver, lit3));
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incAdd4LitClause(int lit1, int lit2, int lit3, int lit4)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  if(use_push_)
    incr_stack_cnfs_.back()->add4LitClause(lit1, lit2, lit3, lit4);
  Solver *solver = incr_stack_.back();
  vec<Lit> m_clause(4);
  m_clause[0] = c2m(*solver, lit1);
  m_clause[1] = c2m(*solver, lit2);
  m_clause[2] = c2m(*solver, lit3);
  m_clause[3] = c2m(*solver, lit4);
  solver->addClause(m_clause);
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incAddCube(const vector<int> &cube)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  if(use_push_)
    incr_stack_cnfs_.back()->addCube(cube);
  Solver *solver = incr_stack_.back();
  for(size_t cnt = 0; cnt < cube.size(); ++cnt)
    solver->addClause(c2m(*solver, cube[cnt]));
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incAddNegCubeAsClause(const vector<int> &cube)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  if(use_push_)
    incr_stack_cnfs_.back()->addNegCubeAsClause(cube);
  Solver *solver = incr_stack_.back();
  vec<Lit> m_clause(cube.size());
  for(size_t lit_cnt = 0; lit_cnt < cube.size(); ++lit_cnt)
    m_clause[lit_cnt] = c2m(*solver, -cube[lit_cnt]);
  solver->addClause(m_clause);
}

// -------------------------------------------------------------------------------------------
bool MiniSatApi::incIsSat()
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  return incr_stack_.back()->solve();
}

// -------------------------------------------------------------------------------------------
bool MiniSatApi::incIsSat(const vector<int> &assumptions)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  Solver *solver = incr_stack_.back();
  vec<Lit> ass(assumptions.size());
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    ass[ass_cnt] = c2m(*solver, assumptions[ass_cnt]);
  return solver->solve(ass);
}

// -------------------------------------------------------------------------------------------
bool MiniSatApi::incIsSatModelOrCore(const vector<int> &assumptions,
                                     const vector<int> &vars_of_interest,
                                     vector<int> &model_or_core)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  Solver *solver = incr_stack_.back();
  vec<Lit> ass(assumptions.size());
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    ass[ass_cnt] = c2m(*solver, assumptions[ass_cnt]);
  bool sat = solver->solve(ass);
  if(sat)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(solver->modelValue(c2m(*solver, vars_of_interest[var_cnt])) == l_True)
        model_or_core.push_back(vars_of_interest[var_cnt]);
      else
        model_or_core.push_back(-vars_of_interest[var_cnt]);
    }
    if(rand_models_)
      randModel(*solver, ass, model_or_core);
    return true;
  }
  else
  {
    model_or_core.clear();
    model_or_core.reserve(solver->conflict.size());
    for(int ass_cnt = 0; ass_cnt < solver->conflict.size(); ++ass_cnt)
      model_or_core.push_back(-m2c(solver->conflict[ass_cnt]));
    if(min_cores_)
    {
      vector<int> orig_core(model_or_core);
      for(size_t lit_cnt = 0; lit_cnt < orig_core.size(); ++lit_cnt)
      {
        vector<int> tmp(model_or_core);
        bool found = Utils::remove(tmp, orig_core[lit_cnt]);
        if(found)
        {
          vec<Lit> smaller_ass(tmp.size());
          for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            smaller_ass[ass_cnt] = c2m(*solver, tmp[ass_cnt]);
          if(!solver->solve(smaller_ass))
          {
            model_or_core.clear();
            model_or_core.reserve(solver->conflict.size());
            for(int ass_cnt = 0; ass_cnt < solver->conflict.size(); ++ass_cnt)
              model_or_core.push_back(-m2c(solver->conflict[ass_cnt]));
          }
        }
      }
    }
    return false;
  }
}

// -------------------------------------------------------------------------------------------
bool MiniSatApi::incIsSatModelOrCore(const vector<int> &core_assumptions,
                                     const vector<int> &more_assumptions,
                                     const vector<int> &vars_of_interest,
                                     vector<int> &model_or_core)
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  Solver *solver = incr_stack_.back();
  vec<Lit> ass(core_assumptions.size() + more_assumptions.size());
  for(size_t ass_cnt = 0; ass_cnt < more_assumptions.size(); ++ass_cnt)
    ass[ass_cnt] = c2m(*solver, more_assumptions[ass_cnt]);
  for(size_t ass_cnt = 0; ass_cnt < core_assumptions.size(); ++ass_cnt)
    ass[ass_cnt + more_assumptions.size()] = c2m(*solver, core_assumptions[ass_cnt]);
  bool sat = solver->solve(ass);
  if(sat)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(solver->modelValue(c2m(*solver, vars_of_interest[var_cnt])) == l_True)
        model_or_core.push_back(vars_of_interest[var_cnt]);
      else
        model_or_core.push_back(-vars_of_interest[var_cnt]);
    }
    if(rand_models_)
      randModel(*solver, ass, model_or_core);
    return true;
  }
  else
  {
    set<int> core_assumptions_set;
    for(size_t cnt = 0; cnt < core_assumptions.size(); ++cnt)
      core_assumptions_set.insert(core_assumptions[cnt]);
    model_or_core.clear();
    model_or_core.reserve(solver->conflict.size());
    for(int ass_cnt = 0; ass_cnt < solver->conflict.size(); ++ass_cnt)
    {
      int l = -m2c(solver->conflict[ass_cnt]);
      if(core_assumptions_set.count(l) != 0)
        model_or_core.push_back(l);
    }
    if(min_cores_)
    {
      vector<int> orig_core(model_or_core);
      for(size_t lit_cnt = 0; lit_cnt < orig_core.size(); ++lit_cnt)
      {
        vector<int> tmp(model_or_core);
        bool found = Utils::remove(tmp, orig_core[lit_cnt]);
        if(found)
        {
          vec<Lit> smaller_ass(tmp.size() + more_assumptions.size());
          for(size_t ass_cnt = 0; ass_cnt < more_assumptions.size(); ++ass_cnt)
            smaller_ass[ass_cnt] = c2m(*solver, more_assumptions[ass_cnt]);
          for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            smaller_ass[ass_cnt + more_assumptions.size()] = c2m(*solver, tmp[ass_cnt]);
          if(!solver->solve(smaller_ass))
          {
            model_or_core.clear();
            model_or_core.reserve(solver->conflict.size());
            for(int ass_cnt = 0; ass_cnt < solver->conflict.size(); ++ass_cnt)
            {
              int l = -m2c(solver->conflict[ass_cnt]);
              if(core_assumptions_set.count(l) != 0)
                model_or_core.push_back(l);
            }
          }
        }
      }
    }
    return false;
  }
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incPush()
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  DASSERT(use_push_, "Using Push and Pop must be declared in startIncrementalSession.")

  // TODO: not very efficient:
  Solver *copy = new Solver();
  const list<vector<int> > &clauses = incr_stack_cnfs_.back()->getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    vec<Lit> clause(it->size());
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      clause[lit_cnt] = c2m(*copy, (*it)[lit_cnt]);
    copy->addClause(clause);
  }
  incr_stack_.push_back(copy);
  incr_stack_cnfs_.push_back(new CNF(*incr_stack_cnfs_.back()));
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::incPop()
{
  DASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  delete incr_stack_.back();
  incr_stack_.pop_back();
  delete incr_stack_cnfs_.back();
  incr_stack_cnfs_.pop_back();
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::addVarsToKeep(const vector<int>& vars_to_keep)
{
	// nothing to do for MiniSat
}

// -------------------------------------------------------------------------------------------
void MiniSatApi::addVarToKeep(int var_to_keep)
{
	// nothing to do for MiniSat
}
