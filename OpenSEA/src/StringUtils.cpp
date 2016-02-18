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
/// @file StringUtils.cpp
/// @brief Contains the definition of the class StringUtils.
// -------------------------------------------------------------------------------------------

#include "StringUtils.h"

// -------------------------------------------------------------------------------------------
const string StringUtils::WS = " \t\r\n";

// -------------------------------------------------------------------------------------------
StringUtils::~StringUtils()
{
  // nothing to be done
}

// -------------------------------------------------------------------------------------------
string StringUtils::toUpperCase(const string &in)
{
  string in_copy = in;
  toUpperCaseIn(in_copy);
  return in_copy;
}

// -------------------------------------------------------------------------------------------
void StringUtils::toUpperCaseIn(string &in)
{
  std::transform(in.begin(), in.end(), in.begin(), ::toupper);
}

// -------------------------------------------------------------------------------------------
string StringUtils::toLowerCase(const string &in)
{
  string in_copy = in;
  toLowerCaseIn(in_copy);
  return in_copy;
}

// -------------------------------------------------------------------------------------------
void StringUtils::toLowerCaseIn(string &in)
{
  std::transform(in.begin(), in.end(), in.begin(), ::tolower);
}

// -------------------------------------------------------------------------------------------
string StringUtils::trim(const string &in, const string &chars)
{
  string in_copy = in;
  trimIn(in_copy, chars);
  return in_copy;
}

// -------------------------------------------------------------------------------------------
string StringUtils::lTrim(const string &in, const string &chars)
{
  string in_copy = in;
  lTrimIn(in_copy, chars);
  return in_copy;
}

// -------------------------------------------------------------------------------------------
string StringUtils::rTrim(const string &in, const string &chars)
{
  string in_copy = in;
  rTrimIn(in_copy, chars);
  return in_copy;
}

// -------------------------------------------------------------------------------------------
void StringUtils::trimIn(string &in, const string &chars)
{
  lTrimIn(in, chars);
  rTrimIn(in, chars);
}

// -------------------------------------------------------------------------------------------
void StringUtils::lTrimIn(string &in, const string &chars)
{
  size_t new_start = in.find_first_not_of(chars);
  if(new_start == string::npos)
    in = "";
  else
    in = in.substr(new_start);
}

// -------------------------------------------------------------------------------------------
void StringUtils::rTrimIn(string &in, const string &chars)
{
  size_t new_end = in.find_last_not_of(chars);
  if(new_end == string::npos)
    in = "";
  else
    in = in.substr(0, new_end + 1);
}

// -------------------------------------------------------------------------------------------
size_t StringUtils::count(const string &in, const string &what)
{
  size_t sizeof_what = what.size();
  if(sizeof_what == 0)
    return 0;
  size_t count = 0;
  size_t pos = in.find(what);
  while(pos != string::npos)
  {
    ++count;
    pos = in.find(what, pos + sizeof_what);
  }
  return count;
}

// -------------------------------------------------------------------------------------------
string StringUtils::replaceAll(const string &in, const string &from, const string &to)
{
  string in_copy = in;
  replaceAllIn(in_copy, from, to);
  return in_copy;
}

// -------------------------------------------------------------------------------------------
void StringUtils::replaceAllIn(string &in, const string &from, const string &to)
{
  size_t sizeof_from = from.size();
  if(sizeof_from == 0)
    return;
  size_t sizeof_to = to.size();
  size_t pos = in.find(from);
  while(pos != string::npos)
  {
    in.replace(pos, sizeof_from, to);
    pos = in.find(from, pos + sizeof_to);
  }
}

// -------------------------------------------------------------------------------------------
void StringUtils::tokenize(const string &in, vector<string> &tok, const string &delim)
{
  size_t start = in.find_first_not_of(delim, 0);
  size_t end = in.find_first_of(delim, start);
  while(start != string::npos || end != string::npos)
  {
    tok.push_back(in.substr(start, end - start));
    start = in.find_first_not_of(delim, end);
    end = in.find_first_of(delim, start);
  }
}

// -------------------------------------------------------------------------------------------
void StringUtils::splitLines(const string &in, vector<string> &lines, bool keep_newlines)
{
  tokenize(in, lines, "\n\r");
  if(keep_newlines)
    for(size_t count = 0; count < lines.size(); ++count)
      lines[count] += "\n";
}

