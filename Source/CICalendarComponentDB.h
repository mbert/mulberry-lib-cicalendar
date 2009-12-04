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
	CICalendarComponentDB.h

	Author:
	Description:	map class to hold UID->component mapping as well as UID->Recurrence instance mapping
*/

#ifndef CICalendarComponentDB_H
#define CICalendarComponentDB_H

#include <map>
#include <vector>

#include "cdstring.h"

#include "CICalendarDateTime.h"


namespace iCal {

class CICalendarComponent;
typedef std::map<cdstring, CICalendarComponent*> CICalendarComponentMap;

class CICalendarComponentRecur;
typedef std::vector<CICalendarComponentRecur*> CICalendarComponentRecurs;

typedef uint32_t	CICalendarRef;	// Unique reference to object
class CICalendar;

class CICalendarComponentDB : public CICalendarComponentMap
{
public:
	CICalendarComponentDB()
		{ }
	CICalendarComponentDB(const CICalendarComponentDB& copy) :
		CICalendarComponentMap(copy)
		{ _copy_CICalendarComponentDB(copy); }
	virtual ~CICalendarComponentDB() {}

	CICalendarComponentDB& operator=(const CICalendarComponentDB& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarComponentDB(copy);
				CICalendarComponentMap::operator=(copy);
			}
			return *this;
		}

	bool AddComponent(CICalendarComponent* comp);
	void RemoveComponent(CICalendarComponent* comp, bool delete_it);
	void RemoveAllComponents();
	void ChangedComponent(CICalendarComponent* comp);

	void GetRecurrenceInstances(const cdstring& uid, CICalendarDateTimeList& ids) const;
	void GetRecurrenceInstances(const cdstring& uid, CICalendarComponentRecurs& items) const;

protected:
	typedef std::map<cdstring, CICalendarDateTimeList> CICalendarRecurrenceMap;

	CICalendarRecurrenceMap	mRecurMap;

	CICalendarComponentRecur* GetRecurrenceInstance(const cdstring& uid, const CICalendarDateTime& rid) const;

private:
	void	_copy_CICalendarComponentDB(const CICalendarComponentDB& copy)
		{ mRecurMap = copy.mRecurMap; }
};

typedef std::vector<const CICalendarComponentDB*> CICalendarComponentDBList;

}	// namespace iCal

#endif	// CICalendarComponentDB_H
