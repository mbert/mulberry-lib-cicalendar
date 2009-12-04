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
	CICalendarPeriodValue.h

	Author:
	Description:	<describe the CICalendarPeriodValue class here>
*/

#ifndef CICalendarPeriodValue_H
#define CICalendarPeriodValue_H

#include "CICalendarValue.h"

#include "CICalendarPeriod.h"

namespace iCal {

class CICalendarPeriodValue: public CICalendarValue
{
public:
	CICalendarPeriodValue() {}
	CICalendarPeriodValue(const CICalendarPeriod& value)
		{ mValue = value; }
	CICalendarPeriodValue(const CICalendarPeriodValue& copy)
		 : CICalendarValue(copy)
		{ _copy_CICalendarPeriodValue(copy); }
	virtual ~CICalendarPeriodValue() {}

	CICalendarPeriodValue& operator=(const CICalendarPeriodValue& copy)
		{ if (this != &copy) _copy_CICalendarPeriodValue(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarPeriodValue(*this); }

	virtual EICalValueType GetType() const
		{ return eValueType_Period; }

	virtual void Parse(const cdstring& data)
		{ mValue.Parse(data); }
	virtual void Generate(std::ostream& os) const
		{ mValue.Generate(os); }

	CICalendarPeriod& GetValue()
		{ return mValue; }
	const CICalendarPeriod& GetValue() const
		{ return mValue; }
	void SetValue(const CICalendarPeriod& value)
		{ mValue = value; }

protected:
	CICalendarPeriod mValue;

private:
	void _copy_CICalendarPeriodValue(const CICalendarPeriodValue& copy)
		{ mValue = copy.mValue; }
};

}	// namespace iCal

#endif	// CICalendarPeriodValue_H
