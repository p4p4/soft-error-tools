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
/// @file LingelingApi.cpp
/// @brief Contains the definition of the class LingelingApi.
// -------------------------------------------------------------------------------------------

#include "LingelingApi.h"
#include "CNF.h"
#include "Utils.h"
extern "C" {
  #include "lglib.h"
}

// -------------------------------------------------------------------------------------------
LingelingApi::LingelingApi(bool rand_models, bool min_cores) :
              SatSolver(rand_models, min_cores)
{
  // nothing to do
}

// -------------------------------------------------------------------------------------------
LingelingApi::~LingelingApi()
{
  clearIncrementalSession();
}

// -------------------------------------------------------------------------------------------
bool LingelingApi::isSat(const CNF &cnf)
{
  LGL *lgl = lglinit();
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      lgladd(lgl, (*it)[lit_cnt]);
    lgladd(lgl, 0);
  }
  int res = lglsat(lgl);
  lglrelease(lgl);
  if(res == LGL_SATISFIABLE)
    return true;
  else if(res == LGL_UNSATISFIABLE)
    return false;
  MASSERT(false, "Strange result from lingeling.");
  return false;
}

// -------------------------------------------------------------------------------------------
bool LingelingApi::isSatModelOrCore(const CNF &cnf,
                                    const vector<int> &assumptions,
                                    const vector<int> &vars_of_interest,
                                    vector<int> &model_or_core)
{

  LGL *lgl = lglinit();
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      lgladd(lgl, (*it)[lit_cnt]);
    lgladd(lgl, 0);
  }
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    lglassume(lgl, assumptions[ass_cnt]);

  int res = lglsat(lgl);
  if(res == LGL_SATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(lglderef(lgl, vars_of_interest[var_cnt]) > 0)
        model_or_core.push_back(vars_of_interest[var_cnt]);
      else
        model_or_core.push_back(-vars_of_interest[var_cnt]);
    }
    if(rand_models_)
      randModel(lgl, assumptions, model_or_core);
    lglrelease(lgl);
    return true;
  }
  else if(res == LGL_UNSATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(assumptions.size());
    for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    {
      if(lglfailed(lgl, assumptions[ass_cnt]))
        model_or_core.push_back(assumptions[ass_cnt]);
    }
    lglrelease(lgl);
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
  MASSERT(false, "Strange result from lingeling.");
  return false;
}

// -------------------------------------------------------------------------------------------
void LingelingApi::startIncrementalSession(const vector<int> &vars_to_keep,
                                           bool use_push)
{
  if(!incr_stack_.empty())
    clearIncrementalSession();
  incr_stack_.push_back(lglinit());
  for(size_t cnt = 0; cnt < vars_to_keep.size(); ++cnt)
    lglfreeze(incr_stack_.back(), vars_to_keep[cnt]);
  //lglsetopt(incr_stack_.back(), "randec", 1);
  //lglsetopt(incr_stack_.back(), "seed", 42);
}

// -------------------------------------------------------------------------------------------
void LingelingApi::clearIncrementalSession()
{
  for(StackConstIter it = incr_stack_.begin(); it != incr_stack_.end(); ++it)
    lglrelease(*it);
  incr_stack_.clear();
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incAddCNF(const CNF &cnf)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  const list<vector<int> > &clauses = cnf.getClauses();
  for(CNF::ClauseConstIter it = clauses.begin(); it != clauses.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
      lgladd(lgl, (*it)[lit_cnt]);
    lgladd(lgl, 0);
  }
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incAddClause(const vector<int> &clause)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  for(size_t lit_cnt = 0; lit_cnt < clause.size(); ++lit_cnt)
    lgladd(lgl, clause[lit_cnt]);
  lgladd(lgl, 0);
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incAddUnitClause(int lit)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  lgladd(lgl, lit);
  lgladd(lgl, 0);
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incAdd2LitClause(int lit1, int lit2)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  lgladd(lgl, lit1);
  lgladd(lgl, lit2);
  lgladd(lgl, 0);
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incAdd3LitClause(int lit1, int lit2, int lit3)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  lgladd(lgl, lit1);
  lgladd(lgl, lit2);
  lgladd(lgl, lit3);
  lgladd(lgl, 0);
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incAdd4LitClause(int lit1, int lit2, int lit3, int lit4)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  lgladd(lgl, lit1);
  lgladd(lgl, lit2);
  lgladd(lgl, lit3);
  lgladd(lgl, lit4);
  lgladd(lgl, 0);
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incAddCube(const vector<int> &cube)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  for(size_t cnt = 0; cnt < cube.size(); ++cnt)
  {
    lgladd(lgl, cube[cnt]);
    lgladd(lgl, 0);
  }
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incAddNegCubeAsClause(const vector<int> &cube)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  for(size_t cnt = 0; cnt < cube.size(); ++cnt)
    lgladd(lgl, -cube[cnt]);
  lgladd(lgl, 0);
}

// -------------------------------------------------------------------------------------------
bool LingelingApi::incIsSat()
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  int res = lglsat(incr_stack_.back());
  if(res == LGL_SATISFIABLE)
    return true;
  else if(res == LGL_UNSATISFIABLE)
    return false;
  MASSERT(false, "Strange result from lingeling.");
  return false;
}

// -------------------------------------------------------------------------------------------
bool LingelingApi::incIsSat(const vector<int> &assumptions)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    lglassume(lgl, assumptions[ass_cnt]);
  int res = lglsat(lgl);
  if(res == LGL_SATISFIABLE)
    return true;
  else if(res == LGL_UNSATISFIABLE)
    return false;
  MASSERT(false, "Strange result from lingeling.");
  return false;
}

// -------------------------------------------------------------------------------------------
bool LingelingApi::incIsSatModelOrCore(const vector<int> &assumptions,
                                       const vector<int> &vars_of_interest,
                                       vector<int> &model_or_core)
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    lglassume(lgl, assumptions[ass_cnt]);
  int res = lglsat(lgl);
  if(res == LGL_SATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(lglderef(lgl, vars_of_interest[var_cnt]) > 0)
        model_or_core.push_back(vars_of_interest[var_cnt]);
      else
        model_or_core.push_back(-vars_of_interest[var_cnt]);
    }
    if(rand_models_)
      randModel(lgl, assumptions, model_or_core);
    return true;
  }
  else if(res == LGL_UNSATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(assumptions.size());
    for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
    {
      if(lglfailed(lgl, assumptions[ass_cnt]))
        model_or_core.push_back(assumptions[ass_cnt]);
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
          for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            lglassume(lgl, tmp[ass_cnt]);
          if(lglsat(lgl) == LGL_UNSATISFIABLE)
          {
            model_or_core = tmp;
            // using the core again seems to be slower:
            // model_or_core.clear();
            // for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            // {
            //   if(lglfailed(lgl, tmp[ass_cnt]))
            //     model_or_core.push_back(tmp[ass_cnt]);
            // }
          }
        }
      }
    }
    return false;
  }
  MASSERT(false, "Strange result from lingeling.");
  return false;
}

// -------------------------------------------------------------------------------------------
bool LingelingApi::incIsSatModelOrCore(const vector<int> &core_assumptions,
                                       const vector<int> &more_assumptions,
                                       const vector<int> &vars_of_interest,
                                       vector<int> &model_or_core)
{

// was faster with the an old lingeling version, but seems slower now (?):
//  incPush();
//  incAddCube(more_assumptions);
//  bool sat = incIsSatModelOrCore(core_assumptions, vars_of_interest, model_or_core);
//  incPop();
//  return sat;

  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *lgl = incr_stack_.back();
  for(size_t ass_cnt = 0; ass_cnt < core_assumptions.size(); ++ass_cnt)
    lglassume(lgl, core_assumptions[ass_cnt]);
  for(size_t ass_cnt = 0; ass_cnt < more_assumptions.size(); ++ass_cnt)
    lglassume(lgl, more_assumptions[ass_cnt]);
  int res = lglsat(lgl);
  if(res == LGL_SATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(vars_of_interest.size());
    for(size_t var_cnt = 0; var_cnt < vars_of_interest.size(); ++var_cnt)
    {
      if(lglderef(lgl, vars_of_interest[var_cnt]) > 0)
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
      randModel(lgl, all_ass, model_or_core);
    }
    return true;
  }
  else if(res == LGL_UNSATISFIABLE)
  {
    model_or_core.clear();
    model_or_core.reserve(core_assumptions.size());
    for(size_t ass_cnt = 0; ass_cnt < core_assumptions.size(); ++ass_cnt)
    {
      if(lglfailed(lgl, core_assumptions[ass_cnt]))
        model_or_core.push_back(core_assumptions[ass_cnt]);
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
          for(size_t ass_cnt = 0; ass_cnt < more_assumptions.size(); ++ass_cnt)
            lglassume(lgl, more_assumptions[ass_cnt]);
          for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            lglassume(lgl, tmp[ass_cnt]);
          if(lglsat(lgl) == LGL_UNSATISFIABLE)
          {
            model_or_core = tmp;
            // using the core again seems to be slower:
            // model_or_core.clear();
            // for(size_t ass_cnt = 0; ass_cnt < tmp.size(); ++ass_cnt)
            // {
            //   if(lglfailed(lgl, tmp[ass_cnt]))
            //     model_or_core.push_back(tmp[ass_cnt]);
            // }
          }
        }
      }
    }
    return false;
  }
  MASSERT(false, "Strange result from lingeling.");
  return false;
}

// -------------------------------------------------------------------------------------------
void LingelingApi::incPush()
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *base = incr_stack_.back();
  LGL *copy = lglclone(base);
  incr_stack_.push_back(copy);

}

// -------------------------------------------------------------------------------------------
void LingelingApi::incPop()
{
  MASSERT(!incr_stack_.empty() && incr_stack_.back() != NULL, "No open session.");
  LGL *solver_to_delete = incr_stack_.back();
  incr_stack_.pop_back();
  lglrelease(solver_to_delete);
}

// -------------------------------------------------------------------------------------------
void LingelingApi::randModel(LGL* solver, const vector<int> &assumptions, vector<int> &model)
{
  MASSERT(solver != NULL, "No solver given.");
  for(size_t cnt = 0; cnt < model.size(); ++cnt)
  {
    if(rand() % 2 == 0)
    {
      vector<int> mutated_model(model);
      mutated_model[cnt] = -mutated_model[cnt];
      for(size_t ass_cnt = 0; ass_cnt < assumptions.size(); ++ass_cnt)
        lglassume(solver, assumptions[ass_cnt]);
      for(size_t m_cnt = 0; m_cnt < mutated_model.size(); ++m_cnt)
        lglassume(solver, mutated_model[m_cnt]);
      if(lglsat(solver) == LGL_SATISFIABLE)
        model = mutated_model;
    }
  }
}
