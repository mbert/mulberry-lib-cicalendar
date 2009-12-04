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
	CICalendarDurationValue.h

	Author:
	Description:	<describe the CICalendarDurationValue class here>
*/

#ifndef CICalendarDurationValue_H
#define CICalendarDurationValue_H

#include "CICalendarValue.h"

#include "CICalendarDuration.h"

namespace iCal {

class CICalendarDurationValue: public CICalendarValue
{
public:
	CICalendarDurationValue() {}
	CICalendarDurationValue(const CICalendarDuration& value)
		{ mValue = value; }
	CICalendarDurationValue(const CICalendarDurationValue& copy)
		 : CICalendarValue(copy)
		{ _copy_CICalendarDurationValue(copy); }
	virtual ~CICalendarDurationValue() {}

	CICalendarDurationValue& operator=(const CICalendarDurationValue& copy)
		{ if (this != &copy) _copy_CICalendarDurationValue(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarDurationValue(*this); }

	virtual EICalValueType GetType() const
		{ return eValueType_Duration; }

	virtual void Parse(const cdstring& data)
		{ mValue.Parse(data); }
	virtual void Generate(std::ostream& os) const
		{ mValue.Generate(os); }

	CICalendarDuration& GetValue()
		{ return mValue; }
	const CICalendarDuration& GetValue() const
		{ return mValue; }
	void SetValue(const CICalendarDuration& value)
		{ mValue = value; }

protected:
	CICalendarDuration mValue;

private:
	void _copy_CICalendarDurationValue(const CICalendarDurationValue& copy)
		{ mValue = copy.mValue; }
};

}	// namespace iCal

#endif	// CICalendarDurationValue_H
