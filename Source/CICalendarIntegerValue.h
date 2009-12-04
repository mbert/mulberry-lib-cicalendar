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
	CICalendarIntegerValue.h

	Author:
	Description:	<describe the CICalendarIntegerValue class here>
*/

#ifndef CICalendarIntegerValue_H
#define CICalendarIntegerValue_H

#include "CICalendarValue.h"

#include <stdint.h>


namespace iCal {

class CICalendarIntegerValue : public CICalendarValue
{
public:
	CICalendarIntegerValue()
		{ mValue = 0; }
	CICalendarIntegerValue(const int32_t value)
		{ mValue = value; }
	CICalendarIntegerValue(const CICalendarIntegerValue& copy)
		 : CICalendarValue(copy)
		{ _copy_CICalendarIntegerValue(copy); }
	virtual ~CICalendarIntegerValue() {}

	CICalendarIntegerValue& operator=(const CICalendarIntegerValue& copy)
		{ if (this != &copy) _copy_CICalendarIntegerValue(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarIntegerValue(*this); }

	virtual EICalValueType GetType() const
		{ return eValueType_Integer; }

	virtual void Parse(const cdstring& data);
	virtual void Generate(std::ostream& os) const;

	int32_t GetValue() const
		{ return mValue; }
	void SetValue(const int32_t value)
		{ mValue = value; }

protected:
	int32_t mValue;

private:
	void _copy_CICalendarIntegerValue(const CICalendarIntegerValue& copy)
		{ mValue = copy.mValue; }
};

}	// namespace iCal

#endif	// CICalendarIntegerValue_H
