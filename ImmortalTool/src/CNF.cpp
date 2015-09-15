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
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file CNF.cpp
/// @brief Contains the definition of the class CNF.
// -------------------------------------------------------------------------------------------

#include "CNF.h"
#include "Logger.h"
#include "Utils.h"


// -------------------------------------------------------------------------------------------
CNF::CNF() : clauses_()
{
  // nothing to do
}

// -------------------------------------------------------------------------------------------
CNF::CNF(const string &filename)
{
  ifstream in_file(filename.c_str());
  MASSERT(!in_file.fail(), "Could not open DIAMCS file '" + filename + "'.");
  size_t nr_of_clauses = 0;

  in_file >> std::ws; // consume all whitespaces

  char c = 0;
  while(in_file.good())
  {
    c = in_file.peek();
    // skipping comments:
    if(c == 'c')
      while(in_file.good() && in_file.get() != '\n');
    else if(c == 'p')
    {
      // parsing the header:
      int max_var = 0;
      string buf1, buf2;
      in_file >> buf1 >> buf2 >> max_var >> nr_of_clauses;
      MASSERT(!in_file.fail(), "Syntax error in header of DIMACS file.");
      MASSERT(buf1 == "p" && buf2 == "cnf", "Syntax error in header of DIMACS file.");
//      MASSERT(max_var <= VarManager::instance().getMaxCNFVar(), "CNF contains unknown var.");
    }
    else
    {
      // parsing a clause:
      vector<int> clause;
      int literal = 0;
      MASSERT(in_file.good(), "Unexpected end-of-file.");
      in_file >> literal;
      while(literal != 0)
      {
        clause.push_back(literal);
        MASSERT(in_file.good(), "Unexpected end-of-file.");
        in_file >> literal;
      }
      clauses_.push_back(clause);
    }
    in_file >> std::ws; // consume all whitespaces
  }
  in_file.close();
  MASSERT(clauses_.size() == nr_of_clauses, "Mismatch in nr of clauses");
}

// -------------------------------------------------------------------------------------------
CNF::CNF(const CNF &other): clauses_(other.clauses_)
{
  // nothing to be done
}

// -------------------------------------------------------------------------------------------
CNF& CNF::operator=(const CNF &other)
{
  clauses_ = other.clauses_;
  return *this;
}

// -------------------------------------------------------------------------------------------
CNF::~CNF()
{
  // nothing to do
}

// -------------------------------------------------------------------------------------------
void CNF::clear()
{
  clauses_.clear();
}

// -------------------------------------------------------------------------------------------
void CNF::addClause(const vector<int> &clause)
{
  clauses_.push_back(clause);
}

// -------------------------------------------------------------------------------------------
void CNF::addNegClauseAsCube(const vector<int> &clause)
{
  for(size_t cnt = 0; cnt < clause.size(); ++cnt)
    add1LitClause(-clause[cnt]);
}

// -------------------------------------------------------------------------------------------
void CNF::addCube(const vector<int> &cube)
{
  for(size_t cnt = 0; cnt < cube.size(); ++cnt)
    add1LitClause(cube[cnt]);
}

// -------------------------------------------------------------------------------------------
void CNF::addNegCubeAsClause(const vector<int> &cube)
{
  vector<int> neg_cube(cube);
  for(size_t cnt = 0; cnt < neg_cube.size(); ++cnt)
    neg_cube[cnt] = -neg_cube[cnt];
  clauses_.push_back(neg_cube);
}

// -------------------------------------------------------------------------------------------
bool CNF::addClauseAndSimplify(const vector<int> &clause)
{
  // search for clauses which are supersets of the new clause:
  size_t init_size = clauses_.size();
  ClauseIter it = clauses_.begin();
  for(; it != clauses_.end();)
  {
    if(Utils::isSubset(clause, *it))
      it = clauses_.erase(it);
    else
      ++it;
  }
  bool simplified = false;
  if(init_size != clauses_.size())
  {
    //L_DBG("CNF size reduction: " << init_size << " --> " << clauses_.size());
    simplified = true;
  }
  clauses_.push_back(clause);
  return simplified;
}

// -------------------------------------------------------------------------------------------
void CNF::add1LitClause(int lit1)
{
  vector<int> clause(1,0);
  clause[0] = lit1;
  clauses_.push_back(clause);
}

// -------------------------------------------------------------------------------------------
void CNF::add2LitClause(int lit1, int lit2)
{
  vector<int> clause(2,0);
  clause[0] = lit1;
  clause[1] = lit2;
  clauses_.push_back(clause);
}

// -------------------------------------------------------------------------------------------
void CNF::add3LitClause(int lit1, int lit2, int lit3)
{
  vector<int> clause(3,0);
  clause[0] = lit1;
  clause[1] = lit2;
  clause[2] = lit3;
  clauses_.push_back(clause);
}

// -------------------------------------------------------------------------------------------
void CNF::add4LitClause(int lit1, int lit2, int lit3, int lit4)
{
  vector<int> clause(4,0);
  clause[0] = lit1;
  clause[1] = lit2;
  clause[2] = lit3;
  clause[3] = lit4;
  clauses_.push_back(clause);
}

// -------------------------------------------------------------------------------------------
vector<int> CNF::removeSmallest()
{
  MASSERT(clauses_.size() != 0, "No clauses there.");
  ClauseIter smallest = clauses_.begin();
  for(ClauseIter it = clauses_.begin(); it != clauses_.end(); ++it)
  {
    if(smallest->size() > it->size())
      smallest = it;
  }
  vector<int> res = *smallest;
  clauses_.erase(smallest);
  return res;
}

// -------------------------------------------------------------------------------------------
vector<int> CNF::removeSomeClause()
{
  vector<int> res = clauses_.back();
  clauses_.pop_back();
  return res;
}

// -------------------------------------------------------------------------------------------
void CNF::addCNF(const CNF& cnf)
{
  clauses_.insert(clauses_.end(), cnf.clauses_.begin(), cnf.clauses_.end());
}

// -------------------------------------------------------------------------------------------
void CNF::swapWith(list<vector<int> > &clauses)
{
  clauses_.swap(clauses);
}

// -------------------------------------------------------------------------------------------
void CNF::swapWith(CNF &other)
{
  clauses_.swap(other.clauses_);
}

// -------------------------------------------------------------------------------------------
size_t CNF::getNrOfClauses() const
{
  return clauses_.size();
}

// -------------------------------------------------------------------------------------------
size_t CNF::getNrOfLits() const
{
  size_t nr_of_lits = 0;
  for(ClauseConstIter it = clauses_.begin(); it != clauses_.end(); ++it)
    nr_of_lits += it->size();
  return nr_of_lits;
}

// -------------------------------------------------------------------------------------------
string CNF::toString() const
{
  ostringstream str;
  for(ClauseConstIter it = clauses_.begin(); it != clauses_.end(); ++it)
  {
    for(size_t cnt = 0; cnt < it->size(); ++cnt)
      str << (*it)[cnt] << " ";
    str << "0" << endl;
  }
  return str.str();
}

// -------------------------------------------------------------------------------------------
void CNF::saveToFile(const string &filename) const
{
  // We have to find out the maximum variable index:
  int max_var = 0;
  for(ClauseConstIter it = clauses_.begin(); it != clauses_.end(); ++it)
  {
    for(size_t cnt = 0; cnt < it->size(); ++cnt)
    {
      int var = (*it)[cnt];
      if(var < 0)
        var = - var;
      if(var > max_var)
        max_var = var;
    }
  }

  ofstream outfile;
  outfile.open(filename.c_str(), ofstream::out | ofstream::trunc);
  MASSERT(!outfile.fail(), "Cannot open file '" + filename + "' for writing.");
  outfile << "p cnf " << max_var << " " << clauses_.size() << endl;
  outfile << toString();
  MASSERT(!outfile.fail(), "Failed to write file '" + filename + "'.");
  outfile.close();
}

// -------------------------------------------------------------------------------------------
const list<vector<int> >& CNF::getClauses() const
{
  return clauses_;
}

// -------------------------------------------------------------------------------------------
void CNF::simplify()
{
  // search for clauses which are supersets of other clauses:
  size_t init_size = clauses_.size();
  for(ClauseIter it1 = clauses_.begin(); it1 != clauses_.end();)
  {
    ClauseIter it2 = it1;
    ++it2;
    bool it1_changed = false;
    for(; it2 != clauses_.end();)
    {
      if(Utils::isSubset(*it1, *it2))
        it2 = clauses_.erase(it2);
      else
        ++it2;
      if(Utils::isSubset(*it2, *it1))
      {
        it1 = clauses_.erase(it1);
        it1_changed = true;
        break;
      }
    }
    if(!it1_changed)
      ++it1;
  }
  if(init_size != clauses_.size())
  {
    L_DBG("CNF size reduction: " << init_size << " --> " << clauses_.size());
  }

}

// -------------------------------------------------------------------------------------------
void CNF::removeDuplicates()
{
  set<vector<int> > clause_set;
  for(ClauseIter it = clauses_.begin(); it != clauses_.end(); ++it)
    clause_set.insert(*it);
  clauses_.clear();
  for(set<vector<int> >::const_iterator it = clause_set.begin(); it != clause_set.end(); ++it)
    clauses_.push_back(*it);
}

// -------------------------------------------------------------------------------------------
bool CNF::isSatBy(const vector<int> &cube) const
{
  for(ClauseConstIter it = clauses_.begin(); it != clauses_.end(); ++it)
  {
    bool satisfied = false;
    for(size_t c1 = 0; c1 < it->size(); ++c1)
    {
      int lit_in_clause = (*it)[c1];
      for(size_t c2 = 0; c2 < cube.size(); ++c2)
      {
        if(lit_in_clause == cube[c2])
        {
          satisfied = true;
          break;
        }
      }
    }
    if(!satisfied)
      return false;
  }
  return true;
}

// -------------------------------------------------------------------------------------------
void CNF::setVarValue(int var, bool value)
{
  for(ClauseIter it = clauses_.begin(); it != clauses_.end();)
  {
    bool clause_removed = false;
    for(size_t lit_cnt = 0; lit_cnt < it->size(); )
    {
      int lit = (*it)[lit_cnt];
      if((lit == var && value == false) || (-lit == var && value == true))
      {
        // the literal is replaced by false, i.e., removed from the clause
        (*it)[lit_cnt] = it->back();
        it->pop_back();
        if(it->empty()) // if a clause gets empty, then the entire CNF is false:
        {
          clauses_.clear();
          clauses_.push_back(vector<int>());
          return;
        }
      }
      else if ((lit == var && value == true) || (-lit == var && value == false))
      {
        // the literal is replaced by true, i.e., the clause is removed
        it = clauses_.erase(it);
        clause_removed = true;
        break;
      }
      else
        lit_cnt++;
    }
    if(!clause_removed)
      ++it;
  }
}

// -------------------------------------------------------------------------------------------
void CNF::doPureAndUnit(const vector<int> &keep)
{
  set<int> unit;
  bool unit_changed = true;
  while(unit_changed)
  {
    unit_changed = false;
    for(ClauseIter it = clauses_.begin(); it != clauses_.end();)
    {
      bool erase = false;
      if(it->size() == 1)
      {
        unit.insert(it->front());
        erase = true;
        unit_changed = true;
      }
      else if(!unit.empty())
      {
        for(size_t c = 0; c < it->size(); )
        {
          int lit = (*it)[c];
          if(unit.count(lit)) // this clause is true, remove it:
          {
            erase = true;
            break;
          }
          else if(unit.count(-lit)) // this literal is false, remove it:
          {
            (*it)[c] = it->back();
            it->pop_back();
            if(it->size() == 1)
            {
              unit.insert(it->front());
              erase = true;
              unit_changed = true;
              break;
            }
          }
          else
            ++c;
        }
      }
      if(erase)
        it = clauses_.erase(it);
      else
        ++it;
    }
  }

  for(size_t cnt = 0; cnt < keep.size(); ++cnt)
  {
    if(unit.count(keep[cnt]) != 0)
      add1LitClause(keep[cnt]);
    else if(unit.count(-keep[cnt]) != 0)
      add1LitClause(-keep[cnt]);
  }
}

// -------------------------------------------------------------------------------------------
void CNF::renameVars(const vector<int> &rename_map)
{
  for(ClauseIter it = clauses_.begin(); it != clauses_.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
    {
      int lit = (*it)[lit_cnt];
      if(lit < 0)
        (*it)[lit_cnt] = -rename_map[-lit];
      else
        (*it)[lit_cnt] = rename_map[lit];
    }
  }
}

// -------------------------------------------------------------------------------------------
bool CNF::contains(int var) const
{
  for(ClauseConstIter it = clauses_.begin(); it != clauses_.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
    {
      int cnf_lit = (*it)[lit_cnt];
      int cnf_var = cnf_lit < 0 ? -cnf_lit : cnf_lit;
      if(cnf_var == var)
        return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------
vector<int> CNF::getVars() const
{
  set<int> var_set;
  appendVarsTo(var_set);
  vector<int> res;
  res.reserve(var_set.size());
  res.insert(res.end(), var_set.begin(), var_set.end());
  return res;
}

// -------------------------------------------------------------------------------------------
void CNF::appendVarsTo(set<int> &var_set) const
{
  for(ClauseConstIter it = clauses_.begin(); it != clauses_.end(); ++it)
  {
    for(size_t lit_cnt = 0; lit_cnt < it->size(); ++lit_cnt)
    {
      int cnf_lit = (*it)[lit_cnt];
      int cnf_var = cnf_lit < 0 ? -cnf_lit : cnf_lit;
      var_set.insert(cnf_var);
    }
  }
}

// -------------------------------------------------------------------------------------------
bool CNF::operator==(const CNF &other) const
{
  return clauses_ == other.clauses_;
}
