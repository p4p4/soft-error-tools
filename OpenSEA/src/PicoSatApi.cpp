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
/// @file PicoSatApi.cpp
/// @brief Contains the definition of the class PicoSatApi.
// -------------------------------------------------------------------------------------------

#include "PicoSatApi.h"
#include "CNF.h"
#include "Utils.h"
extern "C" {
  #include "picosat.h"
}

// -------------------------------------------------------------------------------------------
PicoSatApi::PicoSatApi(bool rand_models, bool min_cores) :
            SatSolver(rand_models, min_cores),
            incr_(NULL)
{
  // nothing to do
}

// -------------------------------------------------------------------------------------------
PicoSatApi::~PicoSatApi()
{
  clearIncrementalSession();
}

// -------------------------------------------------------------------------------------------
bool PicoSatApi::isSat(const CNF &cnf)
{
  PicoSAT *solver = picosat_init();
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      picosat_add(solver, (*it)[lit_cnt]);
    picosat_add(solver, 0);
  }
  int res = picosat_sat(solver, -1);
  picosat_reset(solver);
  if(res == PICOSAT_SATISFIABLE)
    return true;
  else if(res == PICOSAT_UNSATISFIABLE)
    return false;
  MASSERT(false, "Strange result from picosat.");
  return false;
}

// -------------------------------------------------------------------------------------------
bool PicoSatApi::isSatModelOrCore(const CNF &cnf,
                                  const vector<int> &assumptions,
                                  const vector<int> &vars_of_interest,
                                  vector<int> &model_or_core)
{

  PicoSAT *solver = picosat_init();
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      picosat_add(solver, (*it)[lit_cnt]);
    picosat_add(solver, 0);
  }
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    picosat_assume(solver, assumptions[ass_cnt]);

  int res = picosat_sat(solver, -1);
  if(res == PICOSAT_SATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(picosat_deref(solver, vars_of_interest[var_cnt]) > 0)
        model_or_core.push_back(vars_of_interest[var_cnt]);
      else
        model_or_core.push_back(-vars_of_interest[var_cnt]);
    }
    if(rand_models_)
      randModel(solver, assumptions, model_or_core);
    picosat_reset(solver);
    return true;
  }
  else if(res == PICOSAT_UNSATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(assumptions.size());
    for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    {
      if(picosat_failed_assumption(solver, assumptions[ass_cnt]))
        model_or_core.push_back(assumptions[ass_cnt]);
    }
    picosat_reset(solver);
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
  MASSERT(false, "Strange result from picosat.");
  return false;
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::startIncrementalSession(const vector<int> &vars_to_keep,
                                         bool use_push)
{
  if(incr_ != NULL)
    clearIncrementalSession();
  incr_ = picosat_init();
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::clearIncrementalSession()
{
  if(incr_ != NULL)
  {
    picosat_reset(incr_);
    incr_ = NULL;
  }
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incAddCNF(const CNF &cnf)
{
  MASSERT(incr_ != NULL, "No open session.");
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      picosat_add(incr_, (*it)[lit_cnt]);
    picosat_add(incr_, 0);
  }
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incAddClause(const vector<int> &clause)
{
  MASSERT(incr_ != NULL, "No open session.");
  for(size_t lit_cnt = 0; lit_cnt < clause.size(); ++lit_cnt)
    picosat_add(incr_, clause[lit_cnt]);
  picosat_add(incr_, 0);
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incAddUnitClause(int lit)
{
  MASSERT(incr_ != NULL, "No open session.");
  picosat_add(incr_, lit);
  picosat_add(incr_, 0);
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incAdd2LitClause(int lit1, int lit2)
{
  MASSERT(incr_ != NULL, "No open session.");
  picosat_add(incr_, lit1);
  picosat_add(incr_, lit2);
  picosat_add(incr_, 0);
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incAdd3LitClause(int lit1, int lit2, int lit3)
{
  MASSERT(incr_ != NULL, "No open session.");
  picosat_add(incr_, lit1);
  picosat_add(incr_, lit2);
  picosat_add(incr_, lit3);
  picosat_add(incr_, 0);
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incAdd4LitClause(int lit1, int lit2, int lit3, int lit4)
{
  MASSERT(incr_ != NULL, "No open session.");
  picosat_add(incr_, lit1);
  picosat_add(incr_, lit2);
  picosat_add(incr_, lit3);
  picosat_add(incr_, lit4);
  picosat_add(incr_, 0);
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incAddCube(const vector<int> &cube)
{
  MASSERT(incr_ != NULL, "No open session.");
  for(size_t cnt = 0; cnt < cube.size(); ++cnt)
  {
    picosat_add(incr_, cube[cnt]);
    picosat_add(incr_, 0);
  }
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incAddNegCubeAsClause(const vector<int> &cube)
{
  MASSERT(incr_ != NULL, "No open session.");
  for(size_t cnt = 0; cnt < cube.size(); ++cnt)
    picosat_add(incr_, -cube[cnt]);
  picosat_add(incr_, 0);
}

// -------------------------------------------------------------------------------------------
bool PicoSatApi::incIsSat()
{
  MASSERT(incr_ != NULL, "No open session.");
  int res = picosat_sat(incr_, -1);
  if(res == PICOSAT_SATISFIABLE)
    return true;
  else if(res == PICOSAT_UNSATISFIABLE)
    return false;
  MASSERT(false, "Strange result from picosat.");
  return false;
}

// -------------------------------------------------------------------------------------------
bool PicoSatApi::incIsSat(const vector<int> &assumptions)
{
  MASSERT(incr_ != NULL, "No open session.");
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    picosat_assume(incr_, assumptions[ass_cnt]);
  int res = picosat_sat(incr_, -1);
  if(res == PICOSAT_SATISFIABLE)
    return true;
  else if(res == PICOSAT_UNSATISFIABLE)
    return false;
  MASSERT(false, "Strange result from picosat.");
  return false;
}

// -------------------------------------------------------------------------------------------
bool PicoSatApi::incIsSatModelOrCore(const vector<int> &assumptions,
                                     const vector<int> &vars_of_interest,
                                     vector<int> &model_or_core)
{
  MASSERT(incr_ != NULL, "No open session.");
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    picosat_assume(incr_, assumptions[ass_cnt]);

  int res = picosat_sat(incr_, -1);
  if(res == PICOSAT_SATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(picosat_deref(incr_, vars_of_interest[var_cnt]) > 0)
        model_or_core.push_back(vars_of_interest[var_cnt]);
      else
        model_or_core.push_back(-vars_of_interest[var_cnt]);
    }
    if(rand_models_)
      randModel(incr_, assumptions, model_or_core);
    return true;
  }
  else if(res == PICOSAT_UNSATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(assumptions.size());
    for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    {
      if(picosat_failed_assumption(incr_, assumptions[ass_cnt]))
        model_or_core.push_back(assumptions[ass_cnt]);
    }
    if(min_cores_)
    {
      // TODO: maybe use picosat_mus_assumptions here
      vector<int> orig_core(model_or_core);
      for(size_t lit_cnt = 0; lit_cnt < orig_core.size(); ++lit_cnt)
      {
        vector<int> tmp(model_or_core);
        bool found = Utils::remove(tmp, orig_core[lit_cnt]);
        if(found)
        {
          for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            picosat_assume(incr_, tmp[ass_cnt]);
          if(picosat_sat(incr_, -1) == PICOSAT_UNSATISFIABLE)
          {
            model_or_core.clear();
            for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            {
              if(picosat_failed_assumption(incr_, tmp[ass_cnt]))
                model_or_core.push_back(tmp[ass_cnt]);
            }
          }
        }
      }
    }
    return false;
  }
  MASSERT(false, "Strange result from picosat.");
  return false;
}

// -------------------------------------------------------------------------------------------
bool PicoSatApi::incIsSatModelOrCore(const vector<int> &core_assumptions,
                                     const vector<int> &more_assumptions,
                                     const vector<int> &vars_of_interest,
                                     vector<int> &model_or_core)
{
  MASSERT(incr_ != NULL, "No open session.");
  for(size_t ass_cnt = 0; ass_cnt < more_assumptions.size(); ++ass_cnt)
    picosat_assume(incr_, more_assumptions[ass_cnt]);
  for(size_t ass_cnt = 0; ass_cnt < core_assumptions.size(); ++ass_cnt)
    picosat_assume(incr_, core_assumptions[ass_cnt]);

  int res = picosat_sat(incr_, -1);
  if(res == PICOSAT_SATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(picosat_deref(incr_, vars_of_interest[var_cnt]) > 0)
        model_or_core.push_back(vars_of_interest[var_cnt]);
      else
        model_or_core.push_back(-vars_of_interest[var_cnt]);
    }
    if(rand_models_)
    {
      vector<int> all_ass;
      all_ass.reserve(core_assumptions.size() + more_assumptions.size());
      all_ass.insert(all_ass.end(), core_assumptions.begin(), core_assumptions.end());
      all_ass.insert(all_ass.end(), more_assumptions.begin(), more_assumptions.end());
      randModel(incr_, all_ass, model_or_core);
    }
    return true;
  }
  else if(res == PICOSAT_UNSATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(core_assumptions.size());
    for(size_t ass_cnt = 0; ass_cnt < core_assumptions.size(); ++ass_cnt)
    {
      if(picosat_failed_assumption(incr_, core_assumptions[ass_cnt]))
        model_or_core.push_back(core_assumptions[ass_cnt]);
    }
    if(min_cores_)
    {
      // TODO: maybe use picosat_mus_assumptions here
      vector<int> orig_core(model_or_core);
      for(size_t lit_cnt = 0; lit_cnt < orig_core.size(); ++lit_cnt)
      {
        vector<int> tmp(model_or_core);
        bool found = Utils::remove(tmp, orig_core[lit_cnt]);
        if(found)
        {
          for(size_t ass_cnt = 0; ass_cnt < more_assumptions.size(); ++ass_cnt)
            picosat_assume(incr_, more_assumptions[ass_cnt]);
          for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            picosat_assume(incr_, tmp[ass_cnt]);
          if(picosat_sat(incr_, -1) == PICOSAT_UNSATISFIABLE)
          {
            model_or_core.clear();
            for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            {
              if(picosat_failed_assumption(incr_, tmp[ass_cnt]))
                model_or_core.push_back(tmp[ass_cnt]);
            }
          }
        }
      }
    }
    return false;
  }
  MASSERT(false, "Strange result from picosat.");
  return false;
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incPush()
{
  MASSERT(incr_ != NULL, "No open session.");
  picosat_push(incr_);
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::incPop()
{
  MASSERT(incr_ != NULL, "No open session.");
  picosat_pop(incr_);
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::randModel(PicoSAT *solver, const vector<int> &ass, vector<int> &model)
{
  MASSERT(solver != NULL, "No solver given.");
  for(size_t cnt = 0; cnt < model.size(); ++cnt)
  {
    if(rand() % 2 == 0)
    {
      vector<int> mutated_model(model);
      mutated_model[cnt] = -mutated_model[cnt];
      for(size_t ass_cnt = 0; ass_cnt < ass.size(); ++ass_cnt)
        picosat_assume(solver, ass[ass_cnt]);
      for(size_t m_cnt = 0; m_cnt < mutated_model.size(); ++m_cnt)
        picosat_assume(solver, mutated_model[m_cnt]);
      if(picosat_sat(solver, -1) == PICOSAT_SATISFIABLE)
        model = mutated_model;
    }
  }
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::addVarsToKeep(const vector<int>& vars_to_keep)
{
	// nothing to do for picosat
}

// -------------------------------------------------------------------------------------------
void PicoSatApi::addVarToKeep(int var_to_keep)
{
	// nothing to do for picosat
}
