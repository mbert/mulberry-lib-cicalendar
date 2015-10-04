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
	CICalendarProperty.cpp

	Author:
	Description:	<describe the CICalendarProperty class here>
*/

#include "CICalendarProperty.h"

#include "CICalendarCalAddressValue.h"
#include "CICalendarDateTimeValue.h"
#include "CICalendarDefinitions.h"
#include "CICalendarDurationValue.h"
#include "CICalendarIntegerValue.h"
#include "CICalendarMultiValue.h"
#include "CICalendarPeriodValue.h"
#include "CICalendarPlainTextValue.h"
#include "CICalendarRecurrenceValue.h"
#include "CICalendarURIValue.h"
#include "CICalendarUTCOffsetValue.h"
#include "CICalendarValue.h"
#include "CStringUtils.h"

#include <iostream>
#include <memory>
#include <strstream>

using namespace iCal;

#ifndef __VCPP__
CICalendarProperty::CValueTypeMap CICalendarProperty::sDefaultValueTypeMap;
CICalendarProperty::CValueTypeMap CICalendarProperty::sValueTypeMap;
CICalendarProperty::CTypeValueMap CICalendarProperty::sTypeValueMap;
CICalendarProperty::CMultiValues CICalendarProperty::sMultiValues;
#endif

void CICalendarProperty::_init_attr_value(const int32_t& ival)
{
	// Value
	mValue = new CICalendarIntegerValue(ival);

	// Attributes
	SetupValueAttribute();
}

void CICalendarProperty::_init_attr_value(const cdstring& txt, CICalendarValue::EICalValueType value_type)
{
	// Value
	mValue = CICalendarValue::CreateFromType(value_type);
	CICalendarPlainTextValue* tvalue = dynamic_cast<CICalendarPlainTextValue*>(mValue);
	if (tvalue != NULL)
		tvalue->SetValue(txt);

	// Attributes
	SetupValueAttribute();
}

void CICalendarProperty::_init_attr_value(const CICalendarDateTime& dt)
{
	// Value
	mValue = new CICalendarDateTimeValue(dt);

	// Attributes
	SetupValueAttribute();
	
	// Look for timezone
	if (!dt.IsDateOnly() && !dt.GetTimezone().GetUTC() && !dt.GetTimezone().GetTimezoneID().empty())
	{
		mAttributes.erase(cICalAttribute_TZID);
		mAttributes.insert(CICalendarAttributeMap::value_type(cICalAttribute_TZID, CICalendarAttribute(cICalAttribute_TZID, dt.GetTimezone().GetTimezoneID())));
	}
}

void CICalendarProperty::_init_attr_value(const CICalendarDateTimeList& dtl)
{
	// Value
	mValue = new CICalendarMultiValue((dtl.size() > 0) && dtl.front().IsDateOnly() ? CICalendarValue::eValueType_Date : CICalendarValue::eValueType_DateTime);
	for(CICalendarDateTimeList::const_iterator iter = dtl.begin(); iter != dtl.end(); iter++)
	{
		static_cast<CICalendarMultiValue*>(mValue)->AddValue(new CICalendarDateTimeValue(*iter));
	}

	// Attributes
	SetupValueAttribute();
	
	// Look for timezone
	if ((dtl.size() > 0) && !dtl.front().IsDateOnly() && !dtl.front().GetTimezone().GetUTC() && !dtl.front().GetTimezone().GetTimezoneID().empty())
	{
		mAttributes.erase(cICalAttribute_TZID);
		mAttributes.insert(CICalendarAttributeMap::value_type(cICalAttribute_TZID, CICalendarAttribute(cICalAttribute_TZID, dtl.front().GetTimezone().GetTimezoneID())));
	}
}

void CICalendarProperty::_init_attr_value(const CICalendarDuration& du)
{
	// Value
	mValue = new CICalendarDurationValue(du);

	// Attributes
	SetupValueAttribute();
}

void CICalendarProperty::_init_attr_value(const CICalendarPeriod& pe)
{
	// Value
	mValue = new CICalendarPeriodValue(pe);

	// Attributes
	SetupValueAttribute();
}

void CICalendarProperty::_init_attr_value(const CICalendarRecurrence& recur)
{
	// Value
	mValue = new CICalendarRecurrenceValue(recur);

	// Attributes
	SetupValueAttribute();
}

void CICalendarProperty::_init_map()
{
	// Only if empty
	if (sDefaultValueTypeMap.empty())
	{
		// 2445 4.8.1
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_ATTACH, CICalendarValue::eValueType_URI));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_CATEGORIES, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_CLASS, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_COMMENT, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_DESCRIPTION, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_GEO, CICalendarValue::eValueType_Geo));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_LOCATION, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_PERCENT_COMPLETE, CICalendarValue::eValueType_Integer));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_PRIORITY, CICalendarValue::eValueType_Integer));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_RESOURCES, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_STATUS, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_SUMMARY, CICalendarValue::eValueType_Text));

		// 2445 4.8.2
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_COMPLETED, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_DTEND, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_DUE, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_DTSTART, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_DURATION, CICalendarValue::eValueType_Duration));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_FREEBUSY, CICalendarValue::eValueType_Period));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_TRANSP, CICalendarValue::eValueType_Text));

		// 2445 4.8.3
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_TZID, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_TZNAME, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_TZOFFSETFROM, CICalendarValue::eValueType_UTC_Offset));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_TZOFFSETTO, CICalendarValue::eValueType_UTC_Offset));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_TZURL, CICalendarValue::eValueType_URI));

		// 2445 4.8.4
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_ATTENDEE, CICalendarValue::eValueType_CalAddress));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_CONTACT, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_ORGANIZER, CICalendarValue::eValueType_CalAddress));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_RECURRENCE_ID, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_RELATED_TO, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_URL, CICalendarValue::eValueType_URI));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_UID, CICalendarValue::eValueType_Text));

		// 2445 4.8.5
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_EXDATE, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_EXRULE, CICalendarValue::eValueType_Recur));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_RDATE, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_RRULE, CICalendarValue::eValueType_Recur));

		// 2445 4.8.6
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_ACTION, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_REPEAT, CICalendarValue::eValueType_Integer));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_TRIGGER, CICalendarValue::eValueType_Duration));

		// 2445 4.8.7
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_CREATED, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_DTSTAMP, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_LAST_MODIFIED, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_SEQUENCE, CICalendarValue::eValueType_Integer));
		
		// Apple Extensions
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_XWRCALNAME, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_XWRCALDESC, CICalendarValue::eValueType_Text));
		
		// Mulberry extensions
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_X_PRIVATE_RURL, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_X_PRIVATE_ETAG, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_ACTION_X_SPEAKTEXT, CICalendarValue::eValueType_Text));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_ALARM_X_LASTTRIGGER, CICalendarValue::eValueType_DateTime));
		sDefaultValueTypeMap.insert(CValueTypeMap::value_type(cICalProperty_ALARM_X_ALARMSTATUS, CICalendarValue::eValueType_Text));
	}

	// Only if empty
	if (sValueTypeMap.empty())
	{
		// 2445 4.3
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_BINARY, CICalendarValue::eValueType_Binary));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_BOOLEAN, CICalendarValue::eValueType_Boolean));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_CAL_ADDRESS, CICalendarValue::eValueType_CalAddress));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_DATE, CICalendarValue::eValueType_Date));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_DATE_TIME, CICalendarValue::eValueType_DateTime));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_DURATION, CICalendarValue::eValueType_Duration));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_FLOAT, CICalendarValue::eValueType_Float));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_INTEGER, CICalendarValue::eValueType_Integer));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_PERIOD, CICalendarValue::eValueType_Period));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_RECUR, CICalendarValue::eValueType_Recur));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_TEXT, CICalendarValue::eValueType_Text));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_TIME, CICalendarValue::eValueType_Time));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_URI, CICalendarValue::eValueType_URI));
		sValueTypeMap.insert(CValueTypeMap::value_type(cICalValue_UTC_OFFSET, CICalendarValue::eValueType_UTC_Offset));

	}

	if (sTypeValueMap.empty())
	{
		// 2445 4.3
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Binary, cICalValue_BINARY));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Boolean, cICalValue_BOOLEAN));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_CalAddress, cICalValue_CAL_ADDRESS));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Date, cICalValue_DATE));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_DateTime, cICalValue_DATE_TIME));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Duration, cICalValue_DURATION));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Float, cICalValue_FLOAT));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Geo, cICalValue_FLOAT));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Integer, cICalValue_INTEGER));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Period, cICalValue_PERIOD));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Recur, cICalValue_RECUR));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Text, cICalValue_TEXT));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_Time, cICalValue_TIME));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_URI, cICalValue_URI));
		sTypeValueMap.insert(CTypeValueMap::value_type(CICalendarValue::eValueType_UTC_Offset, cICalValue_UTC_OFFSET));

	}

	if (sMultiValues.empty())
	{
		sMultiValues.insert(cICalProperty_CATEGORIES);
		sMultiValues.insert(cICalProperty_RESOURCES);
		sMultiValues.insert(cICalProperty_FREEBUSY);
		sMultiValues.insert(cICalProperty_EXDATE);
		sMultiValues.insert(cICalProperty_RDATE);
	}
}

void CICalendarProperty::AddAttribute(const CICalendarAttribute& attr)
{
	mAttributes.insert(CICalendarAttributeMap::value_type(attr.GetName(), attr));
}

void CICalendarProperty::RemoveAttributes(const cdstring& attr)
{
	mAttributes.erase(attr);
}

const CICalendarCalAddressValue* CICalendarProperty::GetCalAddressValue() const
{
	return dynamic_cast<CICalendarCalAddressValue*>(mValue);
}

const CICalendarDateTimeValue* CICalendarProperty::GetDateTimeValue() const
{
	return dynamic_cast<CICalendarDateTimeValue*>(mValue);
}

const CICalendarDurationValue* CICalendarProperty::GetDurationValue() const
{
	return dynamic_cast<CICalendarDurationValue*>(mValue);
}

const CICalendarIntegerValue* CICalendarProperty::GetIntegerValue() const
{
	return dynamic_cast<CICalendarIntegerValue*>(mValue);
}

const CICalendarMultiValue* CICalendarProperty::GetMultiValue() const
{
	return dynamic_cast<CICalendarMultiValue*>(mValue);
}

const CICalendarPeriodValue* CICalendarProperty::GetPeriodValue() const
{
	return dynamic_cast<CICalendarPeriodValue*>(mValue);
}

const CICalendarRecurrenceValue* CICalendarProperty::GetRecurrenceValue() const
{
	return dynamic_cast<CICalendarRecurrenceValue*>(mValue);
}

const CICalendarPlainTextValue* CICalendarProperty::GetTextValue() const
{
	return dynamic_cast<CICalendarPlainTextValue*>(mValue);
}

const CICalendarURIValue* CICalendarProperty::GetURIValue() const
{
	return dynamic_cast<CICalendarURIValue*>(mValue);
}

const CICalendarUTCOffsetValue* CICalendarProperty::GetUTCOffsetValue() const
{
	return dynamic_cast<CICalendarUTCOffsetValue*>(mValue);
}

bool CICalendarProperty::Parse(cdstring& data)
{
	char* p = const_cast<char*>(data.c_str());

	// Look for attribute or value delimiter
	{
		std::auto_ptr<char> prop_name(::strduptokenstr(&p, ";:"));
		if ((prop_name.get() == NULL) || (*p == 0))
			return false;

		// We have the name
		mName.assign(prop_name.get());
	}

	// Now loop getting data
	bool done = false;
	while(!done && (*p != 0))
	{
		switch(*p)
		{
		case ';':
			// Parse attribute
			{
				// Move past delimiter
				p++;

				// Get quoted string or token
				std::auto_ptr<char> attribute_name(::strduptokenstr(&p, "="));
				if (attribute_name.get() == NULL)
					return false;
				p++;
				std::auto_ptr<char> attribute_value(::strduptokenstr(&p, ":;,"));
				// CD: parameter values can be empty
				//if (attribute_value.get() == NULL)
				//	return false;

				// Now add attribute value
				CICalendarAttribute attrvalue(attribute_name.get(), attribute_value.get());
				mAttributes.insert(CICalendarAttributeMap::value_type(attrvalue.GetName(), attrvalue));

				// Look for additional values
				while(*p == ',')
				{
					p++;
					std::auto_ptr<char> attribute_value2(::strduptokenstr(&p, ":;,"));
					if (attribute_value2.get() != NULL)
						attrvalue.AddValue(attribute_value2.get());
				}
			}
			break;
		case ':':
			CreateValue(p + 1);
			done = true;
			break;
		default:;
		}
	}
	
	// We must have a value of some kind
	return (mValue != NULL);
}

void CICalendarProperty::CreateValue(const char* data)
{
	// Tidy first
	delete mValue;
	mValue = NULL;

	// Get value type from property name
	CICalendarValue::EICalValueType type = CICalendarValue::eValueType_Text;
	_init_map();
	CValueTypeMap::const_iterator found = sDefaultValueTypeMap.find(mName);
	if (found != sDefaultValueTypeMap.end())
		type = (*found).second;

	// Check whether custom value is set
	if (mAttributes.count(cICalAttribute_VALUE))
	{
		CValueTypeMap::const_iterator found = sValueTypeMap.find(GetAttributeValue(cICalAttribute_VALUE));
		if (found != sValueTypeMap.end())
			type = (*found).second;
	}

	// Check for multivalued
	if (sMultiValues.count(mName))
	{
		mValue = new CICalendarMultiValue(type);
	}
	else
	{
		// Create the type
		mValue = CICalendarValue::CreateFromType(type);
	}

	// Now parse the data
	mValue->Parse(data);

	// Special post-create for some types
	switch(type)
	{
	case CICalendarValue::eValueType_Time:
	case CICalendarValue::eValueType_DateTime:
	{
		// Look for TZID attribute
		cdstring tzid;
		if (HasAttribute(cICalAttribute_TZID))
		{
			tzid = GetAttributeValue(cICalAttribute_TZID);
		}
		
		if (dynamic_cast<CICalendarDateTimeValue*>(mValue) != NULL)
			static_cast<CICalendarDateTimeValue*>(mValue)->GetValue().GetTimezone().SetTimezoneID(tzid);
		else if (dynamic_cast<CICalendarMultiValue*>(mValue) != NULL)
		{
			for(CICalendarValueList::iterator iter = static_cast<CICalendarMultiValue*>(mValue)->GetValues().begin(); iter != static_cast<CICalendarMultiValue*>(mValue)->GetValues().end(); iter++)
			{
				if (dynamic_cast<CICalendarDateTimeValue*>(*iter) != NULL)
					static_cast<CICalendarDateTimeValue*>(*iter)->GetValue().GetTimezone().SetTimezoneID(tzid);
			}
		}
		break;
	}
	default:;
	}
}

// Make sure current VALUE= attribute matches the actual data we have
// or is absent if default value
void CICalendarProperty::SetupValueAttribute()
{
	mAttributes.erase(cICalAttribute_VALUE);

	// Only if we have a value right now
	if (mValue == NULL)
		return;

	// See if current type is default for this property
	_init_map();
	CValueTypeMap::const_iterator found = sDefaultValueTypeMap.find(mName);
	if (found != sDefaultValueTypeMap.end())
	{
		CICalendarValue::EICalValueType default_type = (*found).second;
		if (default_type != mValue->GetType())
		{
			CTypeValueMap::const_iterator found2 = sTypeValueMap.find(mValue->GetType());
			if (found2 != sTypeValueMap.end())
			{
				mAttributes.insert(CICalendarAttributeMap::value_type(cICalAttribute_VALUE, CICalendarAttribute(cICalAttribute_VALUE, (*found2).second)));
			}
		}
	}
}

void CICalendarProperty::Generate(std::ostream& os) const
{
	const_cast<CICalendarProperty*>(this)->SetupValueAttribute();

	// Must write to temp buffer and then wrap
	std::ostrstream sout;
	
	sout << mName;

	// Write all attributes
	for(CICalendarAttributeMap::const_iterator iter = mAttributes.begin(); iter != mAttributes.end(); iter++)
	{
		sout << ";";
		(*iter).second.Generate(sout);
	}

	// Write value
	sout << ":";
	if (mValue)
		mValue->Generate(sout);
	
	sout << std::ends;
	cdstring temp;
	temp.steal(sout.str());
	if (temp.length() < 75)
		os << temp.c_str();
	else
	{
		const char* p = temp.c_str();
		const char* q = p + temp.length();
		while(p < q)
		{
			if (p != temp.c_str())
			{
				os << net_endl << " ";
			}

			int bytes = q - p;
			if (bytes > 74)
			{
				bytes = 74;
				
				// Make sure we do not split in the middle of a utf-8 multi-octet sequence
				unsigned char* up = (unsigned char*) p;
				while((up[bytes] > 0x7F) && ((up[bytes] & 0xC0) == 0x80))
					bytes--;
			}
			os.write(p, bytes);
			p += bytes;
		}
	}

	os << net_endl;
}

