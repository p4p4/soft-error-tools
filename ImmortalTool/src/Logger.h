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
/// @file Logger.h
/// @brief Contains the declaration of the class Logger and some macros for logging.
// -------------------------------------------------------------------------------------------

#ifndef Logger_H__
#define Logger_H__

#include "defines.h"

// -------------------------------------------------------------------------------------------
///
/// @class Logger
/// @brief Prints messages of different kind, which can all be enabled and disabled.
///
/// This utility class is able to print 6 different types of messages: ERRORs, WARNINGs
/// RESULTs, INFOs, DEBUG messages and LOG messages. Every kind of message can be enabled and
/// disabled.
/// Do not directly print to stdout or stderr but use this class instead (except for special
/// cases like printing messages before this Logger has been initialized).
///
/// This class is implemented as a Singleton. That is, you cannot instantiate objects of
/// this class with the constructor. Use the method @link #instance instance() @endlink to
/// obtain the one and only instance of this class.
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.2.0
class Logger
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the one and only instance of this class.
///
/// This class is implemented as a Singleton. That is, you cannot instantiate objects of
/// this class with the constructor. Use this method to obtain the one and only instance of
/// this class.
///
/// @return The one and only instance of this class.
  static Logger& instance();

// -------------------------------------------------------------------------------------------
///
/// @brief A type for the kind of message.
  enum Logtype {

// -------------------------------------------------------------------------------------------
///
/// @brief The value for error messages.
///
/// Use this kind for messages like 'could not open file'.
    ERR = 0,

// -------------------------------------------------------------------------------------------
///
/// @brief The value for warnings.
///
/// Use this kind for messages like 'pending signal'.
    WRN = 1,

// -------------------------------------------------------------------------------------------
///
/// @brief The value for printing results.
///
/// Use this kind for messages like 'the spec is realizable'.
    RES = 2,

// -------------------------------------------------------------------------------------------
///
/// @brief The value for printing additional information.
///
/// Use this kind for messages like 'starting to compute a circuit ...'.
    INF = 3,

// -------------------------------------------------------------------------------------------
///
/// @brief The value for printing debug messages.
///
/// Use this kind for messages like 'the aiger literal 4 corresponds to 12 in CNF'.
    DBG = 4,

// -------------------------------------------------------------------------------------------
///
/// @brief The value for printing log messages.
///
/// This is typically used for performance measures in messages like 'computing the winning
/// region took 16 seconds'.
    LOG = 5
  };

// -------------------------------------------------------------------------------------------
///
/// @brief Enables the printing of a certain kind of message.
///
/// If a message kind is disabled, all messaged of this type are discarded.
///
/// @param lt The kind of message to enable.
  void enable(Logtype lt);

// -------------------------------------------------------------------------------------------
///
/// @brief Disables the printing of a certain kind of message.
///
/// If a message kind is disabled, all messaged of this type are discarded.
///
/// @param lt The kind of message to disable.
  void disable(Logtype lt);

// -------------------------------------------------------------------------------------------
///
/// @brief Checks if a certain kind of message is enabled.
///
/// If a message kind is disabled, all messaged of this type are discarded.
///
/// @param lt The kind of message in question.
/// @return True if this kind of message is enabled, false if it is disabled.
  bool isEnabled(Logtype lt) const;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns the output stream for a certain kind of message.
///
/// Use this method to pipe arbitrary messages into the requested streams.
/// See also the macros
/// <ul>
///  <li> #L_ERR
///  <li> #L_WRN
///  <li> #L_RES
///  <li> #L_INF
///  <li> #L_DBG
///  <li> #L_LOG
/// </ul>
/// which might be more convenient to use.
///
/// @note This method does not check if the particular kind of message is enabled or
///       disabled, i.e, it allows you to print messages although they are disabled.
/// @param lt The kind of message for which the output stream is requested.
/// @return The output stream for this message.
  ostream &getOstream(Logtype lt);

protected:

// -------------------------------------------------------------------------------------------
///
/// @brief The one and only instance of this class.
  static Logger *instance_;

// -------------------------------------------------------------------------------------------
///
/// @brief The output streams to be used for the different kinds of messages.
///
/// The order in the array is as following:
/// <ul>
///  <li> streams_[0] is the output stream for errors (Logtype::ERR)
///  <li> streams_[1] is the output stream for warnings (Logtype::WRN)
///  <li> streams_[2] is the output stream for results (Logtype::RES)
///  <li> streams_[3] is the output stream for infos (Logtype::INF)
///  <li> streams_[4] is the output stream for debug messages (Logtype::DBG)
///  <li> streams_[5] is the output stream for log messages (Logtype::LOG)
/// </ul>
  vector<ostream*> streams_;

// -------------------------------------------------------------------------------------------
///
/// @brief The flags indicating which kind of message is enabled.
///
/// The order in the array is as following:
/// <ul>
///  <li> enabled_[0] is true if errors are enabled.
///  <li> enabled_[1] is true if warnings are enabled.
///  <li> enabled_[2] is true if results are enabled.
///  <li> enabled_[3] is true if infos are enabled.
///  <li> enabled_[4] is true if debug messages are enabled.
///  <li> enabled_[5] is true if log messages are enabled.
/// </ul>
  vector<bool> enabled_;

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// The constructor is disabled (set private) as this method is implemented as a Singleton.
/// Use the method @link #instance instance @endlink to obtain the one and only instance of
/// this class.
  Logger();

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
///
/// The destructor is disabled (set private) as this method is implemented as a Singleton.
/// One cannot instantiate objects of this class, so there is no need to be able to delete
/// them.
  virtual ~Logger();

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  Logger(const Logger &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  Logger &operator= (const Logger &other);
};

// -------------------------------------------------------------------------------------------
///
/// @def L_ERR(message)
/// @brief Prints an error message if this message type is enabled.
///
/// The message to be printed is preceded by '[ERR] ' and terminated with a newline.
///
/// @param message The message to print.
#define L_ERR(message)                                                    \
{                                                                         \
  if (Logger::instance().isEnabled(Logger::ERR))                          \
  {                                                                       \
    (Logger::instance().getOstream(Logger::ERR)) <<                       \
    "[ERR] " << message << std::endl;                                     \
  }                                                                       \
}

// -------------------------------------------------------------------------------------------
///
/// @def L_WRN(message)
/// @brief Prints a warning message if this message type is enabled.
///
/// The message to be printed is preceded by '[WRN] ' and terminated with a newline.
///
/// @param message The message to print.
#define L_WRN(message)                                                    \
{                                                                         \
  if (Logger::instance().isEnabled(Logger::WRN))                          \
  {                                                                       \
    (Logger::instance().getOstream(Logger::WRN)) <<                       \
    "[WRN] " << message << std::endl;                                     \
  }                                                                       \
}

// -------------------------------------------------------------------------------------------
///
/// @def L_RES(message)
/// @brief Prints a result message if this message type is enabled.
///
/// The message to be printed is preceded by '[RES] ' and terminated with a newline.
///
/// @param message The message to print.
#define L_RES(message)                                                    \
{                                                                         \
  if (Logger::instance().isEnabled(Logger::RES))                          \
  {                                                                       \
    (Logger::instance().getOstream(Logger::RES)) <<                       \
    "[RES] " << message << std::endl;                                     \
  }                                                                       \
}

// -------------------------------------------------------------------------------------------
///
/// @def L_INF(message)
/// @brief Prints an info message if this message type is enabled.
///
/// The message to be printed is preceded by '[INF] ' and terminated with a newline.
///
/// @param message The message to print.
#define L_INF(message)                                                    \
{                                                                         \
  if (Logger::instance().isEnabled(Logger::INF))                          \
  {                                                                       \
    (Logger::instance().getOstream(Logger::INF)) <<                       \
    "[INF] " << message << std::endl;                                     \
  }                                                                       \
}

// -------------------------------------------------------------------------------------------
///
/// @def L_DBG(message)
/// @brief Prints a debug message if this message type is enabled.
///
/// The message to be printed is preceded by '[DBG] ' and terminated with a newline.
///
/// @param message The message to print.
#define L_DBG(message)                                                    \
{                                                                         \
  if (Logger::instance().isEnabled(Logger::DBG))                          \
  {                                                                       \
    (Logger::instance().getOstream(Logger::DBG)) <<                       \
    "[DBG] " << message << std::endl;                                     \
  }                                                                       \
}

// -------------------------------------------------------------------------------------------
///
/// @def L_LOG(message)
/// @brief Prints a log message if this message type is enabled.
///
/// The message to be printed is preceded by '[LOG] ' and terminated with a newline.
///
/// @param message The message to print.
#define L_LOG(message)                                                    \
{                                                                         \
  if (Logger::instance().isEnabled(Logger::LOG))                          \
  {                                                                       \
    (Logger::instance().getOstream(Logger::LOG)) <<                       \
    "[LOG] " << message << std::endl;                                     \
  }                                                                       \
}

#endif // Logger_H__
