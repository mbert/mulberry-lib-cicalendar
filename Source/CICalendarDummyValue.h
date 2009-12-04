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
	CICalendarDummyValue.h

	Author:
	Description:	<describe the CICalendarDummyValue class here>
*/

#ifndef CICalendarDummyValue_H
#define CICalendarDummyValue_H

#include "CICalendarValue.h"


namespace iCal {

class CICalendarDummyValue : public CICalendarValue
{
public:
	CICalendarDummyValue(EICalValueType type)
		{ mType = type; }
	CICalendarDummyValue(EICalValueType type, const cdstring& value)
		{ mType = type; mValue = value; }
	CICalendarDummyValue(const CICalendarDummyValue& copy)
		 : CICalendarValue(copy)
		{ _copy_CICalendarDummyValue(copy); }
	virtual ~CICalendarDummyValue() {}

	CICalendarDummyValue& operator=(const CICalendarDummyValue& copy)
		{ if (this != &copy) _copy_CICalendarDummyValue(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarDummyValue(*this); }

	virtual EICalValueType GetType() const
		{ return mType; }

	virtual void Parse(const cdstring& data);
	virtual void Generate(std::ostream& os) const;

	cdstring& GetValue()
		{ return mValue; }
	const cdstring& GetValue() const
		{ return mValue; }
	void SetValue(const cdstring& value)
		{ mValue = value; }

protected:
	EICalValueType		mType;
	cdstring			mValue;

private:
	void _copy_CICalendarDummyValue(const CICalendarDummyValue& copy)
		{ mType = copy.mType; mValue = copy.mValue; }
};

}	// namespace iCal

#endif	// CICalendarDummyValue_H
