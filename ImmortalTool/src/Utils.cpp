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
/// @file Utils.cpp
/// @brief Contains the definition of the class Utils.
// -------------------------------------------------------------------------------------------

#include "Utils.h"
#include "Logger.h"
#include "CNF.h"
#include "SatSolver.h"
#include "Options.h"
#include "AIG2CNF.h"
#include <unistd.h>

extern "C" {
  #include "aiger.h"
}

// -------------------------------------------------------------------------------------------
bool Utils::containsInit(const vector<int> &cube)
{
  for(size_t cnt = 0; cnt < cube.size(); ++cnt)
  {
    if(cube[cnt] > 0)
      return false;
  }
  return true;
}

// -------------------------------------------------------------------------------------------
bool Utils::containsInit(const set<int> &cube)
{
  for(set<int>::const_iterator it = cube.begin(); it != cube.end(); ++it)
  {
    if((*it) > 0)
      return false;
  }
  return true;
}


// -------------------------------------------------------------------------------------------
vector<int> Utils::extract(const vector<int> &cube_or_clause, const vector<int> &vars)
{
  vector<int> res;
  res.reserve(cube_or_clause.size());
  for(size_t cnt = 0; cnt < cube_or_clause.size(); ++cnt)
  {
    int lit = cube_or_clause[cnt];
    int var = lit < 0 ? -lit : lit;
    if(contains(vars, var))
      res.push_back(lit);
  }
  return res;
}

// -------------------------------------------------------------------------------------------
void Utils::randomize(vector<int> &vec)
{
  random_shuffle(vec.begin(),vec.end());
}

// -------------------------------------------------------------------------------------------
void Utils::sort(vector<int> &vec)
{
  std::sort(vec.begin(), vec.end());
}

// -------------------------------------------------------------------------------------------
bool Utils::remove(vector<int> &vec, int elem)
{
  for(size_t cnt = 0; cnt < vec.size(); ++cnt)
  {
    if(vec[cnt] == elem)
    {
      vec[cnt] = vec.back();
      vec.pop_back();
      return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------
bool Utils::contains(const vector<int> &vec, int elem)
{
  for(size_t cnt = 0; cnt < vec.size(); ++cnt)
  {
    if(vec[cnt] == elem)
      return true;
  }
  return false;
}

// -------------------------------------------------------------------------------------------
bool Utils::eq(const vector<int> &v1, const vector<int> &v2, int start_idx)
{
  if(v1.size() != v2.size())
    return false;
  set<int> s1, s2;
  for(size_t cnt = start_idx; cnt < v1.size(); ++cnt)
  {
    s1.insert(v1[cnt]);
    s2.insert(v2[cnt]);
  }
  return (s1 == s2);
}

// -------------------------------------------------------------------------------------------
void Utils::negateLiterals(vector<int> &cube_or_clause)
{
  for(size_t cnt = 0; cnt < cube_or_clause.size(); ++cnt)
    cube_or_clause[cnt] = -cube_or_clause[cnt];
}



// -------------------------------------------------------------------------------------------
bool Utils::intersectionEmpty(const vector<int> &x, const set<int> &y)
{
  vector<int>::const_iterator i = x.begin();
  set<int>::const_iterator j = y.begin();
  while (i != x.end() && j != y.end())
  {
    if (*i == *j)
      return false;
    else if (*i < *j)
      ++i;
    else
      ++j;
  }
  return true;
}

// -------------------------------------------------------------------------------------------
bool Utils::intersectionEmpty(const set<int> &x, const set<int> &y)
{
  set<int>::const_iterator i = x.begin();
  set<int>::const_iterator j = y.begin();
  while (i != x.end() && j != y.end())
  {
    if (*i == *j)
      return false;
    else if (*i < *j)
      ++i;
    else
      ++j;
  }
  return true;
}

// -------------------------------------------------------------------------------------------
bool Utils::isSubset(const vector<int> &subset, const vector<int> &superset)
{
  if(subset.size() > superset.size())
    return false;
  for(size_t cnt1 = 0; cnt1 < subset.size(); ++cnt1)
  {
    bool found = false;
    int to_find = subset[cnt1];
    for(size_t cnt2 = 0; cnt2 < superset.size(); ++cnt2)
    {
      if(superset[cnt2] == to_find)
      {
        found = true;
        break;
      }
    }
    if(!found)
      return false;
  }
  return true;
}




// -------------------------------------------------------------------------------------------
size_t Utils::getCurrentMemUsage()
{
  using std::ios_base;
  using std::ifstream;
  using std::string;

  // 'file' stat seems to give the most reliable results
  //
  ifstream stat_stream("/proc/self/stat",ios_base::in);

  // dummy vars for leading entries in stat that we don't care about
  //
  string pid, comm, state, ppid, pgrp, session, tty_nr;
  string tpgid, flags, minflt, cminflt, majflt, cmajflt;
  string utime, stime, cutime, cstime, priority, nice;
  string O, itrealvalue, starttime;

  // the two fields we want
  //
  unsigned long vsize;
  long rss;

  stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
              >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
              >> utime >> stime >> cutime >> cstime >> priority >> nice
              >> O >> itrealvalue >> starttime >> vsize >> rss;

  stat_stream.close();

  return vsize / 1024;
}

// -------------------------------------------------------------------------------------------
void Utils::debugPrint(const vector<int> &vec, string prefix)
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
  L_DBG(prefix << oss.str());
}

// -------------------------------------------------------------------------------------------
void Utils::debugCheckWinReg(const CNF &winning_region)
{
#ifndef NDEBUG
  CNF neg_winnning_region(winning_region);
  neg_winnning_region.negate();
  debugCheckWinReg(winning_region, neg_winnning_region);
#endif
}


// -------------------------------------------------------------------------------------------
void Utils::debugPrintCurrentMemUsage()
{
  using std::ios_base;
  using std::ifstream;
  using std::string;

  double vm_usage     = 0.0;
  double resident_set = 0.0;

  // 'file' stat seems to give the most reliable results
  //
  ifstream stat_stream("/proc/self/stat",ios_base::in);

  // dummy vars for leading entries in stat that we don't care about
  //
  string pid, comm, state, ppid, pgrp, session, tty_nr;
  string tpgid, flags, minflt, cminflt, majflt, cmajflt;
  string utime, stime, cutime, cstime, priority, nice;
  string O, itrealvalue, starttime;

  // the two fields we want
  //
  unsigned long vsize;
  long rss;

  stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
              >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
              >> utime >> stime >> cutime >> cstime >> priority >> nice
              >> O >> itrealvalue >> starttime >> vsize >> rss;

  stat_stream.close();

  long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024;
  resident_set = rss * page_size_kb;
  L_DBG("Resident set: " << resident_set << " kB.");
  vm_usage     = vsize / 1024.0;
  L_DBG("Virtual Memory: " << vm_usage << " kB.");
}

aiger* Utils::readAiger(string path)
{
	// read file:
		aiger* aig_input = aiger_init();
		const char *read_err = aiger_open_and_read_from_file(aig_input, path.c_str());

		if(read_err != NULL)
		{
			L_ERR("Error: Could not open AIGER file: '" << path << "'");
			exit(-1);
		}

		return aig_input;
}
