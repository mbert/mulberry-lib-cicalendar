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
	CICalendarProperty.h

	Author:
	Description:	<describe the CICalendarProperty class here>
*/

#ifndef CICalendarProperty_H
#define CICalendarProperty_H

#include "CICalendarAttribute.h"
#include "CICalendarValue.h"

#include <stdint.h>
#include <map>
#include <set>

#include "cdstring.h"


namespace iCal {

typedef uint32_t	CICalendarRef;	// Unique reference to object

class CICalendarCalAddressValue;
class CICalendarDateTimeValue;
class CICalendarDurationValue;
class CICalendarIntegerValue;
class CICalendarMultiValue;
class CICalendarPeriodValue;
class CICalendarPlainTextValue;
class CICalendarRecurrenceValue;
class CICalendarURIValue;
class CICalendarUTCOffsetValue;

class CICalendarDateTime;
typedef std::vector<CICalendarDateTime> CICalendarDateTimeList;
class CICalendarDuration;
class CICalendarPeriod;
class CICalendarRecurrence;

class CICalendarProperty
{
public:
	CICalendarProperty()
		{ _init_CICalendarProperty(); }
	CICalendarProperty(const cdstring& name, const int32_t& int_value)
		{ mName = name; _init_attr_value(int_value); }
	CICalendarProperty(const cdstring& name, const cdstring& text_value, CICalendarValue::EICalValueType value_type = CICalendarValue::eValueType_Text)
		{ mName = name; _init_attr_value(text_value, value_type); }
	CICalendarProperty(const cdstring& name, const CICalendarDateTime& dt)
		{ mName = name; _init_attr_value(dt); }
	CICalendarProperty(const cdstring& name, const CICalendarDateTimeList& dtl)
		{ mName = name; _init_attr_value(dtl); }
	CICalendarProperty(const cdstring& name, const CICalendarDuration& du)
		{ mName = name; _init_attr_value(du); }
	CICalendarProperty(const cdstring& name, const CICalendarPeriod& pe)
		{ mName = name; _init_attr_value(pe); }
	CICalendarProperty(const cdstring& name, const CICalendarRecurrence& recur)
		{ mName = name; _init_attr_value(recur); }
	CICalendarProperty(const CICalendarProperty& copy)
		{ _init_CICalendarProperty(); _copy_CICalendarProperty(copy); }
	virtual ~CICalendarProperty()
		{ _tidy_CICalendarProperty(); }

	CICalendarProperty& operator=(const CICalendarProperty& copy)
		{ if (this != &copy) _copy_CICalendarProperty(copy); return *this; }

	cdstring& GetName()
		{ return mName; }
	const cdstring& GetName() const
		{ return mName; }
	void SetName(const cdstring& name)
		{ mName = name; }

	CICalendarAttributeMap& GetAttributes()
		{ return mAttributes; }
	const CICalendarAttributeMap& GetAttributes() const
		{ return mAttributes; }
	void SetAttributes(const CICalendarAttributeMap& attributes)
		{ mAttributes = attributes; }
	bool HasAttribute(const cdstring& attr) const
	{
		return mAttributes.count(attr) != 0;
	}
	const cdstring& GetAttributeValue(const cdstring& attr) const
	{
		return (*mAttributes.find(attr)).second.GetFirstValue();
	}

	void AddAttribute(const CICalendarAttribute& attr);
	void RemoveAttributes(const cdstring& attr);

	CICalendarValue* GetValue()
		{ return mValue; }
	const CICalendarValue* GetValue() const
		{ return mValue; }

	const CICalendarCalAddressValue* GetCalAddressValue() const;
	const CICalendarDateTimeValue* GetDateTimeValue() const;
	const CICalendarDurationValue* GetDurationValue() const;
	const CICalendarIntegerValue* GetIntegerValue() const;
	const CICalendarMultiValue* GetMultiValue() const;
	const CICalendarPeriodValue* GetPeriodValue() const;
	const CICalendarRecurrenceValue* GetRecurrenceValue() const;
	const CICalendarPlainTextValue* GetTextValue() const;
	const CICalendarURIValue* GetURIValue() const;
	const CICalendarUTCOffsetValue* GetUTCOffsetValue() const;

	bool Parse(cdstring& data);
	void Generate(std::ostream& os) const;

protected:
	cdstring						mName;
	CICalendarAttributeMap			mAttributes;
	CICalendarValue*				mValue;

	typedef std::map<cdstring, CICalendarValue::EICalValueType>	CValueTypeMap;
	static CValueTypeMap		sDefaultValueTypeMap;
	static CValueTypeMap		sValueTypeMap;

	typedef std::map<CICalendarValue::EICalValueType, cdstring>	CTypeValueMap;
	static CTypeValueMap		sTypeValueMap;

	typedef cdstrset			CMultiValues;
	static CMultiValues			sMultiValues;

private:
	void _init_CICalendarProperty()
		{ mValue = NULL; }
	void _copy_CICalendarProperty(const CICalendarProperty& copy)
		{ _tidy_CICalendarProperty(); mName = copy.mName; mAttributes = copy.mAttributes; mValue = copy.mValue->clone(); }
	void _tidy_CICalendarProperty()
		{ delete mValue; mValue = NULL; }
	void _init_map();

	void CreateValue(const char* data);
	void SetupValueAttribute();


	// Creation
	void _init_attr_value(const int32_t& ival);
	void _init_attr_value(const cdstring& txt, CICalendarValue::EICalValueType value_type = CICalendarValue::eValueType_Text);
	void _init_attr_value(const CICalendarDateTime& dt);
	void _init_attr_value(const CICalendarDateTimeList& dtl);
	void _init_attr_value(const CICalendarDuration& du);
	void _init_attr_value(const CICalendarPeriod& pe);
	void _init_attr_value(const CICalendarRecurrence& recur);
};

typedef std::multimap<cdstring, CICalendarProperty> CICalendarPropertyMap;
typedef std::vector<CICalendarProperty> CICalendarPropertyList;

}	// namespace iCal

#endif	// CICalendarProperty_H
