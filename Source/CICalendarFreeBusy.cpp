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
	CICalendarFreeBusy.cpp

	Author:
	Description:	<describe the CICalendarFreeBusy class here>
*/

#include "CICalendarFreeBusy.h"

#include <algorithm>

using namespace iCal;

// Resolve and merge any overlapping periods in the list
void CICalendarFreeBusy::ResolveOverlaps(CICalendarFreeBusyList& fb)
{
	// First sort it
	std::sort(fb.begin(), fb.end());

	// Now do merge
	for(CICalendarFreeBusyList::iterator iter = fb.begin(); iter != fb.end(); iter++)
	{
		CICalendarFreeBusyList::iterator next = iter + 1;
		while (next != fb.end())
		{
			// Check for overlap
			if ((*iter).GetPeriod().IsPeriodOverlap((*next).GetPeriod()))
			{
				// Determine if same type
				if ((*iter).GetType() != (*next).GetType())
				{
					// Need to partition current/next range according to overlap
					if ((*iter).GetType() > (*next).GetType())
					{
						// The current one wins over the next one
						if ((*next).GetPeriod().GetEnd() > (*iter).GetPeriod().GetEnd())
							// Truncate start of next one to end of current one
							(*next).SetPeriod(CICalendarPeriod((*iter).GetPeriod().GetEnd(), (*next).GetPeriod().GetEnd()));
						else
							// Delete the next one as it is completely contained in the current one
							fb.erase(next);
					}
					else
					{
						CICalendarFreeBusy::EBusyType iter_type = (*iter).GetType();
						CICalendarFreeBusy::EBusyType next_type = (*next).GetType();
						CICalendarPeriod iter_period((*iter).GetPeriod());
						CICalendarPeriod next_period((*iter).GetPeriod());
						
						// Truncate at start
						if (next_period.GetStart() > iter_period.GetStart())
						{
							// Truncate end of current one to start of next one
							(*iter).SetPeriod(CICalendarPeriod(iter_period.GetStart(), next_period.GetStart()));
							
							// Now see what happens at the end
							if (iter_period.GetEnd() > next_period.GetEnd())
							{
								// Need to insert new item (preserve current iterator value via its index)
								unsigned long current_pos = iter - fb.begin();
								fb.insert(next + 1, CICalendarFreeBusy(iter_type, CICalendarPeriod(next_period.GetEnd(), iter_period.GetEnd())));
								iter = fb.begin() + current_pos;
							}
						}
						else
						{
							// Make the current one the same as the next one and delete the next one
							*iter = *next;
							
							// Now see what happens at the end
							if (iter_period.GetEnd() > next_period.GetEnd())
							{
								// Convert next item into current item type and truncate end
								(*next).SetType(iter_type);
								(*next).SetPeriod(CICalendarPeriod(next_period.GetEnd(), iter_period.GetEnd()));
							}
							else
								// No additional overlap at end so remove remaining piece
								fb.erase(next);
						}
					}
				}
				else
				{
					// Just merge the two
					
					// Extend the end of the current one, if the next goes beyond it
					// NB The start of the current does not need to be changed as it is always
					// less or equal to the start of the next after the sort
					if ((*next).GetPeriod().GetEnd() > (*iter).GetPeriod().GetEnd())
						(*iter).SetPeriod(CICalendarPeriod((*iter).GetPeriod().GetStart(), (*next).GetPeriod().GetEnd()));
					fb.erase(next);
				}

				// Reset to next one which may have changed
				next = iter + 1;
			}
			else
				// Skip to next test if no overlap
				break;
		}
	}
}

/*
void CICalendarFreeBusy::Merge(const CICalendarFreeBusy& fb)
{
	// Only if same type
	if (GetType() != fb.GetType())
		return;
	
	// Add other ones periods to this one
	for(CICalendarPeriodList::const_iterator iter = fb.GetPeriods().begin(); iter != fb.GetPeriods().end(); iter++)
		mPeriods.push_back(*iter);
	
	// Sort list
	std::sort(mPeriods.begin(), mPeriods.end());
	
	// Now do merge
	for(CICalendarPeriodList::iterator iter = mPeriods.begin(); iter != mPeriods.end(); iter++)
	{
		while (iter + 1 != GetPeriods().end())
		{
			// Check for overlap
			if ((*iter).IsPeriodOverlap(*(iter + 1)))
			{
				if ((iter + 1)->GetEnd() > (*iter).GetEnd())
					(*iter) = CICalendarPeriod((*iter).GetStart(), (iter + 1)->GetEnd());
				mPeriods.erase(iter + 1);
			}
			else
				break;
		}
	}
}
*/
