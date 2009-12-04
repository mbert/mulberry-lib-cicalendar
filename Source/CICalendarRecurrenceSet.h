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
	CICalendarRecurrenceSet.h

	Author:
	Description:	<describe the CICalendarRecurrenceSet class here>
*/

#ifndef CICalendarRecurrenceSet_H
#define CICalendarRecurrenceSet_H

#include "CICalendarDateTime.h"
#include "CICalendarDefinitions.h"
#include "CICalendarPeriod.h"
#include "CICalendarRecurrence.h"

#include <map>


namespace iCal {

class CICalendarRecurrenceSet
{
public:
	CICalendarRecurrenceSet() {}
	CICalendarRecurrenceSet(const CICalendarRecurrenceSet& copy)
		{ _copy_CICalendarRecurrenceSet(copy); }
	virtual ~CICalendarRecurrenceSet() {}

	CICalendarRecurrenceSet& operator=(const CICalendarRecurrenceSet& copy)
		{ if (this != &copy) _copy_CICalendarRecurrenceSet(copy); return *this; }

	bool HasRecurrence() const;

	bool Equals(const CICalendarRecurrenceSet* comp) const;

	void Add(const CICalendarRecurrence& rule)
		{ mRrules.push_back(rule); }
	void Subtract(const CICalendarRecurrence& rule)
		{ mExrules.push_back(rule); }
	void Add(const CICalendarDateTime& dt)
		{ mRdates.push_back(dt); }
	void Subtract(const CICalendarDateTime& dt)
		{ mExdates.push_back(dt); }
	void Add(const CICalendarPeriod& p)
		{ mRperiods.push_back(p); }
	void Subtract(const CICalendarPeriod& p)
		{ mExperiods.push_back(p); }

	const CICalendarRecurrenceList& GetRules() const
		{ return mRrules; }
	const CICalendarRecurrenceList& GetExrules() const
		{ return mExrules; }
	const CICalendarDateTimeList& GetDates() const
		{ return mRdates; }
	const CICalendarDateTimeList& GetExdates() const
		{ return mExdates; }
	const CICalendarPeriodList& GetPeriods() const
		{ return mRperiods; }
	const CICalendarPeriodList& GetExperiods() const
		{ return mExperiods; }

	void Expand(const CICalendarDateTime& start, const CICalendarPeriod& range, CICalendarDateTimeList& items) const;
	void Changed();
	void ExcludeFutureRecurrence(const CICalendarDateTime& exclude);

	// UI operations
	bool IsSimpleUI() const;
	bool IsAdvancedUI() const;
	const CICalendarRecurrence* GetUIRecurrence() const
		{ return (mRrules.size() == 1) ? &mRrules.front() : NULL; }

	cdstring GetUIDescription() const;

protected:
	CICalendarRecurrenceList	mRrules;
	CICalendarRecurrenceList	mExrules;
	CICalendarDateTimeList		mRdates;
	CICalendarDateTimeList		mExdates;
	CICalendarPeriodList		mRperiods;
	CICalendarPeriodList		mExperiods;

	bool EqualsRules(const CICalendarRecurrenceList& rules1, const CICalendarRecurrenceList& rules2) const;
	bool EqualsDates(const CICalendarDateTimeList& dates1, const CICalendarDateTimeList& dates2) const;
	bool EqualsPeriods(const CICalendarPeriodList& periods1, const CICalendarPeriodList& periods2) const;

private:
	void _copy_CICalendarRecurrenceSet(const CICalendarRecurrenceSet& copy);
};

}	// namespace iCal

#endif	// CICalendarRecurrenceSet_H
