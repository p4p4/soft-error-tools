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
/// @file Options.cpp
/// @brief Contains the definition of the class Options.
// -------------------------------------------------------------------------------------------

#include "Options.h"
#include "Logger.h"
#include "LingelingApi.h"
#include "Stopwatch.h"

#include <sys/stat.h>
#include <unistd.h>

// -------------------------------------------------------------------------------------------
const string Options::VERSION = string("1.2.0");

const string Options::TP_VAR = string("DEMIURGETP");

// -------------------------------------------------------------------------------------------
Options *Options::instance_ = NULL;

// -------------------------------------------------------------------------------------------
Options &Options::instance()
{
  if(instance_ == NULL)
    instance_ = new Options;
  MASSERT(instance_ != NULL, "Could not create Options instance.");
  return *instance_;
}

// -------------------------------------------------------------------------------------------
bool Options::parse(int argc, char **argv)
{
	// TODO
	return false;
}

// -------------------------------------------------------------------------------------------
const string& Options::getAigInFileName() const
{
  return aig_in_file_name_;
}

// -------------------------------------------------------------------------------------------
const string Options::getAigInFileNameOnly() const
{
  size_t name_start = aig_in_file_name_.find_last_of("\\/");
  if(name_start == string::npos)
    name_start = 0;
  else
    name_start++;
  // the following line also works if '.' is not found:
}

// -------------------------------------------------------------------------------------------
const string& Options::getBackEndName() const
{
  return back_end_;
}


// -------------------------------------------------------------------------------------------
int Options::getBackEndMode() const
{
  return mode_;
}

// -------------------------------------------------------------------------------------------
const string& Options::getCircuitExtractionName() const
{
  return circuit_extraction_name_;
}

// -------------------------------------------------------------------------------------------
int Options::getCircuitExtractionMode() const
{
  return circuit_extraction_mode_;
}



// -------------------------------------------------------------------------------------------
string Options::getTmpDirName() const
{
  struct stat st;
  if(stat(tmp_dir_.c_str(), &st) != 0)
  {
    int fail = mkdir(tmp_dir_.c_str(), 0777);
    MASSERT(fail == 0, "Could not create directory for temporary files.");
  }
  return tmp_dir_;
}

// -------------------------------------------------------------------------------------------
string Options::getUniqueTmpFileName(string start) const
{
  static int unique_counter = 0;
  ostringstream res;
  res << getTmpDirName() << "/" << start << "_" << getAigInFileNameOnly();
  res << "_" << getpid() << "_" << unique_counter++;
  return res.str();
}

// -------------------------------------------------------------------------------------------
string Options::getTPDirName() const
{
  // in the competition, setting environment variables is difficult:
  //return string("./third_party_install/");
  const char *tp_env = getenv(Options::TP_VAR.c_str());
  MASSERT(tp_env != NULL, "You have not set the variable " << Options::TP_VAR);
  return string(tp_env);
}

// -------------------------------------------------------------------------------------------
SatSolver* Options::getSATSolver(bool rand_models, bool min_cores) const
{
  if(sat_solver_ == "lin_api")
    return new LingelingApi(rand_models, min_cores);
//  if(sat_solver_ == "min_api")
//    return new MiniSatApi(rand_models, min_cores);
//  if(sat_solver_ == "pic_api")
//    return new PicoSatApi(rand_models, min_cores);
  MASSERT(false, "Unknown SAT solver name.");
  return NULL;
}

// -------------------------------------------------------------------------------------------
SatSolver* Options::getSATSolverExtr(bool rand_models, bool min_cores) const
{
  if(circuit_sat_solver_ == "")
    return getSATSolver(rand_models, min_cores);
  if(circuit_sat_solver_ == "lin_api")
    return new LingelingApi(rand_models, min_cores);
//  if(circuit_sat_solver_ == "min_api")
//    return new MiniSatApi(rand_models, min_cores);
//  if(circuit_sat_solver_ == "pic_api")
//    return new PicoSatApi(rand_models, min_cores);
  MASSERT(false, "Unknown SAT solver name.");
  return NULL;
}

// -------------------------------------------------------------------------------------------
bool Options::doRealizabilityOnly() const
{
  return real_only_;
}

// -------------------------------------------------------------------------------------------
size_t Options::getSizeLimitForExpansion() const
{
  return exp_limit_in_kb_;
}

// -------------------------------------------------------------------------------------------
void Options::setSizeLimitForExpansion(size_t limit_in_kb)
{
  exp_limit_in_kb_ = limit_in_kb;
}

// -------------------------------------------------------------------------------------------
size_t Options::getEstimatedTimeRemaining() const
{
  size_t elapsed = Stopwatch::getRealTimeSec(tool_started_);
  if(elapsed >= hint_to_in_sec_)
    return 0;
  return hint_to_in_sec_ - elapsed;
}

// -------------------------------------------------------------------------------------------
void Options::printHelp() const
{
  cout << "Usage: TODO.................."                                           << endl;
  cout << "Have fun!"                                                               << endl;
}

// -------------------------------------------------------------------------------------------
void Options::initLogger() const
{
  Logger &logger = Logger::instance();
  if(print_string_.find("E") != string::npos || print_string_.find("e") != string::npos)
    logger.enable(Logger::ERR);
  else
    logger.disable(Logger::ERR);
  if(print_string_.find("W") != string::npos || print_string_.find("w") != string::npos)
    logger.enable(Logger::WRN);
  else
    logger.disable(Logger::WRN);
  if(print_string_.find("R") != string::npos || print_string_.find("r") != string::npos)
    logger.enable(Logger::RES);
  else
    logger.disable(Logger::RES);
  if(print_string_.find("I") != string::npos || print_string_.find("i") != string::npos)
    logger.enable(Logger::INF);
  else
    logger.disable(Logger::INF);
  if(print_string_.find("D") != string::npos || print_string_.find("d") != string::npos)
    logger.enable(Logger::DBG);
  else
    logger.disable(Logger::DBG);
  if(print_string_.find("L") != string::npos || print_string_.find("l") != string::npos)
    logger.enable(Logger::LOG);
  else
    logger.disable(Logger::LOG);
}

// -------------------------------------------------------------------------------------------
Options::Options():
    aig_in_file_name_(),
    aig_out_file_name_("stdout"),
    print_string_("ERWI"),
    tmp_dir_("./tmp"),
    back_end_("lp1"),
    mode_(0),
    circuit_extraction_name_("lp1"),
    circuit_extraction_mode_(0),
    qbf_solver_("depqbf_api"),
    sat_solver_("min_api"),
    circuit_sat_solver_(""),
    real_only_(false),
    exp_limit_in_kb_(3*1024*1024),
    hint_to_in_sec_(0),
    tool_started_(Stopwatch::start())
{
  // nothing to be done
}

// -------------------------------------------------------------------------------------------
Options::~Options()
{
  // nothing to be done
}
