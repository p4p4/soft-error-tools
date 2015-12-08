// ----------------------------------------------------------------------------
// Copyright (c) 2015 by Graz University of Technology
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
//
// ----------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
/// @file SuperFluousTrace.h
/// @brief Contains the declaration of the class SuperFluousTrace.
// -------------------------------------------------------------------------------------------

#ifndef SuperFluousTrace_H__
#define SuperFluousTrace_H__

#include "defines.h"
	class SuperfluousTrace
	{
	public:
		int component_=0;
		unsigned flip_timestep_=0;
		unsigned alarm_timestep_=0;
		unsigned error_gone_timestep_=0;
		TestCase& testcase_;

		SuperfluousTrace(TestCase& testcase)  : testcase_(testcase) {}
		SuperfluousTrace(int component, TestCase& testcase,  unsigned flip_timestep, unsigned alarm_timestep, unsigned error_gone_ts) : testcase_(testcase)
		{
			component_ = component;
			flip_timestep_ = flip_timestep;
			alarm_timestep_ = alarm_timestep;
			error_gone_timestep_ = error_gone_ts;
		}

		string toString()
		{
			ostringstream oss;
			oss << "component=" << component_ << " flip_timestep=" << flip_timestep_ << ", alarm_timestep=" << alarm_timestep_ << ",error_gone_ts=" << error_gone_timestep_;

			return oss.str();
		}


	};

#endif // SuperFluousTrace_H__
