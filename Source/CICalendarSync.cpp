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
	CICalendarSync.cpp

	Author:
	Description:	Synchronise two iCalendars
*/

#include "CICalendarSync.h"

#include <algorithm>
#include <iterator>

using namespace iCal;

void CICalendarSync::Sync()
{
	// Do this for each component list in turn:
	
	// 1. Get set of UIDs/RIDs for each component in cal1
	// 2. Get set of UIDs/RIDs for each component in cal2
	// 3. Determine set of keys in cal1 but not in cal2
	//  3.1 Remove from this set the ones that are recorded as new (those are new components added to cal1)
	//  3.2 Remove the components for the remaining keys from cal1 (they are components deleted on the server)
	//  3.3 Redo set of UIDs/RIDs for each component in cal1 if it changed in 3.2
	// 4. Determine set of keys in cal2 but not in cal1
	//  4.1 Remove from this set the ones recorded as deleted in cal1 (those are deleted components to be removed)
	//  4.2 Copy the components for the remaining keys from cal2 to cal1 (these are new components on the server)
	// 5. Determine overlapping keys in cal1 and cal2
	//  5.1 Compare SEQ for each overlapping pair
	//   5.1.1 If cal1 has not changed and cal2 is greater then replace cal1 item with cal2 item
	//   5.1.2 If cal1 has changed and original cal1 is same as cal2, leave cal1 item alone
	//   5.1.3 If cal1 has changed and original cal1 is not same as cal2, need to merge
	//    5.1.3.1 If cal1 and cal2 have last-modified properties use the most recent
	//    5.1.3.2 If cal1 has last-modified or neither have last-modified leave cal1 item alone
	//    5.1.3.3 If cal2 has last-modified then replace cal1 item with cal2 item
	// 6 Broadcast cal1 changes
	// Done!
	
	bool cal1_changed = false;

	// Step 1
	CICalendarSyncDataList keys1;
	GetAllKeys(mCal1, keys1);
	std::sort(keys1.begin(), keys1.end());

	// Step 2
	CICalendarSyncDataList keys2;
	GetAllKeys(mCal2, keys2);
	std::sort(keys2.begin(), keys2.end());
	
	// Step 3
	CICalendarSyncDataList keyset;
	std::set_difference(keys1.begin(), keys1.end(), keys2.begin(), keys2.end(), std::back_inserter<CICalendarSyncDataList>(keyset));
	
	// Step 3.1
	RemoveKeys(keyset, mCal1.GetRecording(), CICalendarComponentRecord::eAdded);
	
	// Step 3.2
	for(CICalendarSyncDataList::const_iterator iter = keyset.begin(); iter != keyset.end(); iter++)
	{
		mCal1.RemoveComponentByKey((*iter).GetMapKey());
		cal1_changed = true;
	}
	
	// Step 3.3
	if (keyset.size() != 0)
	{
		keys1.clear();
		GetAllKeys(mCal1, keys1);
		std::sort(keys1.begin(), keys1.end());
	}
	
	// Step 4
	keyset.clear();
	std::set_difference(keys2.begin(), keys2.end(), keys1.begin(), keys1.end(), std::back_inserter<CICalendarSyncDataList>(keyset));
	
	// Step 4.1
	RemoveKeys(keyset, mCal1.GetRecording(), CICalendarComponentRecord::eRemoved | CICalendarComponentRecord::eRemovedAdded);

	// Step 4.2
	for(CICalendarSyncDataList::const_iterator iter = keyset.begin(); iter != keyset.end(); iter++)
	{
		const CICalendarComponent* comp = mCal2.GetComponentByKey((*iter).GetMapKey());
		if (comp != NULL)
		{
			CICalendarComponent* new_comp = comp->clone();
			new_comp->SetCalendar(mCal1.GetRef());
			mCal1.AddComponent(new_comp);
			cal1_changed = true;
		}
	}
	
	// Step 5
	keyset.clear();
	{
		// This code is std::set_intersection except that we do out action when the items are equal

		CICalendarSyncDataList::const_iterator first1 = keys1.begin();
		CICalendarSyncDataList::const_iterator last1 = keys1.end();
		CICalendarSyncDataList::const_iterator first2 = keys2.begin();
		CICalendarSyncDataList::const_iterator last2 = keys2.end();

		while (first1 != last1 && first2 != last2)
		{
			if (*first2 < *first1)
				++first2;
			else if (*first1 < *first2)
				++first1;
			else
			{
				// Items match
				
				// Step 5.1
				if ((*first1).GetSeq() != (*first2).GetSeq())
				{
					// Step 5.1.1
					if ((*first1).GetSeq() == (*first1).GetOriginalSeq())
					{
						// Double check the one on the server is newer
						if ((*first1).GetSeq() < (*first2).GetSeq())
						{
							// Also make sure we can get the one from the server
							const CICalendarComponent* comp = mCal2.GetComponentByKey((*first2).GetMapKey());
							if (comp != NULL)
							{
								// Remove one in the cache
								mCal1.RemoveComponentByKey((*first1).GetMapKey());

								// Copy one from server
								CICalendarComponent* new_comp = comp->clone();
								new_comp->SetCalendar(mCal1.GetRef());
								mCal1.AddComponent(new_comp);
							}
						}
					}
					else
					{
						// Step 5.1.2
						if ((*first1).GetOriginalSeq() == (*first2).GetSeq())
						{
							// Nothing to do
						}
						
						// Step 5.1.3
						else
						{
							// Get last-modified for each component
							const CICalendarComponent* comp1 = mCal1.GetComponentByKey((*first1).GetMapKey());
							const CICalendarComponent* comp2 = mCal2.GetComponentByKey((*first2).GetMapKey());
							
							CICalendarDateTime dt1;
							bool has_last_modified1 = (comp1 != NULL) ? comp1->GetProperty(cICalProperty_LAST_MODIFIED, dt1) : false;
							
							CICalendarDateTime dt2;
							bool has_last_modified2 = (comp2 != NULL) ? comp2->GetProperty(cICalProperty_LAST_MODIFIED, dt2) : false;
							
							//    5.1.3.1
							if (has_last_modified1 && has_last_modified2)
							{
								if (dt2 > dt1)
								{
									// Copy one from server
									CICalendarComponent* new_comp = comp2->clone();
									new_comp->SetCalendar(mCal1.GetRef());
									mCal1.AddComponent(new_comp);
								}
							}
							//    5.1.3.2
							else if (has_last_modified1 || (!has_last_modified1 && !has_last_modified2))
							{
								// Leave alone
							}
							//    5.1.3.3
							else if (has_last_modified2)
							{
								// Copy one from server
								CICalendarComponent* new_comp = comp2->clone();
								new_comp->SetCalendar(mCal1.GetRef());
								mCal1.AddComponent(new_comp);
							}
						}
					}
				}

				++first1;
				++first2;
			}
		}
	}

	// Step 6
	//if (cal1_changed) ;
}

// NB Assumes components are the same but different versions
int CICalendarSync::CompareComponentVersions(const CICalendarComponent* comp1, const CICalendarComponent* comp2)
{
	// First check SEQ
	if (comp1->GetSeq() != comp2->GetSeq())
		return comp1->GetSeq() < comp2->GetSeq() ? -1 : 1;
	
	// Compare last modified
	CICalendarDateTime dt1;
	bool has_last_modified1 = (comp1 != NULL) ? comp1->GetProperty(cICalProperty_LAST_MODIFIED, dt1) : false;
	
	CICalendarDateTime dt2;
	bool has_last_modified2 = (comp2 != NULL) ? comp2->GetProperty(cICalProperty_LAST_MODIFIED, dt2) : false;
	
	if (has_last_modified1 && has_last_modified2)
	{
		if (dt1 == dt2)
			return 0;
		return dt1 < dt2 ? -1 : 1;
	}

	else if (has_last_modified1)
	{
		return 1;
	}

	else if (has_last_modified2)
	{
		return -1;
	}
	else
		return 0;
}

// Get keys of all VEVENTs, VTODOs and VJOURNALs
void CICalendarSync::GetAllKeys(const CICalendar& cal, CICalendarSyncDataList& keys)
{
	// Do for each DB
	GetKeys(cal.GetVEvents(), keys);
	GetKeys(cal.GetVToDos(), keys);
	GetKeys(cal.GetVJournals(), keys);
}

// Get keys of components
void CICalendarSync::GetKeys(const CICalendarComponentDB& db, CICalendarSyncDataList& keys)
{
	for(CICalendarComponentDB::const_iterator iter = db.begin(); iter != db.end(); iter++)
	{
		keys.push_back(CICalendarSyncData((*iter).second->GetMapKey(), (*iter).second->GetSeq(), (*iter).second->GetOriginalSeq()));
	}
}

void CICalendarSync::RemoveKeys(CICalendarSyncDataList& keys, const CICalendarComponentRecordDB& recorded, unsigned long filter)
{
	// Find recorded keys matching filter
	CICalendarSyncDataList recorded_keys;
	for(CICalendarComponentRecordDB::const_iterator iter = recorded.begin(); iter != recorded.end(); iter++)
	{
		if (((*iter).second.GetAction() & filter) != 0)
			recorded_keys.push_back(CICalendarSyncData((*iter).first, 0, 0));
	}
	
	// Now remove recorded keys from key set
	if (recorded_keys.size() != 0)
	{
		CICalendarSyncDataList result;
		std::set_difference(keys.begin(), keys.end(), recorded_keys.begin(), recorded_keys.end(), std::back_inserter<CICalendarSyncDataList>(result));
		if (result.size() != keys.size())
			keys = result;
	}
}
