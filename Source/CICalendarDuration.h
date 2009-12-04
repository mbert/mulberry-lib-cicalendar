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
	CICalendarDuration.h

	Author:
	Description:	<describe the CICalendarDuration class here>
*/

#ifndef CICalendarDuration_H
#define CICalendarDuration_H

#include <stdint.h>
#include <ostream>

#include "cdstring.h"


namespace iCal {

class CICalendarDuration
{
public:
	CICalendarDuration();
	CICalendarDuration(int64_t seconds)
		{ SetDuration(seconds); }
	CICalendarDuration(const CICalendarDuration& copy)
		{ _copy_CICalendarDuration(copy); }
	virtual ~CICalendarDuration() {}

	CICalendarDuration& operator=(const CICalendarDuration& copy)
		{ if (this != &copy) _copy_CICalendarDuration(copy); return *this; }

	int64_t		GetTotalSeconds() const;
	void		SetDuration(const int64_t seconds);

	bool		GetForward() const
		{ return mForward; }

	uint32_t	GetWeeks() const
		{ return mWeeks; }
	uint32_t	GetDays() const
		{ return mDays; }

	uint32_t	GetHours() const
		{ return mHours; }
	uint32_t	GetMinutes() const
		{ return mMinutes; }
	uint32_t	GetSeconds() const
		{ return mSeconds; }

	void Parse(const cdstring& data);
	void Generate(std::ostream& os) const;

protected:
	bool		mForward;

	uint32_t	mWeeks;
	uint32_t	mDays;

	uint32_t	mHours;
	uint32_t	mMinutes;
	uint32_t	mSeconds;

private:
	void _copy_CICalendarDuration(const CICalendarDuration& copy);
};

}	// namespace iCal

#endif	// CICalendarDuration_H
