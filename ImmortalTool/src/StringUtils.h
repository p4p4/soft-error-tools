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
/// @file StringUtils.h
/// @brief Contains the declaration of the class StringUtils.
// -------------------------------------------------------------------------------------------

#ifndef StringUtils_H__
#define StringUtils_H__

#include "defines.h"

// -------------------------------------------------------------------------------------------
///
/// @class StringUtils
/// @brief Contains utility functions on strings.
///
/// @author Robert Koenighofer (robert.koenighofer@iaik.tugraz.at)
/// @version 1.2.0
class StringUtils
{
public:

// -------------------------------------------------------------------------------------------
///
/// @brief A string containing all whitespaces.
  static const string WS;

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a copy of a string where all lower-case letters are turned into upper-case.
///
/// The only difference to @link #toUpperCaseIn toUppperCaseIn @endlink is that this method
/// does not modify the passed string.
///
/// @param in The string in which all lower-case letters should be turned into upper-case
///        letters. This string is NOT modified.
/// @return A copy of the passed string, where all lower-case letters are turned into upper
///         case letters.
  static string toUpperCase(const string &in);

// -------------------------------------------------------------------------------------------
///
/// @brief Turns all lower-case letters into upper-case letters.
///
/// The only difference to @link #toUpperCase toUppperCase @endlink is that this method
/// modifies the passed string.
///
/// @param in The string to process.
  static void toUpperCaseIn(string &in);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a copy of a string where all upper-case letters are turned into lower-case.
///
/// The only difference to @link #toLowerCaseIn toLowerCaseIn @endlink is that this method
/// does not modify the passed string.
///
/// @param in The string in which all upper-case letters should be turned into lower-case
///        letters. This string is NOT modified.
/// @return A copy of the passed string, where all upper-case letters are turned into lower
///         case letters.
  static string toLowerCase(const string &in);

// -------------------------------------------------------------------------------------------
///
/// @brief Turns all upper-case letters into lower-case letters.
///
/// The only difference to @link #toLowerCase toLowerCase @endlink is that this method
/// modifies the passed string.
///
/// @param in The string to process.
  static void toLowerCaseIn(string &in);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a copy of a string where all leading and trailing whitespaces are removed.
///
/// The only difference to @link #trimIn trimIn @endlink is that this method
/// does not modify the passed string.
///
/// @param in The string in which all leading and trailing whitespaces should be removed.
/// @param chars A string containing all characters to trim. The default is a string
///        containing all whitespace characters.
/// @return The passed string where all leading and trailing whitespaces have been removed.
  static string trim(const string &in, const string &chars = WS);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a copy of the passed string where all leading whitespaces are removed.
///
/// The only difference to @link #lTrimIn lTrimIn @endlink is that this method
/// does not modify the passed string.
///
/// @param in The string in which all leading whitespaces should be removed.
/// @param chars A string containing all characters to trim. The default is a string
///        containing all whitespace characters.
/// @return The passed string where all leading whitespaces have been removed.
  static string lTrim(const string &in, const string &chars = WS);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a copy of the passed string where all trailing whitespaces are removed.
///
/// The only difference to @link #rTrimIn rTrimIn @endlink is that this method
/// does not modify the passed string.
///
/// @param in The string in which all trailing whitespaces should be removed.
/// @param chars A string containing all characters to trim. The default is a string
///        containing all whitespace characters.
/// @return The passed string where all trailing whitespaces have been removed.
  static string rTrim(const string &in, const string &chars = WS);

// -------------------------------------------------------------------------------------------
///
/// @brief Removes all leading and trailing whitespaces.
///
/// The only difference to @link #trim trim @endlink is that this method
/// modifies the passed string.
///
/// @param in The string in which all leading and trailing whitespaces should be removed.
/// @param chars A string containing all characters to trim. The default is a string
///        containing all whitespace characters.
  static void trimIn(string &in, const string &chars = WS);

// -------------------------------------------------------------------------------------------
///
/// @brief Removes all leading whitespaces.
///
/// The only difference to @link #lTrim lTrim @endlink is that this method
/// modifies the passed string.
///
/// @param in The string in which all leading whitespaces should be removed.
/// @param chars A string containing all characters to trim. The default is a string
///        containing all whitespace characters.
  static void lTrimIn(string &in, const string &chars = WS);

// -------------------------------------------------------------------------------------------
///
/// @brief Removes all trailing whitespaces.
///
/// The only difference to @link #rTrim rTrim @endlink is that this method
/// modifies the passed string.
///
/// @param in The string in which all trailing whitespaces should be removed.
/// @param chars A string containing all characters to trim. The default is a string
///        containing all whitespace characters.
  static void rTrimIn(string &in, const string &chars = WS);

// -------------------------------------------------------------------------------------------
///
/// @brief Counts the occurrences of a certain pattern in a string.
///
/// @param in The string in which the occurrences of 'what' should be counted.
/// @param what The string to count.
/// @return The number of times 'what' occurs in 'in'. If what is the empty string, 0 is
///         returned.
  static size_t count(const string &in, const string &what);

// -------------------------------------------------------------------------------------------
///
/// @brief Returns a copy of a string where all occurrences of a certain pattern are replaced.
///
/// This method copies the passed string, replaces all occurrences of the string 'from'
/// with the string 'to', and returns the resulting string.
/// That is, StringUtils.replaceAll("Evil devil", "vi", "mai") returns "Email demail".
/// If 'from' is the empty string, nothing is replaced.
///
/// The only difference to @link #replaceAllIn replaceAllIn @endlink is that this method
/// does not modify the passed string.
///
/// @param in The string in which the replacement should be made.
/// @param from The string from which all occurrences should be replaced in 'in'.
/// @param to The string with which all occurrences of 'from' should be replaced.
/// @return The resulting string where the replacement has been performed.
  static string replaceAll(const string &in, const string &from, const string &to);

// -------------------------------------------------------------------------------------------
///
/// @brief Replaces all occurrences of a substring in a string with another substring.
///
/// This method replaces all occurrences of the string 'from' with the string 'to' in the
/// string 'in'. If 'from' is the empty string, nothing is replaced.
///
/// The only difference to @link #replaceAll replaceAll @endlink is that this method
/// modifies the passed string.
///
/// @param in The string in which the replacement should be made.
/// @param from The string from which all occurrences should be replaced in 'in'.
/// @param to The string with which all occurrences of 'from' should be replaced.
  static void replaceAllIn(string &in, const string &from, const string &to);

// -------------------------------------------------------------------------------------------
///
/// @brief Tokenizes a string with a given list of delimiters.
///
/// @param in The string to tokenize.
/// @param tok The container to add the found tokens to.
/// @param delim A string where every character is seen as a delimiter. The default is
///        to treat all whitespaces as delimiters.
  static void tokenize(const string &in, vector<string> &tok, const string &delim = WS);

// -------------------------------------------------------------------------------------------
///
/// @brief Splits a string into its lines (separated by \\n or \\r).
///
/// Emtpy lines are ignored.
///
/// @param in The string to split.
/// @param lines The container to add the found lines to. Empty lines are not added.
/// @param keep_newlines True if the newline characters should be kept, false otherwise.
  static void splitLines(const string &in, vector<string> &lines, bool keep_newlines);

// -------------------------------------------------------------------------------------------
///
/// @brief Destructor.
  virtual ~StringUtils();

private:

// -------------------------------------------------------------------------------------------
///
/// @brief Constructor.
///
/// The constructor is private and not implemented. Use the static methods.
  StringUtils();

// -------------------------------------------------------------------------------------------
///
/// @brief Copy constructor.
///
/// The copy constructor is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
  StringUtils(const StringUtils &other);

// -------------------------------------------------------------------------------------------
///
/// @brief Assignment operator.
///
/// The assignment operator is disabled (set private) and not implemented.
///
/// @param other The source for creating the copy.
/// @return The result of the assignment, i.e, *this.
  StringUtils &operator=(const StringUtils &other);

};

#endif // StringUtils_H__

