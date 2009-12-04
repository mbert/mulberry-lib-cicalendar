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
	CICalendarRecurrence.h

	Author:
	Description:	<describe the CICalendarRecurrence class here>
*/

#ifndef CICalendarRecurrence_H
#define CICalendarRecurrence_H

#include "CICalendarDateTime.h"
#include "CICalendarDefinitions.h"

#include <map>
#include <vector>


namespace iCal {

class CICalendarPeriod;

class CICalendarRecurrence
{
public:
	typedef std::pair<int32_t, CICalendarDateTime::EDayOfWeek>	CWeekDayNum;

	CICalendarRecurrence()
		{ _init_CICalendarRecurrence(); }
	CICalendarRecurrence(const CICalendarRecurrence& copy)
		{ _copy_CICalendarRecurrence(copy); }
	virtual ~CICalendarRecurrence() {}

	CICalendarRecurrence& operator=(const CICalendarRecurrence& copy)
		{ if (this != &copy) _copy_CICalendarRecurrence(copy); return *this; }

	bool Equals(const CICalendarRecurrence& comp) const;

	ERecurrence_FREQ GetFreq() const
		{ return mFreq; }
	void SetFreq(ERecurrence_FREQ freq)
		{ mFreq = freq; }

	bool GetUseUntil() const
		{ return mUseUntil; }
	void SetUseUntil(bool use_until)
		{ mUseUntil = use_until; }

	const CICalendarDateTime& GetUntil() const
		{ return mUntil; }
	void SetUntil(const CICalendarDateTime& until)
		{ mUntil = until; }

	bool GetUseCount() const
		{ return mUseCount; }
	void SetUseCount(bool use_count)
		{ mUseCount = use_count; }

	int32_t GetCount() const
		{ return mCount; }
	void SetCount(int32_t count)
		{ mCount = count; }

	int32_t GetInterval() const
		{ return mInterval; }
	void SetInterval(int32_t interval)
		{ mInterval = interval; }

	const std::vector<int32_t>& GetByMonth() const
		{ return mByMonth; }
	void SetByMonth(const std::vector<int32_t>& by)
		{ mByMonth = by; }
		
	const std::vector<int32_t>& GetByMonthDay() const
		{ return mByMonthDay; }
	void SetByMonthDay(const std::vector<int32_t>& by)
		{ mByMonthDay = by; }
		
	const std::vector<CWeekDayNum>& GetByDay() const
		{ return mByDay; }
	void SetByDay(const std::vector<CWeekDayNum>& by)
		{ mByDay = by; }
		
	const std::vector<int32_t>& GetBySetPos() const
		{ return mBySetPos; }
	void SetBySetPos(const std::vector<int32_t>& by)
		{ mBySetPos = by; }
		
	void Parse(const cdstring& data);
	void Generate(std::ostream& os) const;

	bool HasBy() const
	{
		return !mBySeconds.empty() || !mByMinutes.empty() || !mByHours.empty() ||
				!mByDay.empty() || !mByMonthDay.empty() || !mByYearDay.empty() ||
				!mByWeekNo.empty() || !mByMonth.empty() || !mBySetPos.empty();
	}

	bool IsSimpleRule() const;
	bool IsAdvancedRule() const;

	cdstring GetUIDescription() const;

	void Expand(const CICalendarDateTime& start, const CICalendarPeriod& range, CICalendarDateTimeList& items) const;
	void Clear();
	void ExcludeFutureRecurrence(const CICalendarDateTime& exclude);

protected:
	ERecurrence_FREQ	mFreq;

	bool				mUseUntil;
	CICalendarDateTime	mUntil;
	bool				mUseCount;
	int32_t				mCount;

	int32_t						mInterval;
	std::vector<int32_t>				mBySeconds;
	std::vector<int32_t>				mByMinutes;
	std::vector<int32_t>				mByHours;
	std::vector<CWeekDayNum>			mByDay;
	std::vector<int32_t>				mByMonthDay;
	std::vector<int32_t>				mByYearDay;
	std::vector<int32_t>				mByWeekNo;
	std::vector<int32_t>				mByMonth;
	std::vector<int32_t>				mBySetPos;
	ERecurrence_WEEKDAY			mWeekstart;

	mutable bool						mCached;
	mutable CICalendarDateTime			mCacheStart;
	mutable CICalendarDateTime			mCacheUpto;
	mutable bool						mFullyCached;
	mutable CICalendarDateTimeList		mRecurrences;

private:
	typedef std::map<cdstring, ERecurrence_FREQ>	CFreqMap;
	static CFreqMap		sFreqMap;

	void _init_CICalendarRecurrence();
	void _copy_CICalendarRecurrence(const CICalendarRecurrence& copy);

	bool Equals(const std::vector<int32_t>& items1, const std::vector<int32_t>& items2) const;
	bool Equals(const std::vector<CWeekDayNum>& items1, const std::vector<CWeekDayNum>& items2) const;

	void ParseList(const char* txt, std::vector<int32_t>& list);
	void ParseList(const char* txt, std::vector<CWeekDayNum>& list);

	bool SimpleExpand(const CICalendarDateTime& start, const CICalendarPeriod& range, CICalendarDateTimeList& items) const;
	bool ComplexExpand(const CICalendarDateTime& start, const CICalendarPeriod& range, CICalendarDateTimeList& items) const;

	void GenerateYearlySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const;
	void GenerateMonthlySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const;
	void GenerateWeeklySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const;
	void GenerateDailySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const;
	void GenerateHourlySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const;
	void GenerateMinutelySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const;
	void GenerateSecondlySet(const CICalendarDateTime& start, CICalendarDateTimeList& items) const;

	void ByMonthExpand(CICalendarDateTimeList& dates) const;
	void ByWeekNoExpand(CICalendarDateTimeList& dates) const;
	void ByYearDayExpand(CICalendarDateTimeList& dates) const;
	void ByMonthDayExpand(CICalendarDateTimeList& dates) const;
	void ByDayExpandYearly(CICalendarDateTimeList& dates) const;
	void ByDayExpandMonthly(CICalendarDateTimeList& dates) const;
	void ByDayExpandWeekly(CICalendarDateTimeList& dates) const;
	void ByHourExpand(CICalendarDateTimeList& dates) const;
	void ByMinuteExpand(CICalendarDateTimeList& dates) const;
	void BySecondExpand(CICalendarDateTimeList& dates) const;

	void ByMonthLimit(CICalendarDateTimeList& dates) const;
	void ByWeekNoLimit(CICalendarDateTimeList& dates) const;
	void ByMonthDayLimit(CICalendarDateTimeList& dates) const;
	void ByDayLimit(CICalendarDateTimeList& dates) const;
	void ByHourLimit(CICalendarDateTimeList& dates) const;
	void ByMinuteLimit(CICalendarDateTimeList& dates) const;
	void BySecondLimit(CICalendarDateTimeList& dates) const;
	void BySetPosLimit(CICalendarDateTimeList& dates) const;
};

typedef std::vector<CICalendarRecurrence>	CICalendarRecurrenceList;

}	// namespace iCal

#endif	// CICalendarRecurrence_H
