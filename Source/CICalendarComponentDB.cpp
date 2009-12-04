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
	CICalendarComponentDB.cpp

	Author:
	Description:	map class to hold UID->component mapping as well as UID->Recurrence instance mapping
*/

#include "CICalendarComponentDB.h"

#include "CICalendarComponentRecur.h"

using namespace iCal;

bool CICalendarComponentDB::AddComponent(CICalendarComponent* comp)
{
	// Must have valid UID
	if (comp->GetMapKey().length() == 0)
		return false;

	// Add the component to the calendar
	bool bresult = true;
	std::pair<CICalendarComponentMap::iterator, bool> result = insert(CICalendarComponentMap::value_type(comp->GetMapKey(), comp));

	// See if duplicate
	if (!result.second)
	{
		// Replace existing if sequence is higher
		if (comp->GetSeq() > (*result.first).second->GetSeq())
		{
			(*result.first).second = comp;
			bresult = true;
		}
		else
			bresult = false;
	}
	else
		bresult = true;
	
	// Now look for a recurrence component if it was added
	CICalendarComponentRecur* recur = dynamic_cast<CICalendarComponentRecur*>(comp);
	if (bresult && (recur != NULL))
	{
		// Add each overridden instance to the override map
		if (recur->IsRecurrenceInstance())
		{
			// Look for existiung UID->RID map entry
			CICalendarRecurrenceMap::iterator found = mRecurMap.find(recur->GetUID());
			if (found != mRecurMap.end())
			{
				// Add to existing list
				(*found).second.push_back(recur->GetRecurrenceID());
			}
			else
			{
				// Create new entry for this UID
				CICalendarDateTimeList temp;
				temp.push_back(recur->GetRecurrenceID());
				mRecurMap.insert(CICalendarRecurrenceMap::value_type(recur->GetUID(), temp));
			}
			
			// Now try and find the master component if it currently exists
			iterator found2 = find(recur->GetUID());
			if (found2 != end())
			{
				// Tell the instance who its master is
				recur->SetMaster(static_cast<CICalendarComponentRecur*>((*found2).second));
			}
		}
		
		// Make sure the master is sync'd with any instances that may have been added before it, those added after
		// will be sync'd when they are added
		else
		{
			// See if master has an entry in the UID->RID map
			CICalendarRecurrenceMap::iterator found = mRecurMap.find(comp->GetUID());
			if (found != mRecurMap.end())
			{
				// Make sure each instance knows about its master
				const CICalendarDateTimeList& recurs = (*found).second;
				for(CICalendarDateTimeList::const_iterator iter = recurs.begin(); iter != recurs.end(); iter++)
				{
					// Get the instance
					CICalendarComponentRecur* instance = GetRecurrenceInstance(comp->GetUID(), *iter);
					if (instance != NULL)
					{
						// Tell the instance who its master is
						static_cast<CICalendarComponentRecur*>(instance)->SetMaster(recur);
					}
				}
			}
		}
	}
	
	// Tell component it has now been added
	if (bresult)
		comp->Added();

	return bresult;
}

void CICalendarComponentDB::RemoveComponent(CICalendarComponent* comp, bool delete_it)
{
	// Tell component it is removed
	comp->Removed();

	// Only if present
	erase(comp->GetMapKey());
	
	// Delete if required
	if (delete_it)
		delete comp;
}

void CICalendarComponentDB::RemoveAllComponents()
{
	for(CICalendarComponentDB::const_iterator iter = begin(); iter != end(); iter++)
	{
		// Tell component it is removed and delete it
		(*iter).second->Removed();
		delete (*iter).second;
	}
	
	clear();
}

void CICalendarComponentDB::ChangedComponent(CICalendarComponent* comp)
{
	// Tell component it is changed
	comp->Changed();
}

void CICalendarComponentDB::GetRecurrenceInstances(const cdstring& uid, CICalendarDateTimeList& ids) const
{
	// Look for matching UID in recurrence instance map
	CICalendarRecurrenceMap::const_iterator found = mRecurMap.find(uid);
	if (found != mRecurMap.end())
	{
		// Return the recurrence ids
		ids = (*found).second;
	}
}

void CICalendarComponentDB::GetRecurrenceInstances(const cdstring& uid, CICalendarComponentRecurs& items) const
{
	// Look for matching UID in recurrence instance map
	CICalendarRecurrenceMap::const_iterator found = mRecurMap.find(uid);
	if (found != mRecurMap.end())
	{
		// Return all the recurrence ids
		for(CICalendarDateTimeList::const_iterator iter = (*found).second.begin(); iter != (*found).second.end(); iter++)
		{
			// Look it up
			CICalendarComponentRecur* recur = GetRecurrenceInstance(uid, *iter);
			if (recur != NULL)
				items.push_back(recur);
		}
	}
}

CICalendarComponentRecur* CICalendarComponentDB::GetRecurrenceInstance(const cdstring& uid, const CICalendarDateTime& rid) const
{
	const_iterator found = find(CICalendarComponentRecur::MapKey(uid, rid.GetText()));
	if (found != end())
	{
		// Tell the instance who its master is
		return static_cast<CICalendarComponentRecur*>((*found).second);
	}
	
	return NULL;
}
