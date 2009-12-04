/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
	CICalendarRecurrence.cpp

	Author:
	Description:	<describe the CICalendarRecurrence class here>
*/

#include "CICalendarRecurrence.h"

#include "CICalendarPeriod.h"
#include "CStringUtils.h"

#include <cerrno>
#include <algorithm>
#include <memory>
#include <strstream>

using namespace iCal;

void CICalendarRecurrence::_init_CICalendarRecurrence()
{
	mFreq = eRecurrence_YEARLY;

	mUseCount = false;
	mCount = 0;

	mUseUntil = false;

	mInterval = 1;
	mBySeconds.clear();
	mByMinutes.clear();
	mByHours.clear();
	mByDay.clear();
	mByMonthDay.clear();
	mByYearDay.clear();
	mByWeekNo.clear();
	mByMonth.clear();
	mBySetPos.clear();
	mWeekstart = eRecurrence_WEEKDAY_MO;

	mCached = false;
	mFullyCached = false;
	mRecurrences.clear();
}

void CICalendarRecurrence::_copy_CICalendarRecurrence(const CICalendarRecurrence& copy)
{
	_init_CICalendarRecurrence();

	mFreq = copy.mFreq;

	mUseCount = copy.mUseCount;
	mCount = copy.mCount;
	mUseUntil = copy.mUseUntil;
	mUntil = copy.mUntil;

	mInterval = copy.mInterval;
	mBySeconds = copy.mBySeconds;
	mByMinutes = copy.mByMinutes;
	mByHours = copy.mByHours;
	mByDay = copy.mByDay;
	mByMonthDay = copy.mByMonthDay;
	mByYearDay = copy.mByYearDay;
	mByWeekNo = copy.mByWeekNo;
	mByMonth = copy.mByMonth;
	mBySetPos = copy.mBySetPos;
	mWeekstart = copy.mWeekstart;

	mCached = copy.mCached;
	mCacheStart = copy.mCacheStart;
	mCacheUpto = copy.mCacheUpto;
	mFullyCached = copy.mFullyCached;
	mRecurrences = copy.mRecurrences;
}

bool CICalendarRecurrence::Equals(const CICalendarRecurrence& comp) const
{
	return (mFreq == comp.mFreq) &&
			(mCount == comp.mCount) &&
			(mUseUntil == comp.mUseUntil) &&
			(mUntil == comp.mUntil) &&
			(mInterval == comp.mInterval) &&
			Equals(mBySeconds, comp.mBySeconds) &&
			Equals(mByMinutes, comp.mByMinutes) &&
			Equals(mByHours, comp.mByHours) &&
			Equals(mByDay, comp.mByDay) &&
			Equals(mByMonthDay, comp.mByMonthDay) &&
			Equals(mByYearDay, comp.mByYearDay) &&
			Equals(mByWeekNo, comp.mByWeekNo) &&
			Equals(mByMonth, comp.mByMonth) &&
			Equals(mBySetPos, comp.mBySetPos) &&
			(mWeekstart == comp.mWeekstart);			
}

bool CICalendarRecurrence::Equals(const std::vector<int32_t>& items1, const std::vector<int32_t>& items2) const
{
	// Check sizes first
	if (items1.size() != items2.size())
		return false;
	else if (items1.size() == 0)
		return true;

	// Copy and sort each one for comparison
	std::vector<int32_t> temp1(items1);
	std::vector<int32_t> temp2(items2);
	std::sort(temp1.begin(), temp1.end());
	std::sort(temp2.begin(), temp2.end());
	
	return std::equal(temp1.begin(), temp1.end(), temp2.begin());
}

bool CICalendarRecurrence::Equals(const std::vector<CWeekDayNum>& items1, const std::vector<CWeekDayNum>& items2) const
{
	// Check sizes first
	if (items1.size() != items2.size())
		return false;
	else if (items1.size() == 0)
		return true;

	// Copy and sort each one for comparison
	std::vector<CWeekDayNum> temp1(items1);
	std::vector<CWeekDayNum> temp2(items2);
	std::sort(temp1.begin(), temp1.end());
	std::sort(temp2.begin(), temp2.end());
	
	return std::equal(temp1.begin(), temp1.end(), temp2.begin());

}

const char* cFreqMap[] = {	cICalValue_RECUR_SECONDLY,
							cICalValue_RECUR_MINUTELY, 
							cICalValue_RECUR_HOURLY,
							cICalValue_RECUR_DAILY,
							cICalValue_RECUR_WEEKLY,
							cICalValue_RECUR_MONTHLY,
							cICalValue_RECUR_YEARLY,
							NULL};

const char* cRecurMap[] = {	cICalValue_RECUR_UNTIL,
							cICalValue_RECUR_COUNT,
							cICalValue_RECUR_INTERVAL,
							cICalValue_RECUR_BYSECOND,
							cICalValue_RECUR_BYMINUTE,
							cICalValue_RECUR_BYHOUR,
							cICalValue_RECUR_BYDAY,
							cICalValue_RECUR_BYMONTHDAY,
							cICalValue_RECUR_BYYEARDAY,
							cICalValue_RECUR_BYWEEKNO,
							cICalValue_RECUR_BYMONTH,
							cICalValue_RECUR_BYSETPOS,
							cICalValue_RECUR_WKST,
							NULL};

const char* cWeekdayMap[] = {	cICalValue_RECUR_WEEKDAY_SU,
								cICalValue_RECUR_WEEKDAY_MO, 
								cICalValue_RECUR_WEEKDAY_TU,
								cICalValue_RECUR_WEEKDAY_WE,
								cICalValue_RECUR_WEEKDAY_TH,
								cICalValue_RECUR_WEEKDAY_FR,
								cICalValue_RECUR_WEEKDAY_SA,
								NULL};

const unsigned long cUnknownIndex = 0xFFFFFFFF;

void CICalendarRecurrence::Parse(const cdstring& data)
{
	_init_CICalendarRecurrence();

	// Tokenise using ';'
	char* p = const_cast<char*>(data.c_str());

	// Look for FREQ= with delimiter
	{
		std::auto_ptr<char> freq(::strduptokenstr(&p, ";"));
		if (freq.get() == NULL)
			return;

		// Make sure it is the token we expect
		if (::strncmp(freq.get(), cICalValue_RECUR_FREQ, cICalValue_RECUR_FREQ_LEN) != 0)
			return;
		const char* q = freq.get() + cICalValue_RECUR_FREQ_LEN;

		// Get the FREQ value
		unsigned long index = ::strindexfind(q, cFreqMap, cUnknownIndex);
		if (index == cUnknownIndex)
			return;
		mFreq = static_cast<ERecurrence_FREQ>(index);
	}

	while(*p)
	{
		// Get next token
		p++;
		std::auto_ptr<char> item(::strduptokenstr(&p, ";"));
		if (item.get() == NULL)
			return;
		
		// Determine token type
		unsigned long index = ::strnindexfind(item.get(), cRecurMap, cUnknownIndex);
		if (index == cUnknownIndex)
			return;
		
		// Parse remainder based on index
		const char* q = strchr(item.get(), '=') + 1;
		
		switch(index)
		{
		case 0:		// UNTIL
			if (mUseCount)
				return;
			mUseUntil = true;
			mUntil.Parse(q);
			break;
		case 1:		// COUNT
			if (mUseUntil)
				return;
			mUseCount = true;
			mCount = atol(q);
			
			// Must not be less than one
			if (mCount < 1)
				mCount = 1;
			break;
		case 2:		// INTERVAL
			mInterval = atol(q);
			
			// Must NOT be less than one
			if (mInterval < 1)
				mInterval = 1;
			break;
		case 3:		// BYSECOND
			if (!mBySeconds.empty())
				return;
			ParseList(q, mBySeconds);
			break;
		case 4:		// BYMINUTE
			if (!mByMinutes.empty())
				return;
			ParseList(q, mByMinutes);
			break;
		case 5:		// BYHOUR
			if (!mByHours.empty())
				return;
			ParseList(q, mByHours);
			break;
		case 6:		// BYDAY
			if (!mByDay.empty())
				return;
			ParseList(q, mByDay);
			break;
		case 7:		// BYMONTHDAY
			if (!mByMonthDay.empty())
				return;
			ParseList(q, mByMonthDay);
			break;
		case 8:		// BYYEARDAY
			if (!mByYearDay.empty())
				return;
			ParseList(q, mByYearDay);
			break;
		case 9:		// BYWEEKNO
			if (!mByWeekNo.empty())
				return;
			ParseList(q, mByWeekNo);
			break;
		case 10:	// BYMONTH
			if (!mByMonth.empty())
				return;
			ParseList(q, mByMonth);
			break;
		case 11:	// BYSETPOS
			if (!mBySetPos.empty())
				return;
			ParseList(q, mBySetPos);
			break;
		case 12:	// WKST
			{
				unsigned long index = ::strindexfind(q, cWeekdayMap, cUnknownIndex);
				if (index == cUnknownIndex)
					return;
				mWeekstart = static_cast<ERecurrence_WEEKDAY>(index);
			}
			break;
		}
	}
}

// Parse comma separated list of int32_t's
void CICalendarRecurrence::ParseList(const char* txt, std::vector<int32_t>& list)
{
	const char* p = txt;
	
	while(p && *p)
	{
		errno = 0;
		int32_t num = strtol(p, (char**) &p, 10);
		if (errno != 0)
			return;
		list.push_back(num);
		if (*p++ != ',')
			p = NULL;
	}
}

void CICalendarRecurrence::ParseList(const char* txt, std::vector<CWeekDayNum>& list)
{
	const char* p = txt;
	
	while(p && *p)
	{
		// Get number if present
		errno = 0;
		int32_t num = 0;
		if (isdigit(*p) || (*p == '+') || (*p == '-'))
		{
			num = strtol(p, (char**) &p, 10);
			if (errno != 0)
				return;
		}
		
		// Get day
		unsigned long index = ::strnindexfind(p, cWeekdayMap, cUnknownIndex);
		if (index == cUnknownIndex)
			return;
		CICalendarDateTime::EDayOfWeek wday = static_cast<CICalendarDateTime::EDayOfWeek>(index);
		p += 2;
		
		list.push_back(CWeekDayNum(num, wday));
		if (*p++ != ',')
			p = NULL;
	}
}

void CICalendarRecurrence::Generate(std::ostream& os) const
{
	os << cICalValue_RECUR_FREQ;
	switch(mFreq)
	{
	case eRecurrence_SECONDLY:
		os << cICalValue_RECUR_SECONDLY;
		break;
	case eRecurrence_MINUTELY:
		os << cICalValue_RECUR_MINUTELY;
		break;
	case eRecurrence_HOURLY:
		os << cICalValue_RECUR_HOURLY;
		break;
	case eRecurrence_DAILY:
		os << cICalValue_RECUR_DAILY;
		break;
	case eRecurrence_WEEKLY:
		os << cICalValue_RECUR_WEEKLY;
		break;
	case eRecurrence_MONTHLY:
		os << cICalValue_RECUR_MONTHLY;
		break;
	case eRecurrence_YEARLY:
		os << cICalValue_RECUR_YEARLY;
		break;
	}

	if (mUseCount)
		os << ';' << cICalValue_RECUR_COUNT << mCount;
	else if (mUseUntil)
	{
		os << ';' << cICalValue_RECUR_UNTIL;
		mUntil.Generate(os);
	}

	if (mInterval > 1)
		os << ';' << cICalValue_RECUR_INTERVAL << mInterval;

	if (mBySeconds.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYSECOND;
		for(std::vector<int32_t>::const_iterator iter = mBySeconds.begin(); iter != mBySeconds.end(); iter++)
		{
			if (iter != mBySeconds.begin())
				os << ',';
			os << *iter;
		}
	}

	if (mByMinutes.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYMINUTE;
		for(std::vector<int32_t>::const_iterator iter = mByMinutes.begin(); iter != mByMinutes.end(); iter++)
		{
			if (iter != mByMinutes.begin())
				os << ',';
			os << *iter;
		}
	}

	if (mByHours.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYHOUR;
		for(std::vector<int32_t>::const_iterator iter = mByHours.begin(); iter != mByHours.end(); iter++)
		{
			if (iter != mByHours.begin())
				os << ',';
			os << *iter;
		}
	}

	if (mByDay.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYDAY;
		for(std::vector<CWeekDayNum>::const_iterator iter = mByDay.begin(); iter != mByDay.end(); iter++)
		{
			if (iter != mByDay.begin())
				os << ',';
			if ((*iter).first != 0)
				os << (*iter).first;
			switch((*iter).second)
			{
			case eRecurrence_WEEKDAY_SU:
				os << cICalValue_RECUR_WEEKDAY_SU;
				break;
			case eRecurrence_WEEKDAY_MO:
				os << cICalValue_RECUR_WEEKDAY_MO;
				break;
			case eRecurrence_WEEKDAY_TU:
				os << cICalValue_RECUR_WEEKDAY_TU;
				break;
			case eRecurrence_WEEKDAY_WE:
				os << cICalValue_RECUR_WEEKDAY_WE;
				break;
			case eRecurrence_WEEKDAY_TH:
				os << cICalValue_RECUR_WEEKDAY_TH;
				break;
			case eRecurrence_WEEKDAY_FR:
				os << cICalValue_RECUR_WEEKDAY_FR;
				break;
			case eRecurrence_WEEKDAY_SA:
				os << cICalValue_RECUR_WEEKDAY_SA;
				break;
			}
		}
	}

	if (mByMonthDay.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYMONTHDAY;
		for(std::vector<int32_t>::const_iterator iter = mByMonthDay.begin(); iter != mByMonthDay.end(); iter++)
		{
			if (iter != mByMonthDay.begin())
				os << ',';
			os << *iter;
		}
	}

	if (mByYearDay.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYYEARDAY;
		for(std::vector<int32_t>::const_iterator iter = mByYearDay.begin(); iter != mByYearDay.end(); iter++)
		{
			if (iter != mByYearDay.begin())
				os << ',';
			os << *iter;
		}
	}

	if (mByWeekNo.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYWEEKNO;
		for(std::vector<int32_t>::const_iterator iter = mByWeekNo.begin(); iter != mByWeekNo.end(); iter++)
		{
			if (iter != mByWeekNo.begin())
				os << ',';
			os << *iter;
		}
	}

	if (mByMonth.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYMONTH;
		for(std::vector<int32_t>::const_iterator iter = mByMonth.begin(); iter != mByMonth.end(); iter++)
		{
			if (iter != mByMonth.begin())
				os << ',';
			os << *iter;
		}
	}

	if (mBySetPos.size() != 0)
	{
		os << ';' << cICalValue_RECUR_BYSETPOS;
		for(std::vector<int32_t>::const_iterator iter = mBySetPos.begin(); iter != mBySetPos.end(); iter++)
		{
			if (iter != mBySetPos.begin())
				os << ',';
			os << *iter;
		}
	}

	if (mWeekstart != eRecurrence_WEEKDAY_MO)	// MO is the default so we do not need it
	{
		os << ';' << cICalValue_RECUR_WKST;
		switch(mWeekstart)
		{
		case eRecurrence_WEEKDAY_SU:
			os << cICalValue_RECUR_WEEKDAY_SU;
			break;
		case eRecurrence_WEEKDAY_MO:
			os << cICalValue_RECUR_WEEKDAY_MO;
			break;
		case eRecurrence_WEEKDAY_TU:
			os << cICalValue_RECUR_WEEKDAY_TU;
			break;
		case eRecurrence_WEEKDAY_WE:
			os << cICalValue_RECUR_WEEKDAY_WE;
			break;
		case eRecurrence_WEEKDAY_TH:
			os << cICalValue_RECUR_WEEKDAY_TH;
			break;
		case eRecurrence_WEEKDAY_FR:
			os << cICalValue_RECUR_WEEKDAY_FR;
			break;
		case eRecurrence_WEEKDAY_SA:
			os << cICalValue_RECUR_WEEKDAY_SA;
			break;
		}
	}
}

// Is rule capable of simple UI display
bool CICalendarRecurrence::IsSimpleRule() const
{
	// One that has no BYxxx rules
	return !HasBy();
}

// Is rule capable of advanced UI display
bool CICalendarRecurrence::IsAdvancedRule() const
{
	// One that has BYMONTH,
	// BYMONTHDAY (with no negative value),
	// BYDAY (with multiple unumbered, or numbered with all the same number (1..4, -2, -1)
	// BYSETPOS with +1, or -1 only
	// no others
	
	// First checks the ones we do not handle at all
	if (!mBySeconds.empty() || !mByMinutes.empty() || !mByHours.empty() || !mByYearDay.empty() || !mByWeekNo.empty())
		return false;
	
	// Check BYMONTHDAY numbers (we can handle -7...-1, 1..31)
	for(std::vector<int32_t>::const_iterator iter = mByMonthDay.begin(); iter != mByMonthDay.end(); iter++)
	{
		if ((*iter < -7) || (*iter > 31) || (*iter == 0))
			return false;
	}
	
	// Check BYDAY numbers
	int32_t number = 0;
	for(std::vector<CWeekDayNum>::const_iterator iter = mByDay.begin(); iter != mByDay.end(); iter++)
	{
		// Get the first number
		if (iter == mByDay.begin())
		{
			number = (*iter).first;
			
			// Check number range
			if ((number > 4) || (number < -2))
				return false;
		}
		
		// If current differs from last, then we have an error
		else if (number != (*iter).first)
			return false;
	}
	
	// Check BYSETPOS numbers
	if (mBySetPos.size() > 1)
		return false;
	if ((mBySetPos.size() == 1) && (mBySetPos[0] != -1) && (mBySetPos[0] != 1))
		return false;

	// If we get here it must be OK
	return true;
}

cdstring CICalendarRecurrence::GetUIDescription() const
{
	// For now just use iCal item
	std::ostrstream ostr;
	Generate(ostr);
	ostr << std::ends;
	cdstring temp;
	temp.steal(ostr.str());
	return temp;
}

void CICalendarRecurrence::Expand(const CICalendarDateTime& start, const CICalendarPeriod& range, CICalendarDateTimeList& items) const
{
	// Wipe cache if start is different
	if (mCached && (start != mCacheStart))
	{
		mCached = false;
		mFullyCached = false;
		mRecurrences.clear();
	}

	// Is the current cache complete or does it extaned past the requested range end
	if (!mCached || !mFullyCached && (mCacheUpto < range.GetEnd()))
	{
		CICalendarPeriod cache_range(range);
		
		// If partially cached just cache from previous cache end up to new end
		if (mCached)
			cache_range = CICalendarPeriod(mCacheUpto, range.GetEnd());
		
		// Simple expansion is one where there is no BYXXX rule part
		if (mBySeconds.empty() && mByMinutes.empty() && mByHours.empty() &&
			mByDay.empty() && mByMonthDay.empty() && mByYearDay.empty() &&
			mByWeekNo.empty() && mByMonth.empty() && mBySetPos.empty())
			mFullyCached = SimpleExpand(start, cache_range, mRecurrences);
		else
			mFullyCached = ComplexExpand(start, cache_range, mRecurrences);
		
		// Set cache values
		mCached = true;
		mCacheStart = start;
		mCacheUpto = range.GetEnd();
	}
	
	// Just return the cached items in the requested range
	for(CICalendarDateTimeList::const_iterator iter = mRecurrences.begin(); iter != mRecurrences.end(); iter++)
	{
		if (range.IsDateWithinPeriod(*iter))
			items.push_back(*iter);
	}
}

bool CICalendarRecurrence::SimpleExpand(const CICalendarDateTime& start, const CICalendarPeriod& range, CICalendarDateTimeList& items) const
{
	CICalendarDateTime start_iter(start);
	int32_t ctr = 0;

	while(true)
	{
		// Exit if after period we want
		if (range.IsDateAfterPeriod(start_iter))
			return false;

		// Add current one to list
		items.push_back(start_iter);
		
		// Get next item
		start_iter.Recur(mFreq, mInterval);

		// Check limits
		if (mUseCount)
		{
			// Bump counter and exit if over
			ctr++;
			if (ctr >= mCount)
				return true;
		}
		else if (mUseUntil)
		{
			// Exit if next item is after until (its OK if its the same as UNTIL as
			// UNTIL is inclusive)
			if (start_iter > mUntil)
				return true;
		}
	}
}

bool CICalendarRecurrence::ComplexExpand(const CICalendarDateTime& start, const CICalendarPeriod& range, CICalendarDateTimeList& items) const
{
	CICalendarDateTime start_iter(start);
	int32_t ctr = 0;

	// Always add the initial instance DTSTART
	items.push_back(start);
	if (mUseCount)
	{
		// Bump counter and exit if over
		ctr++;
		if (ctr >= mCount)
			return true;
	}

	// Need to re-initialise start based on BYxxx rules
	while(true)
	{
		// Behaviour is based on frequency
		CICalendarDateTimeList set_items;
		switch(mFreq)
		{
		case eRecurrence_SECONDLY:
			GenerateSecondlySet(start_iter, set_items);
			break;
		case eRecurrence_MINUTELY:
			GenerateMinutelySet(start_iter, set_items);
			break;
		case eRecurrence_HOURLY:
			GenerateHourlySet(start_iter, set_items);
			break;
		case eRecurrence_DAILY:
			GenerateDailySet(start_iter, set_items);
			break;
		case eRecurrence_WEEKLY:
			GenerateWeeklySet(start_iter, set_items);
			break;
		case eRecurrence_MONTHLY:
			GenerateMonthlySet(start_iter, set_items);
			break;
		case eRecurrence_YEARLY:
			GenerateYearlySet(start_iter, set_items);
			break;
		}

		// Always sort the set as BYxxx rules may not be sorted
		sort(set_items.begin(), set_items.end());

		// Process each one in the generated set
		for(CICalendarDateTimeList::const_iterator iter = set_items.begin(); iter != set_items.end(); iter++)
		{
			// Ignore if it is before the actual start - we need this because the expansion
			// can go back in time from the real start, but we must exclude those when counting
			// even if they are not within the requested range
			if (*iter < start)
				continue;

			// Exit if after period we want
			if (range.IsDateAfterPeriod(*iter))
				return false;
			
			// Exit if beyond the UNTIL limit
			if (mUseUntil)
			{
				// Exit if next item is after until (its OK if its the same as UNTIL as
				// UNTIL is inclusive)
				if (*iter > mUntil)
					return true;
			}

			// Special for start instance
			if ((ctr == 1) && (start == *iter))
				continue;

			// Add current one to list
			items.push_back(*iter);
			
			// Check limits
			if (mUseCount)
			{
				// Bump counter and exit if over
				ctr++;
				if (ctr >= mCount)
					return true;
			}
		}

		// Exit if after period we want
		if (range.IsDateAfterPeriod(start_iter))
			return false;
		
		// Get next item
		start_iter.Recur(mFreq, mInterval);
	}
}

// Clear out cached values due to some sort of change
void CICalendarRecurrence::Clear()
{
	mCached = false;
	mFullyCached = false;
	mRecurrences.clear();
}

// IMPORTANT ExcludeFutureRecurrence assumes mCacheStart is setup with the owning VEVENT's DTSTART
// Currently this method is only called when a recurrence is being removed so the recurrence data should be cached

// Exclude dates on or after the chosen one
void CICalendarRecurrence::ExcludeFutureRecurrence(const CICalendarDateTime& exclude)
{
	// Expand the rule upto the exclude date
	CICalendarDateTimeList items;
	Expand(mCacheStart,CICalendarPeriod(mCacheStart, exclude), items);
	
	// Adjust UNTIL or add one if no COUNT
	if (GetUseUntil() || !GetUseCount())
	{
		// The last one is just less than the exclude date
		if (items.size() != 0)
		{
			// Now use the data as the UNTIL
			mUseUntil = true;
			mUntil = items.back();
		}
	}
	
	// Adjust COUNT
	else if (GetUseCount())
	{
		// The last one is just less than the exclude date
		mUseCount = true;
		mCount = items.size();
	}
	
	// Now clear out the cached set after making changes
	Clear();
}

#pragma mark ____________________________Generate Complex recurrence based on frequency

void CICalendarRecurrence::GenerateYearlySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const
{
	// All possible BYxxx are valid, though some combinations are not

	// Start with initial date-time
	items.push_back(start);

	if (mByMonth.size() != 0)
	{
		ByMonthExpand(items);
	}
	
	if (mByWeekNo.size() != 0)
	{
		ByWeekNoExpand(items);
	}
	
	if (mByYearDay.size() != 0)
	{
		ByYearDayExpand(items);
	}
	
	if (mByMonthDay.size() != 0)
	{
		ByMonthDayExpand(items);
	}
	
	if (mByDay.size() != 0)
	{
		// BYDAY is complicated:
		//   if BYDAY is included with BYYEARDAY or BYMONTHDAY then it contracts the recurrence set
		//   else it expands it, but the expansion depends on the frequency and other BYxxx periodicities
		
		if ((mByYearDay.size() != 0) || (mByMonthDay.size() != 0))
			ByDayLimit(items);
		else if (mByWeekNo.size() != 0)
			ByDayExpandWeekly(items);
		else if (mByMonth.size() != 0)
			ByDayExpandMonthly(items);
		else
			ByDayExpandYearly(items);
	}
	
	if (mByHours.size() != 0)
	{
		ByHourExpand(items);
	}
	
	if (mByMinutes.size() != 0)
	{
		ByMinuteExpand(items);
	}

	if (mBySeconds.size() != 0)
	{
		BySecondExpand(items);
	}
	
	if (mBySetPos.size() != 0)
	{
		BySetPosLimit(items);
	}
}

void CICalendarRecurrence::GenerateMonthlySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const
{
	// Cannot have BYYEARDAY and BYWEEKNO

	// Start with initial date-time
	items.push_back(start);

	if (mByMonth.size() != 0)
	{
		// BYMONTH limits the range of possible values
		ByMonthLimit(items);
		if (items.size() == 0)
			return;
	}
	
	// No BYWEEKNO
	
	// No BYYEARDAY
	
	if (mByMonthDay.size() != 0)
	{
		ByMonthDayExpand(items);
	}
	
	if (mByDay.size() != 0)
	{
		// BYDAY is complicated:
		//   if BYDAY is included with BYYEARDAY or BYMONTHDAY then it contracts the recurrence set
		//   else it expands it, but the expansion depends on the frequency and other BYxxx periodicities
		
		if ((mByYearDay.size() != 0) || (mByMonthDay.size() != 0))
			ByDayLimit(items);
		else
			ByDayExpandMonthly(items);
	}
	
	if (mByHours.size() != 0)
	{
		ByHourExpand(items);
	}
	
	if (mByMinutes.size() != 0)
	{
		ByMinuteExpand(items);
	}

	if (mBySeconds.size() != 0)
	{
		BySecondExpand(items);
	}
	
	if (mBySetPos.size() != 0)
	{
		BySetPosLimit(items);
	}
}

void CICalendarRecurrence::GenerateWeeklySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const
{
	// Cannot have BYYEARDAY and BYMONTHDAY

	// Start with initial date-time
	items.push_back(start);

	if (mByMonth.size() != 0)
	{
		// BYMONTH limits the range of possible values
		ByMonthLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByWeekNo.size() != 0)
	{
		ByWeekNoLimit(items);
		if (items.size() == 0)
			return;
	}
	
	// No BYYEARDAY
	
	// No BYMONTHDAY
	
	if (mByDay.size() != 0)
	{
		ByDayExpandWeekly(items);
	}
	
	if (mByHours.size() != 0)
	{
		ByHourExpand(items);
	}
	
	if (mByMinutes.size() != 0)
	{
		ByMinuteExpand(items);
	}

	if (mBySeconds.size() != 0)
	{
		BySecondExpand(items);
	}
	
	if (mBySetPos.size() != 0)
	{
		BySetPosLimit(items);
	}
}

void CICalendarRecurrence::GenerateDailySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const
{
	// Cannot have BYYEARDAY

	// Start with initial date-time
	items.push_back(start);

	if (mByMonth.size() != 0)
	{
		// BYMONTH limits the range of possible values
		ByMonthLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByWeekNo.size() != 0)
	{
		ByWeekNoLimit(items);
		if (items.size() == 0)
			return;
	}
	
	// No BYYEARDAY
	
	if (mByMonthDay.size() != 0)
	{
		ByMonthDayLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByDay.size() != 0)
	{
		ByDayLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByHours.size() != 0)
	{
		ByHourExpand(items);
	}
	
	if (mByMinutes.size() != 0)
	{
		ByMinuteExpand(items);
	}

	if (mBySeconds.size() != 0)
	{
		BySecondExpand(items);
	}
	
	if (mBySetPos.size() != 0)
	{
		BySetPosLimit(items);
	}
}

void CICalendarRecurrence::GenerateHourlySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const
{
	// Cannot have BYYEARDAY

	// Start with initial date-time
	items.push_back(start);

	if (mByMonth.size() != 0)
	{
		// BYMONTH limits the range of possible values
		ByMonthLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByWeekNo.size() != 0)
	{
		ByWeekNoLimit(items);
		if (items.size() == 0)
			return;
	}
	
	// No BYYEARDAY
	
	if (mByMonthDay.size() != 0)
	{
		ByMonthDayLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByDay.size() != 0)
	{
		ByDayLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByHours.size() != 0)
	{
		ByHourLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByMinutes.size() != 0)
	{
		ByMinuteExpand(items);
	}

	if (mBySeconds.size() != 0)
	{
		BySecondExpand(items);
	}
	
	if (mBySetPos.size() != 0)
	{
		BySetPosLimit(items);
	}
}

void CICalendarRecurrence::GenerateMinutelySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const
{
	// Cannot have BYYEARDAY

	// Start with initial date-time
	items.push_back(start);

	if (mByMonth.size() != 0)
	{
		// BYMONTH limits the range of possible values
		ByMonthLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByWeekNo.size() != 0)
	{
		ByWeekNoLimit(items);
		if (items.size() == 0)
			return;
	}
	
	// No BYYEARDAY
	
	if (mByMonthDay.size() != 0)
	{
		ByMonthDayLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByDay.size() != 0)
	{
		ByDayLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByHours.size() != 0)
	{
		ByHourLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByMinutes.size() != 0)
	{
		ByMinuteLimit(items);
		if (items.size() == 0)
			return;
	}

	if (mBySeconds.size() != 0)
	{
		BySecondExpand(items);
	}
	
	if (mBySetPos.size() != 0)
	{
		BySetPosLimit(items);
	}
}

void CICalendarRecurrence::GenerateSecondlySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const
{
	// Cannot have BYYEARDAY

	// Start with initial date-time
	items.push_back(start);

	if (mByMonth.size() != 0)
	{
		// BYMONTH limits the range of possible values
		ByMonthLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByWeekNo.size() != 0)
	{
		ByWeekNoLimit(items);
		if (items.size() == 0)
			return;
	}
	
	// No BYYEARDAY
	
	if (mByMonthDay.size() != 0)
	{
		ByMonthDayLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByDay.size() != 0)
	{
		ByDayLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByHours.size() != 0)
	{
		ByHourLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mByMinutes.size() != 0)
	{
		ByMinuteLimit(items);
		if (items.size() == 0)
			return;
	}

	if (mBySeconds.size() != 0)
	{
		BySecondLimit(items);
		if (items.size() == 0)
			return;
	}
	
	if (mBySetPos.size() != 0)
	{
		BySetPosLimit(items);
	}
}

#pragma mark ____________________________BYxxx expansions

void CICalendarRecurrence::ByMonthExpand(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYMONTH and generating a new date-time for it and insert into output
		for(std::vector<int32_t>::const_iterator iter2 = mByMonth.begin(); iter2 != mByMonth.end(); iter2++)
		{
			CICalendarDateTime temp(*iter1);
			temp.SetMonth(*iter2);
			output.push_back(temp);
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::ByWeekNoExpand(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYWEEKNO and generating a new date-time for it and insert into output
		for(std::vector<int32_t>::const_iterator iter2 = mByWeekNo.begin(); iter2 != mByWeekNo.end(); iter2++)
		{
			CICalendarDateTime temp(*iter1);
			temp.SetWeekNo(*iter2);
			output.push_back(temp);
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::ByYearDayExpand(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYYEARDAY and generating a new date-time for it and insert into output
		for(std::vector<int32_t>::const_iterator iter2 = mByYearDay.begin(); iter2 != mByYearDay.end(); iter2++)
		{
			CICalendarDateTime temp(*iter1);
			temp.SetYearDay(*iter2);
			output.push_back(temp);
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::ByMonthDayExpand(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYMONTHDAY and generating a new date-time for it and insert into output
		for(std::vector<int32_t>::const_iterator iter2 = mByMonthDay.begin(); iter2 != mByMonthDay.end(); iter2++)
		{
			CICalendarDateTime temp(*iter1);
			temp.SetMonthDay(*iter2);
			output.push_back(temp);
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::ByDayExpandYearly(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYDAY and generating a new date-time for it and insert into output
		for(std::vector<CWeekDayNum>::const_iterator iter2 = mByDay.begin(); iter2 != mByDay.end(); iter2++)
		{
			// Numeric value means specific instance
			if ((*iter2).first != 0)
			{
				CICalendarDateTime temp(*iter1);
				temp.SetDayOfWeekInYear((*iter2).first, (*iter2).second);
				output.push_back(temp);
			}
			else
			{
				// Every matching day in the year
				for(int32_t i = 1; i < 54; i++)
				{
					CICalendarDateTime temp(*iter1);
					temp.SetDayOfWeekInYear(i, (*iter2).second);
					if (temp.GetYear() == (*iter1).GetYear())
						output.push_back(temp);
				}
			}
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::ByDayExpandMonthly(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYDAY and generating a new date-time for it and insert into output
		for(std::vector<CWeekDayNum>::const_iterator iter2 = mByDay.begin(); iter2 != mByDay.end(); iter2++)
		{
			// Numeric value means specific instance
			if ((*iter2).first != 0)
			{
				CICalendarDateTime temp(*iter1);
				temp.SetDayOfWeekInMonth((*iter2).first, (*iter2).second);
				output.push_back(temp);
			}
			else
			{
				// Every matching day in the month
				for(int32_t i = 1; i < 6; i++)
				{
					CICalendarDateTime temp(*iter1);
					temp.SetDayOfWeekInMonth(i, (*iter2).second);
					if (temp.GetMonth() == (*iter1).GetMonth())
						output.push_back(temp);
				}
			}
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::ByDayExpandWeekly(CICalendarDateTimeList& dates) const
{
	// Must take into account the WKST value

	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYDAY and generating a new date-time for it and insert into output
		for(std::vector<CWeekDayNum>::const_iterator iter2 = mByDay.begin(); iter2 != mByDay.end(); iter2++)
		{
			// Numeric values are meaningless so ignore them
			if ((*iter2).first == 0)
			{
				CICalendarDateTime temp(*iter1);
				
				// Determine amount of offset to apply to temp to shift it to the start of the week (backwards)
				int32_t week_start_offset = mWeekstart - temp.GetDayOfWeek();
				if (week_start_offset > 0)
					week_start_offset -= 7;

				// Determine amount of offset from the start of the week to the day we want (forwards)
				int32_t day_in_week_offset = (*iter2).second - mWeekstart;
				if (day_in_week_offset < 0)
					day_in_week_offset += 7;
				
				// Apply offsets
				temp.OffsetDay(week_start_offset + day_in_week_offset);
				output.push_back(temp);
			}
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::ByHourExpand(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYHOUR and generating a new date-time for it and insert into output
		for(std::vector<int32_t>::const_iterator iter2 = mByHours.begin(); iter2 != mByHours.end(); iter2++)
		{
			CICalendarDateTime temp(*iter1);
			temp.SetHours(*iter2);
			output.push_back(temp);
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::ByMinuteExpand(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYMINUTE and generating a new date-time for it and insert into output
		for(std::vector<int32_t>::const_iterator iter2 = mByMinutes.begin(); iter2 != mByMinutes.end(); iter2++)
		{
			CICalendarDateTime temp(*iter1);
			temp.SetMinutes(*iter2);
			output.push_back(temp);
		}
	}
	
	dates = output;
}

void CICalendarRecurrence::BySecondExpand(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYSECOND and generating a new date-time for it and insert into output
		for(std::vector<int32_t>::const_iterator iter2 = mBySeconds.begin(); iter2 != mBySeconds.end(); iter2++)
		{
			CICalendarDateTime temp(*iter1);
			temp.SetSeconds(*iter2);
			output.push_back(temp);
		}
	}
	
	dates = output;
}

#pragma mark ____________________________BYxxx limits

void CICalendarRecurrence::ByMonthLimit(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYMONTH and indicate keep if input month matches
		bool keep = false;
		for(std::vector<int32_t>::const_iterator iter2 = mByMonth.begin(); !keep && (iter2 != mByMonth.end()); iter2++)
		{
			keep = ((*iter1).GetMonth() == *iter2);
		}
		if (keep)
			output.push_back(*iter1);
	}
	
	dates = output;
}

void CICalendarRecurrence::ByWeekNoLimit(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYWEEKNO and indicate keep if input month matches
		bool keep = false;
		for(std::vector<int32_t>::const_iterator iter2 = mByWeekNo.begin(); !keep && (iter2 != mByWeekNo.end()); iter2++)
		{
			keep = (*iter1).IsWeekNo(*iter2);
		}
		if (keep)
			output.push_back(*iter1);
	}
	
	dates = output;
}

void CICalendarRecurrence::ByMonthDayLimit(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYMONTHDAY and indicate keep if input month matches
		bool keep = false;
		for(std::vector<int32_t>::const_iterator iter2 = mByMonthDay.begin(); !keep && (iter2 != mByMonthDay.end()); iter2++)
		{
			keep = (*iter1).IsMonthDay(*iter2);
		}
		if (keep)
			output.push_back(*iter1);
	}
	
	dates = output;
}

void CICalendarRecurrence::ByDayLimit(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYDAY and indicate keep if input month matches
		bool keep = false;
		for(std::vector<CWeekDayNum>::const_iterator iter2 = mByDay.begin(); !keep && (iter2 != mByDay.end()); iter2++)
		{
			keep = (*iter1).IsDayOfWeekInMonth((*iter2).first, (*iter2).second);
		}
		if (keep)
			output.push_back(*iter1);
	}
	
	dates = output;
}

void CICalendarRecurrence::ByHourLimit(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYHOUR and indicate keep if input hour matches
		bool keep = false;
		for(std::vector<int32_t>::const_iterator iter2 = mByHours.begin(); !keep && (iter2 != mByHours.end()); iter2++)
		{
			keep = ((*iter1).GetHours() == *iter2);
		}
		if (keep)
			output.push_back(*iter1);
	}
	
	dates = output;
}

void CICalendarRecurrence::ByMinuteLimit(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYMINUTE and indicate keep if input minute matches
		bool keep = false;
		for(std::vector<int32_t>::const_iterator iter2 = mByMinutes.begin(); !keep && (iter2 != mByMinutes.end()); iter2++)
		{
			keep = ((*iter1).GetMinutes() == *iter2);
		}
		if (keep)
			output.push_back(*iter1);
	}
	
	dates = output;
}

void CICalendarRecurrence::BySecondLimit(CICalendarDateTimeList& dates) const
{
	// Loop over all input items
	CICalendarDateTimeList output;
	for(CICalendarDateTimeList::const_iterator iter1 = dates.begin(); iter1 != dates.end(); iter1++)
	{
		// Loop over each BYSECOND and indicate keep if input second matches
		bool keep = false;
		for(std::vector<int32_t>::const_iterator iter2 = mBySeconds.begin(); !keep && (iter2 != mBySeconds.end()); iter2++)
		{
			keep = ((*iter1).GetSeconds() == *iter2);
		}
		if (keep)
			output.push_back(*iter1);
	}
	
	dates = output;
}

void CICalendarRecurrence::BySetPosLimit(CICalendarDateTimeList& dates) const
{
	// The input dates MUST be sorted in order for this to work properly
	sort(dates.begin(), dates.end());

	// Loop over each BYSETPOS and extract the relevant component from the input array and add to the output
	CICalendarDateTimeList output;
	size_t input_size = dates.size();
	for(std::vector<int32_t>::const_iterator iter = mBySetPos.begin(); iter != mBySetPos.end(); iter++)
	{
		if (*iter > 0)
		{
			// Positive values are offset from the start
			if ((*iter) <= input_size)
				output.push_back(dates[*iter - 1]);
		}
		else if (*iter < 0)
		{
			// Negative values are offset from the end
			if (-(*iter) <= input_size)
				output.push_back(dates[input_size + *iter]);
		}
	}
	
	dates = output;
}
