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
	CICalendarUtils.h

	Author:
	Description:	<describe the CICalendarUtils class here>
*/

#ifndef CICalendarUtils_H
#define CICalendarUtils_H

#include "CICalendarDateTime.h"

#include <iostream>
#include <vector>

#include "cdstring.h"


namespace iCal {

class CICalendarUtils
{
public:
	typedef std::vector<std::vector<int32_t> > CICalendarTable;

	static bool	ReadFoldedLine(std::istream& is, cdstring& line1, cdstring& line2);

	static void WriteTextValue(std::ostream& os, const cdstring& value);
	static cdstring DecodeTextValue(const cdstring& value);

	// Date/time calcs
	static int32_t	DaysInMonth(const int32_t month, const int32_t year);
	static int32_t	DaysUptoMonth(const int32_t month, const int32_t year);
	static int32_t	IsLeapYear(const int32_t year);
	static int32_t	LeapDaysSince1970(const int32_t year_offset);

	static int32_t	GetLocalTimezoneOffsetSeconds();

	// Packed date
	static int32_t PackDate(const int32_t year, const int32_t month, const int32_t day)
		{ return (year << 16) | (month << 8) | (day + 128); }

	static void UnpackDate(const int32_t data, int32_t& year, int32_t& month, int32_t& day)
		{ year = (data & 0xFFFF0000) >> 16; month = (data & 0x0000FF00) >> 8; day = (data & 0xFF) - 128; }
	static int32_t UnpackDateYear(const int32_t data)
		{ return (data & 0xFFFF0000) >> 16; }
	static int32_t UnpackDateMonth(const int32_t data)
		{ return (data & 0x0000FF00) >> 8; }
	static int32_t UnpackDateDay(const int32_t data)
		{ return (data & 0xFF) - 128; }

	// Display elements
	static void		GetMonthTable(const int32_t month, const int32_t year, const CICalendarDateTime::EDayOfWeek weekstart, CICalendarTable& table, std::pair<int32_t, int32_t>& today_index);

private:
	CICalendarUtils() {}
	~CICalendarUtils() {}

};

}	// namespace iCal

#endif	// CICalendarUtils_H
