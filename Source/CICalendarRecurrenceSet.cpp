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
	CICalendarRecurrenceSet.cpp

	Author:
	Description:	<describe the CICalendarRecurrenceSet class here>
*/

#include "CICalendarRecurrenceSet.h"

#include "CStringUtils.h"

#include <cerrno>
#include <algorithm>

using namespace iCal;

void CICalendarRecurrenceSet::_copy_CICalendarRecurrenceSet(const CICalendarRecurrenceSet& copy)
{
	mRrules = copy.mRrules;
	mExrules = copy.mExrules;
	mRdates = copy.mRdates;
	mExdates = copy.mExdates;
	mRperiods = copy.mRperiods;
	mExperiods = copy.mExperiods;
}

bool CICalendarRecurrenceSet::HasRecurrence() const
{
	return !mRrules.empty() || !mRdates.empty() || !mRperiods.empty() || 
			!mExrules.empty() || !mExdates.empty() || !mExperiods.empty();
}


bool CICalendarRecurrenceSet::Equals(const CICalendarRecurrenceSet* comp) const
{
	// Look at RRULEs
	if (!EqualsRules(mRrules, comp->mRrules))
		return false;
	
	// Look at EXRULEs
	if (!EqualsRules(mExrules, comp->mExrules))
		return false;
	
	// Look at RDATEs
	if (!EqualsDates(mRdates, comp->mRdates))
		return false;
	if (!EqualsPeriods(mRperiods, comp->mRperiods))
		return false;
	
	// Look at EXDATEs
	if (!EqualsDates(mExdates, comp->mExdates))
		return false;
	if (!EqualsPeriods(mExperiods, comp->mExperiods))
		return false;
	
	// If we get here they match
	return true;
}

bool CICalendarRecurrenceSet::EqualsRules(const CICalendarRecurrenceList& rules1, const CICalendarRecurrenceList& rules2) const
{
	// Check sizes first
	if (rules1.size() != rules2.size())
		return false;
	else if (rules1.size() == 0)
		return true;
	
	// Do sledge hammer O(n^2) approach as its not easy to sort these things for a smarter test.
	// In most cases there will only be one rule anyway, so this should not be too painful.
	CICalendarRecurrenceList temp2(rules2);

	for(CICalendarRecurrenceList::const_iterator iter1 = rules1.begin(); iter1 != rules1.end(); iter1++)
	{
		bool found = false;
		for(CICalendarRecurrenceList::iterator iter2 = temp2.begin(); iter2 != temp2.end(); iter2++)
		{
			if ((*iter1).Equals(*iter2))
			{
				// Remove the one found so it is not tested again
				temp2.erase(iter2);
				found = true;
				break;
			}
		}
		
		if (!found)
			return false;
	}
	
	return true;
}

bool CICalendarRecurrenceSet::EqualsDates(const CICalendarDateTimeList& dates1, const CICalendarDateTimeList& dates2) const
{
	// Check sizes first
	if (dates1.size() != dates2.size())
		return false;
	else if (dates1.size() == 0)
		return true;
	
	// Copy each and sort for comparison
	CICalendarDateTimeList dt1(dates1);
	CICalendarDateTimeList dt2(dates2);
	
	std::sort(dt1.begin(), dt1.end());
	std::sort(dt2.begin(), dt2.end());
	
	return std::equal(dt1.begin(), dt1.end(), dt2.begin());
}

bool CICalendarRecurrenceSet::EqualsPeriods(const CICalendarPeriodList& periods1, const CICalendarPeriodList& periods2) const
{
	// Check sizes first
	if (periods1.size() != periods2.size())
		return false;
	else if (periods1.size() == 0)
		return true;
	
	// Copy each and sort for comparison
	CICalendarPeriodList p1(periods1);
	CICalendarPeriodList p2(periods2);
	
	std::sort(p1.begin(), p1.end());
	std::sort(p2.begin(), p2.end());
	
	return std::equal(p1.begin(), p1.end(), p2.begin());
}

void CICalendarRecurrenceSet::Expand(const CICalendarDateTime& start, const CICalendarPeriod& range, CICalendarDateTimeList& items) const
{
	// Now create list of items to include
	CICalendarDateTimeList include;
	
	// Always include the initial DTSTART if within the range
	if (range.IsDateWithinPeriod(start))
		include.push_back(start);

	// RRULES
	for(CICalendarRecurrenceList::const_iterator iter = mRrules.begin(); iter != mRrules.end(); iter++)
	{
		(*iter).Expand(start, range, include);
	}

	// RDATES
	for(CICalendarDateTimeList::const_iterator iter = mRdates.begin(); iter != mRdates.end(); iter++)
	{
		if (range.IsDateWithinPeriod(*iter))
			include.push_back(*iter);
	}
	for(CICalendarPeriodList::const_iterator iter = mRperiods.begin(); iter != mRperiods.end(); iter++)
	{
		if (range.IsPeriodOverlap(*iter))
			include.push_back((*iter).GetStart());
	}
	
	// Make sure the list is unique
	sort(include.begin(), include.end());
	include.erase(unique(include.begin(), include.end()), include.end());
	
	// Now create list of items to exclude
	CICalendarDateTimeList exclude;
	
	// EXRULES
	for(CICalendarRecurrenceList::const_iterator iter = mExrules.begin(); iter != mExrules.end(); iter++)
	{
		(*iter).Expand(start, range, exclude);
	}

	// EXDATES
	for(CICalendarDateTimeList::const_iterator iter = mExdates.begin(); iter != mExdates.end(); iter++)
	{
		if (range.IsDateWithinPeriod(*iter))
			exclude.push_back(*iter);
	}
	for(CICalendarPeriodList::const_iterator iter = mExperiods.begin(); iter != mExperiods.end(); iter++)
	{
		if (range.IsPeriodOverlap(*iter))
			exclude.push_back((*iter).GetStart());
	}
	
	// Make sure the list is unique
	sort(exclude.begin(), exclude.end());
	exclude.erase(unique(exclude.begin(), exclude.end()), exclude.end());
	
	// Add difference between to the two sets (include - exclude) to the results
	set_difference(include.begin(), include.end(), exclude.begin(), exclude.end(), back_inserter(items));

}

// Recurrence set changed in some way - force reset of all cached values
void CICalendarRecurrenceSet::Changed()
{
	// RRULES
	for(CICalendarRecurrenceList::iterator iter = mRrules.begin(); iter != mRrules.end(); iter++)
	{
		(*iter).Clear();
	}

	// EXRULES
	for(CICalendarRecurrenceList::iterator iter = mExrules.begin(); iter != mExrules.end(); iter++)
	{
		(*iter).Clear();
	}
}

// Exclude dates on or after the chosen one
void CICalendarRecurrenceSet::ExcludeFutureRecurrence(const CICalendarDateTime& exclude)
{
	// Adjust RRULES to end before start
	for(CICalendarRecurrenceList::iterator iter = mRrules.begin(); iter != mRrules.end(); iter++)
	{
		(*iter).ExcludeFutureRecurrence(exclude);
	}

	// Remove RDATES on or after start
	mRdates.erase(std::remove_if(mRdates.begin(), mRdates.end(), std::bind2nd(std::greater_equal<CICalendarDateTime>(), exclude)), mRdates.end());
	for(CICalendarPeriodList::iterator iter = mRperiods.begin(); iter != mRperiods.end(); iter++)
	{
		if (*iter > exclude)
		{
			mRperiods.erase(iter);
			iter--;
		}
	}
}

// Is recurrence set displayable in simple UI
bool CICalendarRecurrenceSet::IsSimpleUI() const
{
	// Right now the Event dialog only handles a single RRULE (but we allow any number of EXDATES as deleted
	// instances will appear as EXDATES)
	if ((mRrules.size() > 1) || (mExrules.size() > 0) || (mRdates.size() > 0) || (mRperiods.size() > 0))
		return false;
	
	// Also, check the rule iteself
	else if (mRrules.size() == 1)
		return mRrules.front().IsSimpleRule();
	else
		return true;
}

// Is recurrence set displayable in advanced UI
bool CICalendarRecurrenceSet::IsAdvancedUI() const
{
	// Right now the Event dialog only handles a single RRULE
	if ((mRrules.size() > 1) || (mExrules.size() > 0) || (mRdates.size() > 0) || (mRperiods.size() > 0))
		return false;
	
	// Also, check the rule iteself
	else if (mRrules.size() == 1)
		return mRrules.front().IsAdvancedRule();
	else
		return true;
}

cdstring CICalendarRecurrenceSet::GetUIDescription() const
{
	// Check for anything
	if (!HasRecurrence())
		return "No Recurrence";

	// Look for a single RRULE and return its descriptor
	if ((mRrules.size() == 1) && mExrules.empty() && mRdates.empty() && mRperiods.empty() && mExdates.empty() && mExperiods.empty())
	{
		return mRrules.front().GetUIDescription();
	}
	
	// Indicate some form of complex recurrence
	return "Multiple recurrence rules, dates or exclusions";
}
