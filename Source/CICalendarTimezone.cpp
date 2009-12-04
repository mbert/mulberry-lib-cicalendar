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
	CICalendarTimezone.cpp

	Author:
	Description:	<describe the CICalendarTimezone class here>
*/

#include "CICalendarTimezone.h"

#include "CICalendar.h"
#include "CICalendarManager.h"
#include "CICalendarUtils.h"

using namespace iCal;

CICalendarTimezone::CICalendarTimezone()
{
	// Copy defauilt timezone if it exists
	if (CICalendarManager::sICalendarManager != NULL)
	{
		_copy_CICalendarTimezone(CICalendarManager::sICalendarManager->GetDefaultTimezone());
		
	}
}

int CICalendarTimezone::operator==(const CICalendarTimezone& comp) const
{
	// Always match if any one of them is 'floating'
	if (!mUTC && mTimezone.empty() || !comp.mUTC && comp.mTimezone.empty())
		return 1;
	else if (mUTC ^ comp.mUTC)
		return 0;
	else
		return (mUTC || (mTimezone == comp.mTimezone)) ? 1 : 0;
}

int32_t CICalendarTimezone::TimeZoneSecondsOffset(const CICalendarDateTime& dt) const
{
	if (mUTC)
		return 0;
	else if (mTimezone.empty())
		return CICalendar::sICalendar.GetTimezoneOffsetSeconds(CICalendarManager::sICalendarManager->GetDefaultTimezone().GetTimezoneID(), dt);

	// Look up timezone and resolve date using default timezones
	return CICalendar::sICalendar.GetTimezoneOffsetSeconds(mTimezone, dt);
}

cdstring CICalendarTimezone::TimeZoneDescriptor(const CICalendarDateTime& dt) const
{
	if (mUTC)
		return "(UTC)";
	else if (mTimezone.empty())
		return CICalendar::sICalendar.GetTimezoneDescriptor(CICalendarManager::sICalendarManager->GetDefaultTimezone().GetTimezoneID(), dt);

	// Look up timezone and resolve date using default timezones
	return CICalendar::sICalendar.GetTimezoneDescriptor(mTimezone, dt);
}
