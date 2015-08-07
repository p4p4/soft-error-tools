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
/// @file Options.h
/// @brief Contains the declaration of the class Options.
// -------------------------------------------------------------------------------------------

#ifndef Options_H__
#define Options_H__

#include "defines.h"

class QBFSolver;
class SatSolver;
class BackEnd;
class CNFImplExtractor;

typedef pair<clock_t, time_t> PointInTime;

// -------------------------------------------------------------------------------------------
///
/// @class Options
/// @brief Parses the command-line options and makes them accessible to the back-ends.
///
/// The main purpose of this class is to parse the command-line options. It provides
/// methods not only for accessing the option values but also for instantiating concrete
/// objects (SAT-solvers, QBF-solvers) as selected by the user.
///
/// This class is implemented as a Singleton. That is, you cannot instantiate objects of
/// this class with the constructor. Use the method @link #instance instance() @endlink to
/// obtain the one and only instance of this class. This is done for convenience. Every
/// back-end can easily access this object without passing it around.
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.2.0
class Options
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief The version of the tool as string.
  static const string VERSION;

// -------------------------------------------------------------------------------------------
///
/// @brief The environment variable with the directory containing all third-party software.
///
/// The third-party software includes SAT-solvers, QBF-solvers, etc.
  static const string TP_VAR;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the one and only instance of this class.
///
/// This class is implemented as a Singleton. That is, you cannot instantiate objects of
/// this class with the constructor. Use this method to obtain the one and only instance of
/// this class.
///
/// @return The one and only instance of this class.
  static Options& instance();

// -------------------------------------------------------------------------------------------
///
/// @brief Parses the command-line options and stores them in fields of this class.
///
/// @param argc The number of arguments passed to main.
/// @param argv The arguments passed to main. argv_[0] is the name of the process, the real
///        arguments start with argv_[1].
/// @return True if the tool should quit, false otherwise.
  bool parse(int argc, char **argv);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the name (including the path) of the AIGER input file.
///
/// @return The name (including the path) of the AIGER input file.
  const string& getAigInFileName() const;



// -------------------------------------------------------------------------------------------
///
/// @brief Returns the name of the input file without path and extension.
///
/// @return The name of the input file without path and extension.
    const string getAigInFileNameOnly() const;


// -------------------------------------------------------------------------------------------
///
/// @brief Returns the name of the back-end selected by the user.
///
/// @return The name of the back-end selected by the user.
  const string& getBackEndName() const;

// -------------------------------------------------------------------------------------------
///
/// @brief A factory method to construct a BackEnd, depending on the command-line parameters.
///
/// This method returns the back-end that has been selected by the user with command-line
/// parameters.
///
/// @note The returned object has to be deleted by the caller.
/// @return The back-end selected by the user with commend-line options.
  BackEnd* getBackEnd() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the mode of the back-end selected by the user.
///
/// Some back-ends can be used in several modes (certain optimizations or heuristics enabled
/// or disabled, etc.). This integer number selects a mode.
///
/// @return The mode of the back-end selected by the user.
  int getBackEndMode() const;



// -------------------------------------------------------------------------------------------
///
/// @brief Returns the name (including the path) of the directory for temporary files.
///
/// Some back-ends or solvers produce temporary files. Store them in the directory returned
/// by this method in order to avoid a mess: If all temporary files are stored in one signle
/// directory, this directory can be deleted manually if something goes wrong.
///
/// The directory returned by this method is guaranteed to exist. If it does not, it will be
/// created by this method.
///
/// @return The mode of the back-end selected by the user.
  string getTmpDirName() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the name (and path) of the directory containing the thirdparty tools.
///
/// The implementation of this method assumes that there is an environment variable named
/// DEMIURGETP, which holds the name of the directory in which the third-party tools
/// (SAT-solvers, QBF-solvers, logic optimizers, etc.) are installed.  This method returns
/// the name of this directory (where the environment variable is already resolved).
///
/// @return The name (and path) of the directory containing the thirdparty tools.
  string getTPDirName() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a unique name of a temporary file in the directory of temporary files.
///
/// @param start An optional prefix for the file name.
/// @return A unique name of a temporary file in the directory of temporary files.
  string getUniqueTmpFileName(string start = "") const;


// -------------------------------------------------------------------------------------------
///
/// @brief Returns a fresh instance of the SAT-solver selected by the user.
///
/// Calling this method several times will give you different instances of the same solver.
///
/// @note The returned object instance must be deleted by the caller.
/// @param rand_models A flag indicating if satisfying assignments should be randomized.
///        This is done in a post-processing step (values are flipped randomly and then we
///        check if this still constitutes a satisfying assignment). This is expensive.
///        If this parameter is skipped, then satisfying assignments are not randomized.
/// @param min_cores A flag indicating if unsatisfiable cores returned by the solver should
///        be minimized further by trying to drop one literal after the other. This makes the
///        calls slower but produces potentially smaller cubes. If this parameter is skipped,
///        then cores will be further minimized.
/// @return A fresh instance of the SAT-solver selected by the user.
  SatSolver* getSATSolver(bool rand_models = false, bool min_cores = true) const;



protected:

// -------------------------------------------------------------------------------------------
///
/// @brief Prints a help text to stdout.
  void printHelp() const;

// -------------------------------------------------------------------------------------------
///
/// @brief Initializes the Logger by enabling the selected message kinds.
  void initLogger() const;

// -------------------------------------------------------------------------------------------
///
/// @brief The name (including the path) of the AIGER input file.
  string aig_in_file_name_;

// -------------------------------------------------------------------------------------------
///
/// @brief A string defining which messages to print.
///
/// The string can be composed of the letters D,E,R,W,I,L in any order. If a letter occurs
/// in the string, then this means that the respective message kind is enabled. If it does
/// not occur in the string, then the message kind is disabled.
/// <ul>
///  <li> E enables error messages.
///  <li> W enables warnings.
///  <li> I enables info messages.
///  <li> R enables result messages.
///  <li> L enables log messages.
///  <li> D enables debug messages.
/// </ul>
  string print_string_;

// -------------------------------------------------------------------------------------------
///
/// @brief The name of a directory for temporary files.
  string tmp_dir_;

// -------------------------------------------------------------------------------------------
///
/// @brief The name of the selected back-end.
  string back_end_;

// -------------------------------------------------------------------------------------------
///
/// @brief The mode of the back-end selected by the user.
///
/// Some back-ends can be used in several modes (certain optimizations or heuristics enabled
/// or disabled, etc.). This integer number selects a mode.
  int mode_;



// -------------------------------------------------------------------------------------------
///
/// @brief The name of the selected SAT-solver.
  string sat_solver_;


// -------------------------------------------------------------------------------------------
///
/// @brief The point in time when the tool has been started.
///
/// It will be used to estimate how close the timeout is. It may also be used for logging
/// statistics.
  PointInTime tool_started_;

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// The constructor is disabled (set private) as this method is implemented as a Singleton.
/// Use the method @link #instance instance @endlink to obtain the one and only instance of
/// this class.
  Options();

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
///
/// The destructor is disabled (set private) as this method is implemented as a Singleton.
/// One cannot instantiate objects of this class, so there is no need to be able to delete
/// them.
  virtual ~Options();

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  Options(const Options &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  Options &operator= (const Options &other);

// -------------------------------------------------------------------------------------------
///
/// @brief The one and only instance of this class.
  static Options *instance_;
};

#endif // Options_H__
