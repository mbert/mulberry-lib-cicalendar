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
	CICalendarUTCOffsetValue.h

	Author:
	Description:	<describe the CICalendarUTCOffsetValue class here>
*/

#ifndef CICalendarUTCOffsetValue_H
#define CICalendarUTCOffsetValue_H

#include "CICalendarValue.h"

#include <stdint.h>


namespace iCal {

class CICalendarUTCOffsetValue : public CICalendarValue
{
public:
	CICalendarUTCOffsetValue()
	{ mValue = 0; }
	CICalendarUTCOffsetValue(const int32_t value)
		{ mValue = value; }
	CICalendarUTCOffsetValue(const CICalendarUTCOffsetValue& copy)
		 : CICalendarValue(copy)
		{ _copy_CICalendarUTCOffsetValue(copy); }
	virtual ~CICalendarUTCOffsetValue() {}

	CICalendarUTCOffsetValue& operator=(const CICalendarUTCOffsetValue& copy)
		{ if (this != &copy) _copy_CICalendarUTCOffsetValue(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarUTCOffsetValue(*this); }

	virtual EICalValueType GetType() const
		{ return eValueType_UTC_Offset; }

	virtual void Parse(const cdstring& data);
	virtual void Generate(std::ostream& os) const;

	int32_t GetValue() const
		{ return mValue; }
	void SetValue(const int32_t value)
		{ mValue = value; }

protected:
	int32_t mValue;

private:
	void _copy_CICalendarUTCOffsetValue(const CICalendarUTCOffsetValue& copy)
		{ mValue = copy.mValue; }
};

}	// namespace iCal

#endif	// CICalendarUTCOffsetValue_H
