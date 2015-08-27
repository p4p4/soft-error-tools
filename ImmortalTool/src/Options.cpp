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
#include "StringUtils.h"
#include "SimulationBasedAnalysis.h"
#include "SymbTimeAnalysis.h"
#include "SymbTimeLocationAnalysis.h"
#include "Utils.h"

#include <sys/stat.h>
#include <unistd.h>

extern "C"
{
#include "aiger.h"
}

// -------------------------------------------------------------------------------------------
const string Options::VERSION = string("0.1");
const string Options::TP_VAR = string("IMMORTALTP");

// -------------------------------------------------------------------------------------------
Options *Options::instance_ = NULL;

// -------------------------------------------------------------------------------------------
Options &Options::instance()
{
	if (instance_ == NULL)
		instance_ = new Options;
	MASSERT(instance_ != NULL, "Could not create Options instance.");
	return *instance_;
}

// -------------------------------------------------------------------------------------------
bool Options::parse(int argc, char **argv)
{
	L_LOG("Tool has been started");
	for (int arg_count = 1; arg_count < argc; ++arg_count)
	{
		string arg(argv[arg_count]);
		if (arg == "--help" || arg == "-h")
		{
			printHelp();
			return true;
		}
		else if (arg == "--version" || arg == "-v")
		{
			cout << "tool version " << Options::VERSION << endl;
			return true;
		}
		else if (arg.find("--in=") == 0)
		{
			aig_in_file_name_ = arg.substr(5, string::npos);
		}
		else if (arg == "-i")
		{
			++arg_count;
			if (arg_count >= argc)
			{
				cerr << "Option -i must be followed by a filename." << endl;
				return true;
			}
			aig_in_file_name_ = string(argv[arg_count]);
		}

		else if (arg.find("--print=") == 0)
		{
			print_string_ = arg.substr(8, string::npos);
		}
		else if (arg == "-p")
		{
			++arg_count;
			if (arg_count >= argc)
			{
				cerr << "Option -p must be followed by a string indicating what to print." << endl;
				return true;
			}
			print_string_ = string(argv[arg_count]);
		}
		else if (arg.find("--tmp=") == 0)
		{
			tmp_dir_ = arg.substr(6, string::npos);
		}
		else if (arg == "-t")
		{
			++arg_count;
			if (arg_count >= argc)
			{
				cerr << "Option -t must be followed by a directory name." << endl;
				return true;
			}
			tmp_dir_ = string(argv[arg_count]);
		}
		else if (arg.find("--backend=") == 0)
		{
			back_end_ = arg.substr(10, string::npos);
			StringUtils::toLowerCaseIn(back_end_);
		}
		else if (arg == "-b")
		{
			++arg_count;
			if (arg_count >= argc)
			{
				cerr << "Option -b must be followed by a back-end name." << endl;
				return true;
			}
			back_end_ = string(argv[arg_count]);
			StringUtils::toLowerCaseIn(back_end_);
		}
		else if (arg.find("--mode=") == 0)
		{
			istringstream iss(arg.substr(7, string::npos));
			iss >> mode_;
		}
		else if (arg.find("--seed=") == 0)
		{
			istringstream iss(arg.substr(7, string::npos));
			iss >> seed_;
		}
		else if (arg == "-m")
		{
			++arg_count;
			if (arg_count >= argc)
			{
				cerr << "Option -m must be followed by an integer number." << endl;
				return true;
			}
			istringstream iss(argv[arg_count]);
			iss >> mode_;
		}
		else if (arg.find("--sat_sv=") == 0)
		{
			sat_solver_ = arg.substr(9, string::npos);
			StringUtils::toLowerCaseIn(sat_solver_);
			if (sat_solver_ != "lin_api" && sat_solver_ != "min_api" && sat_solver_ != "pic_api")
			{
				cerr << "Unknown SAT solver '" << sat_solver_ << "'." << endl;
				return true;
			}
		}
		else if (arg == "-s")
		{
			++arg_count;
			if (arg_count >= argc)
			{
				cerr << "Option -s must be followed by a SAT solver name." << endl;
				return true;
			}
			sat_solver_ = string(argv[arg_count]);
			StringUtils::toLowerCaseIn(sat_solver_);
			if (sat_solver_ != "lin_api" && sat_solver_ != "min_api" && sat_solver_ != "pic_api")
			{
				cerr << "Unknown SAT solver '" << sat_solver_ << "'." << endl;
				return true;
			}
		}
		else if (arg.find("--err_latches=") == 0)
		{
			istringstream iss(arg.substr(14, string::npos));
			iss >> num_err_latches_;
		}
		else if (arg == "-e")
		{
			++arg_count;
			if (arg_count >= argc)
			{
				cerr << "Option -e must be followed by an integer number." << endl;
				return true;
			}
			istringstream iss(argv[arg_count]);
			iss >> num_err_latches_;
		}
		else if (arg == "-tc")
		{
			if (testcase_mode_ != TC_UNDEFINED)
			{
				cerr << "Error: More than one TestCase-Mode given. Which to use?" << endl;
				return true;
			}
			// get path(s) to TestCase-file(s)
			while (++arg_count < argc && argv[arg_count][0] != '-')
			{
				string path_to_testcase(argv[arg_count]);
				paths_to_testcases_.push_back(path_to_testcase);
			}

			if (paths_to_testcases_.size() == 0)
			{
				cerr << "Option -tc must be followed by a path(s) to testcase-file(s)" << endl;
				return true;
			}
			--arg_count;
			testcase_mode_ = TC_FILES;
		}
		else if (arg == "-tcr")
		{
			if (testcase_mode_ != TC_UNDEFINED)
			{
				cerr << "Error: More than one TestCase-Mode given. Which to use?" << endl;
				return true;
			}

			if (arg_count + 2 >= argc)
			{
				cerr << "Option -tcr must be followed by two positive integer numbers." << endl;
				return true;
			}

			istringstream iss(argv[++arg_count]);
			iss >> num_testcases_;

			istringstream iss2(argv[++arg_count]);
			iss2 >> len_rand_testcases_;

			if (num_testcases_ <= 0 || len_rand_testcases_ <= 0)
			{
				cerr << "Option -tcr must be followed by two positive integer numbers." << endl;
				return true;
			}
			testcase_mode_ = TC_RANDOM;
		}
	}

	if (aig_in_file_name_ == "")
	{
		cerr << "No input file given." << endl;
		return true;
	}

	if (testcase_mode_ == TC_UNDEFINED)
	{
		cerr << "No testcase(s) provided." << endl;
		return true;
	}
	initInputCircuit();
	initLogger();

	if(seed_==0)
		srand(time(0)); // seed with current time (used for random TestCases)
	else
		srand(seed_);

	return false; // false = do not quit the tool
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
	if (name_start == string::npos)
		name_start = 0;
	else
		name_start++;
	// the following line also works if '.' is not found:
	size_t len = aig_in_file_name_.find_last_of(".") - name_start;
	return aig_in_file_name_.substr(name_start, len);
}

// -------------------------------------------------------------------------------------------
const string& Options::getBackEndName() const
{
	return back_end_;
}

// -------------------------------------------------------------------------------------------
BackEnd* Options::getBackEnd()
{
	if (back_end_instance_ != 0)
	{
		return back_end_instance_;
	}

	if (back_end_ == "sim")
	{
		back_end_instance_ = new SimulationBasedAnalysis(circuit_, num_err_latches_, mode_);
		return back_end_instance_;
	}
	else if (back_end_ == "sta")
	{
		back_end_instance_ = new SymbTimeAnalysis(circuit_, num_err_latches_, mode_);
		return back_end_instance_;
	}
	else if (back_end_ == "stla")
	{
		back_end_instance_ = new SymbTimeLocationAnalysis(circuit_, num_err_latches_, mode_);
		return back_end_instance_;
	}

	L_ERR("Unknown back-end '" << back_end_ <<"'.");
	exit(-1);
}

// -------------------------------------------------------------------------------------------
int Options::getBackEndMode() const
{
	return mode_;
}

// -------------------------------------------------------------------------------------------
string Options::getTmpDirName() const
{
	struct stat st;
	if (stat(tmp_dir_.c_str(), &st) != 0)
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
	if (sat_solver_ == "lin_api")
		return new LingelingApi(rand_models, min_cores);
//  if(sat_solver_ == "min_api")
//    return new MiniSatApi(rand_models, min_cores);
//  if(sat_solver_ == "pic_api")
//    return new PicoSatApi(rand_models, min_cores);
	MASSERT(false, "Unknown SAT solver name.");
	return NULL;
}

// -------------------------------------------------------------------------------------------
void Options::printHelp() const
{
	cout << "Usage: TODO.................." << endl;
	cout << "Have fun!" << endl;
}

// -------------------------------------------------------------------------------------------
void Options::initLogger() const
{
	Logger &logger = Logger::instance();
	if (print_string_.find("E") != string::npos || print_string_.find("e") != string::npos)
		logger.enable(Logger::ERR);
	else
		logger.disable(Logger::ERR);
	if (print_string_.find("W") != string::npos || print_string_.find("w") != string::npos)
		logger.enable(Logger::WRN);
	else
		logger.disable(Logger::WRN);
	if (print_string_.find("R") != string::npos || print_string_.find("r") != string::npos)
		logger.enable(Logger::RES);
	else
		logger.disable(Logger::RES);
	if (print_string_.find("I") != string::npos || print_string_.find("i") != string::npos)
		logger.enable(Logger::INF);
	else
		logger.disable(Logger::INF);
	if (print_string_.find("D") != string::npos || print_string_.find("d") != string::npos)
		logger.enable(Logger::DBG);
	else
		logger.disable(Logger::DBG);
	if (print_string_.find("L") != string::npos || print_string_.find("l") != string::npos)
		logger.enable(Logger::LOG);
	else
		logger.disable(Logger::LOG);
}

// -------------------------------------------------------------------------------------------
void Options::initInputCircuit()
{
	circuit_ = Utils::readAiger(aig_in_file_name_);
	const string prefix("Err_latch_");

	if (num_err_latches_ == 0)
	{

		for (unsigned i = 0; i < circuit_->num_latches; i++)
		{
			if (!circuit_->latches[i].name)
				continue;

			string latch_name_str(circuit_->latches[i].name);
			if (latch_name_str.compare(0, prefix.size(), prefix) == 0)
			{
				num_err_latches_++;
			}

		}
	}
	L_LOG("Input-File: " << getAigInFileNameOnly())
	L_LOG("Inputs: " << circuit_->num_inputs
			<< ", Latches: " << circuit_->num_latches - num_err_latches_
			<< ", Error Latches: " << num_err_latches_
			<< ", Outputs: " << circuit_->num_outputs-1)

}

// -------------------------------------------------------------------------------------------
Options::Options() :
		testcase_mode_(TC_UNDEFINED), num_testcases_(0), len_rand_testcases_(0), aig_in_file_name_(), print_string_(
				"ERWILD"), tmp_dir_("./tmp"), back_end_("sim"), back_end_instance_(0), mode_(0), sat_solver_(
				"lin_api"), tool_started_(Stopwatch::start()), circuit_(0), num_err_latches_(0), seed_(0)
{
	// nothing to be done
}

// -------------------------------------------------------------------------------------------
Options::~Options()
{
	if (back_end_instance_ != 0)
	{
		delete back_end_instance_;
	}
}

unsigned Options::getNumErrLatches() const
{
	return num_err_latches_;
}
