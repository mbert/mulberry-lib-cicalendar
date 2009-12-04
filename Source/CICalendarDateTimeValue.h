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
	CICalendarDateTimeValue.h

	Author:
	Description:	<describe the CICalendarDateTimeValue class here>
*/

#ifndef CICalendarDateTimeValue_H
#define CICalendarDateTimeValue_H

#include "CICalendarValue.h"

#include "CICalendarDateTime.h"


namespace iCal {

class CICalendarDateTimeValue : public CICalendarValue
{
public:
	CICalendarDateTimeValue() {}
	CICalendarDateTimeValue(const CICalendarDateTime& value)
		{ mValue = value; }
	CICalendarDateTimeValue(const CICalendarDateTimeValue& copy)
		 : CICalendarValue(copy)
		{ _copy_CICalendarDateTimeValue(copy); }
	virtual ~CICalendarDateTimeValue() {}

	CICalendarDateTimeValue& operator=(const CICalendarDateTimeValue& copy)
		{ if (this != &copy) _copy_CICalendarDateTimeValue(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarDateTimeValue(*this); }

	virtual EICalValueType GetType() const
		{ return mValue.IsDateOnly() ? eValueType_Date : eValueType_DateTime; }

	virtual void Parse(const cdstring& data)
		{ mValue.Parse(data); }
	virtual void Generate(std::ostream& os) const
		{ mValue.Generate(os); }

	CICalendarDateTime& GetValue()
		{ return mValue; }
	const CICalendarDateTime& GetValue() const
		{ return mValue; }
	void SetValue(const CICalendarDateTime& value)
		{ mValue = value; }

protected:
	CICalendarDateTime mValue;

private:
	void _copy_CICalendarDateTimeValue(const CICalendarDateTimeValue& copy)
		{ mValue = copy.mValue; }
};

}	// namespace iCal

#endif	// CICalendarDateTimeValue_H
