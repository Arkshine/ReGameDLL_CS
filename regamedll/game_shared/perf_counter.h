﻿/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef PERF_COUNTER_H
#define PERF_COUNTER_H
#ifdef _WIN32
#pragma once
#endif

#ifdef _WIN32
	#include <windows.h>
	#include <io.h>
	#include <direct.h>
#else
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <fcntl.h>
	#include <unistd.h>
	#ifdef OSX
		#include <limits.h>
	#else
		#include <linux/limits.h>
	#endif
	#include <sys/time.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

class CPerformanceCounter
{
public:
	CPerformanceCounter(void);

	void InitializePerformanceCounter(void);
	double GetCurTime(void);

private:
	int m_iLowShift;
	double m_flPerfCounterFreq;
	double m_flCurrentTime;
	double m_flLastCurrentTime;

};/* size: 28, cachelines: 1, members: 4 */

/* <2ebfc> ../game_shared/perf_counter.h:61 */
inline CPerformanceCounter::CPerformanceCounter(void) :
	m_iLowShift(0),
	m_flPerfCounterFreq(0),
	m_flCurrentTime(0),
	m_flLastCurrentTime(0)
{
	InitializePerformanceCounter();
}

/* <2ebdd> ../game_shared/perf_counter.h:69 */
inline void CPerformanceCounter::InitializePerformanceCounter(void)
{
#ifdef _WIN32

	LARGE_INTEGER performanceFreq;
	QueryPerformanceFrequency(&performanceFreq);

	// get 32 out of the 64 time bits such that we have around
	// 1 microsecond resolution
	unsigned int lowpart, highpart;
	lowpart = (unsigned int)performanceFreq.LowPart;
	highpart = (unsigned int)performanceFreq.HighPart;
	m_iLowShift = 0;

	while (highpart || (lowpart > 2000000.0))
	{
		m_iLowShift++;
		lowpart >>= 1;
		lowpart |= (highpart & 1) << 31;
		highpart >>= 1;
	}

	m_flPerfCounterFreq = 1.0 / (double)lowpart;

#endif // _WIN32
}

/* <2ec16> ../game_shared/perf_counter.h:97 */
inline double CPerformanceCounter::GetCurTime(void)
{
#ifdef _WIN32

	static int sametimecount;
	static unsigned int oldtime;
	static int first = 1;
	LARGE_INTEGER PerformanceCount;
	unsigned int temp, t2;
	double time;

	QueryPerformanceCounter(&PerformanceCount);
	if (m_iLowShift == 0)
	{
		temp = (unsigned int)PerformanceCount.LowPart;
	}
	else
	{
		temp = ((unsigned int)PerformanceCount.LowPart >> m_iLowShift) |
			((unsigned int)PerformanceCount.HighPart << (32 - m_iLowShift));
	}

	if (first)
	{
		oldtime = temp;
		first = 0;
	}
	else
	{
		// check for turnover or backward time
		if ((temp <= oldtime) && ((oldtime - temp) < 0x10000000))
		{
			// so we can't get stuck
			oldtime = temp;
		}
		else
		{
			t2 = temp - oldtime;

			time = (double)t2 * m_flPerfCounterFreq;
			oldtime = temp;

			m_flCurrentTime += time;

			if (m_flCurrentTime == m_flLastCurrentTime)
			{
				sametimecount++;

				if (sametimecount > 100000)
				{
					m_flCurrentTime += 1.0;
					sametimecount = 0;
				}
			}
			else
			{
				sametimecount = 0;
			}

			m_flLastCurrentTime = m_flCurrentTime;
		}
	}

	return m_flCurrentTime;

#else // _WIN32

	struct timeval tp;
	static int secbase = 0;

	gettimeofday(&tp, NULL);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return (tp.tv_usec / 1000000.0);
	}

	return ((tp.tv_sec - secbase) + tp.tv_usec / 1000000.0);

#endif // _WIN32
}

#endif // PERF_COUNTER_H
