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
	CICalendarTimezone.h

	Author:
	Description:	<describe the CICalendarTimezone class here>
*/

#ifndef CICalendarTimezone_H
#define CICalendarTimezone_H

#include "cdstring.h"

namespace iCal {

class CICalendarDateTime;

class CICalendarTimezone
{
public:
	CICalendarTimezone();
	CICalendarTimezone(bool utc)
		{ mUTC = utc; }
	CICalendarTimezone(bool utc, const cdstring& tzid)
		{ mUTC = utc; mTimezone = tzid; }
	CICalendarTimezone(const CICalendarTimezone& copy)
		{ _copy_CICalendarTimezone(copy); }
	virtual ~CICalendarTimezone() {}

	CICalendarTimezone& operator=(const CICalendarTimezone& copy)
		{ if (this != &copy) _copy_CICalendarTimezone(copy); return *this; }

	int operator==(const CICalendarTimezone& comp) const;

	bool GetUTC() const
		{ return mUTC; }
	void SetUTC(bool utc)
		{ mUTC = utc; }

	const cdstring& GetTimezoneID() const
		{ return mTimezone; }
	void SetTimezoneID(const cdstring& tzid)
		{ mTimezone = tzid; }

	bool Floating() const
	{
		return !mUTC && mTimezone.empty();
	}

	bool HasTZID() const
	{
		return !mUTC && !mTimezone.empty();
	}

	int32_t		TimeZoneSecondsOffset(const CICalendarDateTime& dt) const;
	cdstring	TimeZoneDescriptor(const CICalendarDateTime& dt) const;

protected:
	bool			mUTC;
	cdstring		mTimezone;

private:
	void _copy_CICalendarTimezone(const CICalendarTimezone& copy)
		{ mUTC = copy.mUTC; mTimezone = copy.mTimezone; }

};

}	// namespace iCal

#endif	// CICalendarTimezone_H
