/*
    Copyright (c) 2007-2010 Cyrus Daboo. All rights reserved.
    
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
	CICalendar.cpp

	Author:
	Description:	<describe the CICalendar class here>
*/

#include "CICalendar.h"

#include "CICalendarComponentExpanded.h"
#include "CICalendarDefinitions.h"
#include "CICalendarTextValue.h"
#include "CICalendarVAlarm.h"
#include "CICalendarVEvent.h"
#include "CICalendarVFreeBusy.h"
#include "CICalendarVJournal.h"
#include "CICalendarVTimezone.h"
#include "CICalendarVTimezoneDaylight.h"
#include "CICalendarVTimezoneStandard.h"
#include "CICalendarVToDo.h"
#include "CICalendarUtils.h"

#include "XMLDocument.h"
#include "XMLNode.h"
#include "XMLSAXSimple.h"

#include <algorithm>
#include <cstring>
#include <memory>

using namespace iCal;

CICalendar* CICalendar::GetICalendar(const CICalendarRef& ref)
{
	CICalendarRefMap::iterator found = sICalendars.find(ref);
	if (found != sICalendars.end())
		return (*found).second;
	else
		return NULL;
}

CICalendar::CComponentRegisterMap CICalendar::sComponents;
CICalendar::CComponentRegisterMap CICalendar::sEmbeddedComponents;

CICalendar::CICalendar()
{
	mICalendarRef = sICalendarRefCtr++;
	sICalendars.insert(CICalendarRefMap::value_type(mICalendarRef, this));

	mReadOnly = false;
	mDirty = false;

	AddDefaultProperties();

	// Special init for static item
	//if (this == &getSICalendar())
	//{
	//	InitDefaultTimezones();
	//}
}

CICalendar::~CICalendar()
{
	// Broadcast closing before removing components
	Broadcast_Message(eBroadcast_Closed, this);

	// Clean up the map items
	mVEvent.RemoveAllComponents();
	mVToDo.RemoveAllComponents();
	mVJournal.RemoveAllComponents();
	mVFreeBusy.RemoveAllComponents();
	mVTimezone.RemoveAllComponents();

	sICalendars.erase(mICalendarRef);
}

void CICalendar::InitComponents()
{
	if (sComponents.empty())
	{
		sComponents.insert(CComponentRegisterMap::value_type(CICalendarVEvent::GetVBegin(), new SComponentRegister(CICalendarVEvent::Create, CICalendarComponent::eVEVENT)));
		sComponents.insert(CComponentRegisterMap::value_type(CICalendarVToDo::GetVBegin(), new SComponentRegister(CICalendarVToDo::Create, CICalendarComponent::eVTODO)));
		sComponents.insert(CComponentRegisterMap::value_type(CICalendarVJournal::GetVBegin(), new SComponentRegister(CICalendarVJournal::Create, CICalendarComponent::eVJOURNAL)));
		sComponents.insert(CComponentRegisterMap::value_type(CICalendarVFreeBusy::GetVBegin(), new SComponentRegister(CICalendarVFreeBusy::Create, CICalendarComponent::eVFREEBUSY)));
		sComponents.insert(CComponentRegisterMap::value_type(CICalendarVTimezone::GetVBegin(), new SComponentRegister(CICalendarVTimezone::Create, CICalendarComponent::eVTIMEZONE)));
	}
	if (sEmbeddedComponents.empty())
	{
		sEmbeddedComponents.insert(CComponentRegisterMap::value_type(CICalendarVAlarm::GetVBegin(), new SComponentRegister(CICalendarVAlarm::Create, CICalendarComponent::eVALARM)));
		sEmbeddedComponents.insert(CComponentRegisterMap::value_type(CICalendarVTimezoneStandard::GetVBegin(), new SComponentRegister(CICalendarVTimezoneStandard::Create, CICalendarComponent::eVTIMEZONE)));
		sEmbeddedComponents.insert(CComponentRegisterMap::value_type(CICalendarVTimezoneDaylight::GetVBegin(), new SComponentRegister(CICalendarVTimezoneDaylight::Create, CICalendarComponent::eVTIMEZONE)));
	}
}

void CICalendar::InitDefaultTimezones()
{
	// Add default timezones to this (static) calendar
}

void CICalendar::Clear()
{
	mName = cdstring::null_str;
	mDescription = cdstring::null_str;
	mReadOnly = false;
	mDirty = false;

	// Remove recording of old data - but leave the etag as-is
	ClearRecording();

	// Remove existing calendar properties then add back the defaults
	mProperties.clear();
	AddDefaultProperties();

	// Clean up the map items
	mVEvent.RemoveAllComponents();
	mVToDo.RemoveAllComponents();
	mVJournal.RemoveAllComponents();
	mVFreeBusy.RemoveAllComponents();
	mVTimezone.RemoveAllComponents();
}

CICalendarComponentDB& CICalendar::GetComponents(CICalendarComponent::EComponentType type)
{
	switch(type)
	{
	case CICalendarComponent::eVEVENT:
	default:
		return mVEvent;
	case CICalendarComponent::eVTODO:
		return mVToDo;
	case CICalendarComponent::eVJOURNAL:
		return mVJournal;
	case CICalendarComponent::eVFREEBUSY:
		return mVFreeBusy;
	case CICalendarComponent::eVTIMEZONE:
		return mVTimezone;
	}
}

void CICalendar::AddDefaultProperties()
{
	AddProperty(CICalendarProperty(cICalProperty_PRODID, "-//mulberrymail.com//Mulberry v4.0//EN"));
	AddProperty(CICalendarProperty(cICalProperty_VERSION, "2.0"));
	AddProperty(CICalendarProperty(cICalProperty_CALSCALE, "GREGORIAN"));
}

void CICalendar::Finalise()
{
	// Get calendar name if present

	// Get X-WR-CALNAME
	LoadValue(cICalProperty_XWRCALNAME, mName);

	// Get X-WR-CALDESC
	LoadValue(cICalProperty_XWRCALDESC, mDescription);
}

void CICalendar::EditName(const cdstring& name)
{
	if (mName != name)
	{
		// Updated cached value
		mName = name;

		// Remove existing items
		RemoveProperties(cICalProperty_XWRCALNAME);
		
		// Now create properties
		if (name.length())
		{
			AddProperty(CICalendarProperty(cICalProperty_XWRCALNAME, name));
		}
		
		// Mark as dirty
		SetDirty();
		
		// Broadcast change
		Broadcast_Message(eBroadcast_Edit, this);
	}
}

void CICalendar::EditDescription(const cdstring& description)
{
	if (mDescription != description)
	{
		// Updated cached value
		mDescription = description;

		// Remove existing items
		RemoveProperties(cICalProperty_XWRCALDESC);
		
		// Now create properties
		if (description.length())
		{
			AddProperty(CICalendarProperty(cICalProperty_XWRCALDESC, description));
		}
		
		// Mark as dirty
		SetDirty();
		
		// Broadcast change
		Broadcast_Message(eBroadcast_Edit, this);
	}
}

cdstring CICalendar::GetMethod() const
{
	cdstring result;
	if (HasProperty(cICalProperty_METHOD))
		LoadValue(cICalProperty_METHOD, result);
	return result;
}

bool CICalendar::Parse(std::istream& is)
{
	// Always init rhe component maps
	InitComponents();

	bool result = false;

	enum EParserState
	{
		eLookForVCalendar,
		eGetPropertyOrComponent,
		eGetComponentProperty,
		eGetSubComponentProperty
	};

	EParserState state = eLookForVCalendar;

	// Get lines looking for start of calendar
	cdstring line1;
	cdstring line2;
	CICalendarComponent* comp = NULL;
	CICalendarComponent* prevcomp = NULL;
	CICalendarComponentDB* compmap = NULL;

	while(!is.fail() && CICalendarUtils::ReadFoldedLine(is, line1, line2))
	{
		switch(state)
		{
		case eLookForVCalendar:
			// Look for start
			if (line1.compare(cICalComponent_BEGINVCALENDAR) == 0)
			{
				// Next state
				state = eGetPropertyOrComponent;

				// Indicate success at this point
				result = true;
			}
			break;
		case eGetPropertyOrComponent:
		{
			// Parse property or look for start of component
			CComponentRegisterMap::const_iterator found = sComponents.find(line1);
			if (found != sComponents.end())
			{
				// Start a new component
				comp = (*found).second->mCreatePP(GetRef());

				// Set the marker for the end of this component and the map to store it in
				compmap = &GetComponents((*found).second->mType);

				// Change state
				state = eGetComponentProperty;
			}
			else if (line1.compare(cICalComponent_ENDVCALENDAR) == 0)
			{
				// Finalise the current calendar
				Finalise();

				// Change state
				state = eLookForVCalendar;
			}
			else
			{
				// Parse attribute/value for top-level calendar item
				CICalendarProperty prop;
				if (prop.Parse(line1))
				{
					// Check for valid property
					if (!ValidProperty(prop))
						return false;
					else if (!IgnoreProperty(prop))
						AddProperty(prop);
				}
			}
			break;
		}
		case eGetComponentProperty:
		case eGetSubComponentProperty:
			// Look for end of current component
			if (line1.compare(comp->GetEndDelimiter()) == 0)
			{
				// Finalise the component (this caches data from the properties)
				comp->Finalise();

				// Check whether this is embedded
				if (prevcomp != NULL)
				{
					// Embed component in parent and reset to use parent
					if (!prevcomp->AddComponent(comp))
						delete comp;
					comp = prevcomp;
					prevcomp = NULL;

					// Reset state
					state = eGetComponentProperty;
				}
				else
				{
					// Check for valid component
					if (!compmap->AddComponent(comp))
						delete comp;
					comp = NULL;
					compmap = NULL;

					// Reset state
					state = eGetPropertyOrComponent;
				}
			}
			else
			{
				// Look for start of embedded component (can only do once)
				CComponentRegisterMap::const_iterator found = sEmbeddedComponents.find(line1);
				if ((state != eGetSubComponentProperty) && (found != sEmbeddedComponents.end()))
				{
					// Start a new component (saving off the current one)
					prevcomp = comp;
					comp = (*found).second->mCreatePP(GetRef());

					// Reset state
					state = eGetSubComponentProperty;
				}
				else
				{
					// Parse attribute/value and store in component
					CICalendarProperty prop;
					if (prop.Parse(line1))
						comp->AddProperty(prop);
				}
			}
			break;
		}
	}

	// We need to store all timezones in the static object so they can be accessed by any date object
	if (this != &getSICalendar())
	{
		getSICalendar().MergeTimezones(*this);
	}

	return result;
}

iCal::CICalendarComponent* CICalendar::ParseComponent(std::istream& is, const cdstring& rurl, const cdstring& etag)
{
	CICalendarComponent* result = NULL;

	// Always init rhe component maps
	InitComponents();

	enum EParserState
	{
		eLookForVCalendar,
		eGetPropertyOrComponent,
		eGetComponentProperty,
		eGetSubComponentProperty,
		eGotVCalendar
	};

	EParserState state = eLookForVCalendar;

	// Get lines looking for start of calendar
	cdstring line1;
	cdstring line2;
	CICalendarComponent* comp = NULL;
	CICalendarComponent* prevcomp = NULL;
	CICalendarComponentDB* compmap = NULL;
	std::auto_ptr<CICalendarProperty> method;
	bool got_timezone = false;

	while(!is.fail() && CICalendarUtils::ReadFoldedLine(is, line1, line2))
	{
		switch(state)
		{
		case eLookForVCalendar:
			// Look for start
			if (line1.compare(cICalComponent_BEGINVCALENDAR) == 0)
			{
				// Next state
				state = eGetPropertyOrComponent;
			}
			break;
		case eGetPropertyOrComponent:
		{
			// Parse property or look for start of component
			CComponentRegisterMap::const_iterator found = sComponents.find(line1);
			if (found != sComponents.end())
			{
				// Start a new component
				comp = (*found).second->mCreatePP(GetRef());
				
				// Set the marker for the end of this component and the map to store it in
				compmap = &GetComponents((*found).second->mType);

				// Look for timezone component to trigger timezone merge only if one is present
				if ((*found).second->mType == CICalendarComponent::eVTIMEZONE)
					got_timezone = true;
				else
				{
					if (method.get() != NULL)
					{
						comp->AddProperty(*method);
					}
					if (result == NULL)
						result = comp;
				}

				// Change state
				state = eGetComponentProperty;
			}
			else if (line1.compare(cICalComponent_ENDVCALENDAR) == 0)
			{
				// Change state
				state = eGotVCalendar;
			}
			else
			{
				// Ignore top-level items except METHOD

				// Parse attribute/value for top-level calendar item
				std::auto_ptr<CICalendarProperty> prop(new CICalendarProperty);
				if (prop->Parse(line1))
				{
					// Check for METHOD
					if (prop->GetName() == cICalProperty_METHOD)
					{
						// Adjust method to X-METHOD
						method = prop;
						method->SetName(cICalProperty_X_PRIVATE_METHOD);
					}
				}
			}
			break;
		}
		case eGetComponentProperty:
		case eGetSubComponentProperty:
			// Look for end of current component
			if (line1.compare(comp->GetEndDelimiter()) == 0)
			{
				// Finalise the component (this caches data from the properties)
				comp->Finalise();
				comp->SetRURL(rurl);
				comp->SetETag(etag);

				// Check whether this is embedded
				if (prevcomp != NULL)
				{
					// Embed component in parent and reset to use parent
					if (!prevcomp->AddComponent(comp))
						delete comp;
					comp = prevcomp;
					prevcomp = NULL;

					// Reset state
					state = eGetComponentProperty;
				}
				else
				{
					// Check for valid component
					if (!compmap->AddComponent(comp))
					{
						if (result == comp)
							result = NULL;
						delete comp;
					}
					comp = NULL;
					compmap = NULL;

					// Reset state
					state = eGetPropertyOrComponent;
				}
			}
			else
			{
				// Look for start of embedded component (can only do once)
				CComponentRegisterMap::const_iterator found = sEmbeddedComponents.find(line1);
				if ((state != eGetSubComponentProperty) && (found != sEmbeddedComponents.end()))
				{
					// Start a new component (saving off the current one)
					prevcomp = comp;
					comp = (*found).second->mCreatePP(GetRef());

					// Reset state
					state = eGetSubComponentProperty;
				}
				else
				{
					// Parse attribute/value and store in component
					CICalendarProperty prop;
					if (prop.Parse(line1))
						comp->AddProperty(prop);
				}
			}
			break;
		default:;
		}
		
		// Exit if we have one - ignore all the rest
		if (state == eGotVCalendar)
			break;
	}

	// We need to store all timezones in the static object so they can be accessed by any date object
	// Only do this if we read in a timezone
	if (got_timezone && (this != &getSICalendar()))
	{
		getSICalendar().MergeTimezones(*this);
	}
	
	return result;
}

bool CICalendar::ValidProperty(const CICalendarProperty& prop) const
{
	if (prop.GetName() == cICalProperty_VERSION)
	{
		const CICalendarPlainTextValue* tvalue = prop.GetTextValue();
		if ((tvalue == NULL) || (tvalue->GetValue() != "2.0"))
			return false;
	}
	else if (prop.GetName() == cICalProperty_CALSCALE)
	{
		const CICalendarPlainTextValue* tvalue = prop.GetTextValue();
		if ((tvalue == NULL) || (tvalue->GetValue() != "GREGORIAN"))
			return false;
	}

	return true;
}

bool CICalendar::IgnoreProperty(const CICalendarProperty& prop) const
{
	return (prop.GetName() == cICalProperty_VERSION) ||
			(prop.GetName() == cICalProperty_CALSCALE) ||
			(prop.GetName() == cICalProperty_PRODID);
}

void CICalendar::Generate(std::ostream& os, bool for_cache) const
{
	// Make sure all required timezones are in this object
	const_cast<CICalendar*>(this)->IncludeTimezones();

	// Write header
	os << cICalComponent_BEGINVCALENDAR << net_endl;

	// Write properties (we always handle PRODID & VERSION)
	WriteProperties(os);

	// Write out each type of component (not VALARMS which are embedded in others)
	// Do VTIMEZONES at the start
	Generate(os, mVTimezone, for_cache);
	Generate(os, mVEvent, for_cache);
	Generate(os, mVToDo, for_cache);
	Generate(os, mVJournal, for_cache);
	Generate(os, mVFreeBusy, for_cache);

	// Write trailer
	os << cICalComponent_ENDVCALENDAR << net_endl;
}

void CICalendar::GenerateOne(std::ostream& os, const CICalendarComponent& comp) const
{
	// Write header
	os << cICalComponent_BEGINVCALENDAR << net_endl;

	// Write properties (we always handle PRODID & VERSION)
	WriteProperties(os);

	// Make sure each timezone is written out
	cdstrset tzids;
	comp.GetTimezones(tzids);
	for(cdstrset::const_iterator iter = tzids.begin(); iter != tzids.end(); iter++)
	{
		const CICalendarVTimezone* tz = GetTimezone(*iter);
		if (tz == NULL)
		{
			// Find it in the static object
			tz = getSICalendar().GetTimezone(*iter);
		}
		
		if (tz != NULL)
			tz->Generate(os);
	}

	// Check for recurring component and potentially write out all instances
	const CICalendarComponentRecur* recur = dynamic_cast<const CICalendarComponentRecur*>(&comp);
	if (recur != NULL)
	{
		// Write this one out first
		comp.Generate(os);
		
		// Get list of all instances
		CICalendarComponentRecurs instances;
		GetRecurrenceInstances(comp.GetType(), comp.GetUID(), instances);

		// Write each instance out
		for(CICalendarComponentRecurs::const_iterator iter = instances.begin(); iter != instances.end(); iter++)
		{
			// Write the component
			(*iter)->Generate(os);
		}
	}
	else
		// Write the component
		comp.Generate(os);

	// Write trailer
	os << cICalComponent_ENDVCALENDAR << net_endl;
}

void CICalendar::Generate(std::ostream& os, const CICalendarComponentDB& components, bool for_cache) const
{
	for(CICalendarComponentDB::const_iterator iter = components.begin(); iter != components.end(); iter++)
		(*iter).second->Generate(os, for_cache);
}

bool CICalendar::HasData() const
{
	return (mVEvent.size() != 0) ||
			(mVToDo.size() != 0) ||
			(mVJournal.size() != 0) ||
			(mVFreeBusy.size() != 0);
}

// Get components based on requirements

void CICalendar::GetVEvents(const CICalendarPeriod& period, CICalendarExpandedComponents& list, bool all_day_at_top) const
{
	// Look at each VEvent
	for(CICalendarComponentDB::const_iterator iter = mVEvent.begin(); iter != mVEvent.end(); iter++)
	{
		CICalendarVEvent* vevent = static_cast<CICalendarVEvent*>((*iter).second);
		vevent->ExpandPeriod(period, list);
	}
	
	std::sort(list.begin(), list.end(), all_day_at_top ? CICalendarComponentExpanded::sort_by_dtstart_allday : CICalendarComponentExpanded::sort_by_dtstart);
}

void CICalendar::GetVToDos(bool only_due, bool all_dates, const CICalendarDateTime& upto_due_date, CICalendarExpandedComponents& list) const
{
	// Get current date-time less one day to test for completed events during the last day
	CICalendarDateTime minusoneday;
	minusoneday.SetNowUTC();
	minusoneday.OffsetDay(-1);

	CICalendarDateTime today;
	today.SetToday();

	// Look at each VToDo
	for(CICalendarComponentDB::const_iterator iter = mVToDo.begin(); iter != mVToDo.end(); iter++)
	{
		CICalendarVToDo* vtodo = static_cast<CICalendarVToDo*>((*iter).second);
        iCal::CICalendarVToDo::ECompletedStatus status = vtodo->GetCompletionState();
		
		// Filter out done (that were complted more than a day ago) or cancelled to dos if required
		if (only_due)
		{
			if (status == iCal::CICalendarVToDo::eCancelled)
				continue;
            else if ((status == iCal::CICalendarVToDo::eDone) &&
                     (!vtodo->HasCompleted() || (vtodo->GetCompleted() < minusoneday)))
				continue;
		}

		// Filter out those with end after chosen date if required
		if (!all_dates)
		{
			if (vtodo->HasEnd() && (vtodo->GetEnd() > upto_due_date))
				continue;
			else if (!vtodo->HasEnd() && (today > upto_due_date))
				continue;
		}

		list.push_back(CICalendarComponentExpandedShared(new CICalendarComponentExpanded(vtodo, NULL)));
	}
}

// Get list of recurrence instance components matching the UID
void CICalendar::GetRecurrenceInstances(CICalendarComponent::EComponentType type, const cdstring& uid, CICalendarComponentRecurs& items) const
{
	// Get instances from list
	GetComponents(type).GetRecurrenceInstances(uid, items);
}

// Get list of recurrence instances matching the UID
void CICalendar::GetRecurrenceInstances(CICalendarComponent::EComponentType type, const cdstring& uid, CICalendarDateTimeList& ids) const
{
	// Get instances from list
	GetComponents(type).GetRecurrenceInstances(uid, ids);
}

void CICalendar::GetVFreeBusy(const CICalendarPeriod& period, CICalendarComponentList& list) const
{
	// Look at each VFreeBusy
	for(CICalendarComponentDB::const_iterator iter = mVFreeBusy.begin(); iter != mVFreeBusy.end(); iter++)
	{
		CICalendarVFreeBusy* vfreebusy = static_cast<CICalendarVFreeBusy*>((*iter).second);
		vfreebusy->ExpandPeriod(period, list);
	}
}

// Freebusy generation
void CICalendar::GetVFreeBusy(const CICalendarPeriod& period, CICalendarComponent& fb) const
{
	// First create expanded set
	CICalendarExpandedComponents list;
	GetVEvents(period, list);
	if (list.size() == 0)
		return;
	
	// Get start/end list for each non-all-day expanded components
	CICalendarDateTimeList dtstart;
	CICalendarDateTimeList dtend;
	for(CICalendarExpandedComponents::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		// Ignore if all-day
		if ((*iter)->GetInstanceStart().IsDateOnly())
			continue;
		
		// Ignore if transparent to free-busy
		cdstring transp;
		if ((*iter)->GetOwner()->GetProperty(cICalProperty_TRANSP, transp) && (transp == cICalProperty_TRANSPARENT))
			continue;
		
		// Add start/end to list
		dtstart.push_back((*iter)->GetInstanceStart());
		dtend.push_back((*iter)->GetInstanceEnd());
	}
	
	// No longer need the expanded items
	list.clear();

	// Create non-overlapping periods as properties in the freebusy component
	CICalendarPeriod temp(dtstart.front(), dtend.front());
	CICalendarDateTimeList::const_iterator dtstart_iter = dtstart.begin() + 1;
	CICalendarDateTimeList::const_iterator dtend_iter = dtend.begin() + 1;
	for(; dtstart_iter != dtstart.end(); dtstart_iter++, dtend_iter++)
	{
		// Check for non-overlap
		if (*dtstart_iter > temp.GetEnd())
		{
			// Current period is complete
			fb.AddProperty(CICalendarProperty(cICalProperty_FREEBUSY, temp));
			
			// Reset period to new range
			temp = CICalendarPeriod(*dtstart_iter, *dtend_iter);
		}
		
		// They overlap - check for extended end
		if (*dtend_iter > temp.GetEnd())
		{
			// Extend the end
			temp = CICalendarPeriod(temp.GetStart(), *dtend_iter);
		}
	}
	
	// Add remaining period as property
	fb.AddProperty(CICalendarProperty(cICalProperty_FREEBUSY, temp));
}
	
// Freebusy generation
void CICalendar::GetFreeBusy(const CICalendarPeriod& period, CICalendarFreeBusyList& fb) const
{
	// First create expanded set
	{
		CICalendarExpandedComponents list;
		GetVEvents(period, list);
		
		// Get start/end list for each non-all-day expanded components
		for(CICalendarExpandedComponents::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			// Ignore if all-day
			if ((*iter)->GetInstanceStart().IsDateOnly())
				continue;
			
			// Ignore if transparent to free-busy
			cdstring transp;
			if ((*iter)->GetOwner()->GetProperty(cICalProperty_TRANSP, transp) && (transp == cICalProperty_TRANSPARENT))
				continue;
			
			// Add free busy item to list
			switch((*iter)->GetMaster<CICalendarVEvent>()->GetStatus())
			{
			case eStatus_VEvent_None:
			case eStatus_VEvent_Confirmed:
				fb.push_back(CICalendarFreeBusy(CICalendarFreeBusy::eBusy, CICalendarPeriod((*iter)->GetInstanceStart(), (*iter)->GetInstanceEnd())));
				break;
			case eStatus_VEvent_Tentative:
				fb.push_back(CICalendarFreeBusy(CICalendarFreeBusy::eBusyTentative, CICalendarPeriod((*iter)->GetInstanceStart(), (*iter)->GetInstanceEnd())));
				break;
			case eStatus_VEvent_Cancelled:
				// Cancelled => does not contribute to busy time
				break;
			}
		}
	}

	// Now get the VFREEBUSY info
	{
		CICalendarComponentList list2;
		GetVFreeBusy(period, list2);
		
		// Get start/end list for each free-busy
		for(CICalendarComponentList::const_iterator iter1 = list2.begin(); iter1 != list2.end(); iter1++)
		{
			// Expand component and add free busy info to list
			static_cast<CICalendarVFreeBusy*>(*iter1)->ExpandPeriod(period, fb);
		}
	}
		
	// Add remaining period as property
	CICalendarFreeBusy::ResolveOverlaps(fb);
}
	
// Freebusy generation from VFREEBUSY only
void CICalendar::GetFreeBusyOnly(CICalendarFreeBusyList& fb) const
{
	// Now get the VFREEBUSY info
	{
		// Get start/end list for each free-busy
		for(CICalendarComponentDB::const_iterator iter = mVFreeBusy.begin(); iter != mVFreeBusy.end(); iter++)
		{
			static_cast<CICalendarVFreeBusy*>((*iter).second)->GetPeriod(fb);
		}
	}
	
	// Add remaining period as property
	CICalendarFreeBusy::ResolveOverlaps(fb);
}

// Merge timezones
void CICalendar::MergeTimezones(const CICalendar& cal)
{
	// Merge each timezone from other calendar
	for(CICalendarComponentDB::const_iterator iter = cal.mVTimezone.begin(); iter != cal.mVTimezone.end(); iter++)
	{
		// See whether matching item is already installed
		CICalendarComponentDB::iterator found = mVTimezone.find((*iter).second->GetMapKey());
		if (found == mVTimezone.end())
		{
			// Item does not already exist - so copy and add it
			CICalendarVTimezone* copy = new CICalendarVTimezone(*static_cast<CICalendarVTimezone*>((*iter).second));
			mVTimezone.AddComponent(copy);
		}
		else
			// Merge similar items
			static_cast<CICalendarVTimezone*>((*found).second)->MergeTimezone(*static_cast<const CICalendarVTimezone*>((*iter).second));
	}
}

// Timezone lookups
int32_t CICalendar::GetTimezoneOffsetSeconds(const cdstring& timezone, const CICalendarDateTime& dt)
{
	// Find timezone that matches the name (which is the same as the map key)
	CICalendarComponentDB::const_iterator found = mVTimezone.find(timezone);
	if (found != mVTimezone.end())
	{
		return static_cast<CICalendarVTimezone*>((*found).second)->GetTimezoneOffsetSeconds(dt);
	}
	else
		return 0;
}

// Timezone lookups
cdstring CICalendar::GetTimezoneDescriptor(const cdstring& timezone, const CICalendarDateTime& dt)
{
	// Find timezone that matches the name (which is the same as the map key)
	CICalendarComponentDB::const_iterator found = mVTimezone.find(timezone);
	if (found != mVTimezone.end())
	{
		return static_cast<CICalendarVTimezone*>((*found).second)->GetTimezoneDescriptor(dt);
	}
	else
		return cdstring::null_str;
}

void CICalendar::GetTimezones(cdstrvect& tzids) const
{
	// Get all timezones in a list for sorting
	typedef std::multimap<int32_t, CICalendarComponent*> CSortedComponentMap;
	CSortedComponentMap sorted;
	for(CICalendarComponentDB::const_iterator iter = mVTimezone.begin(); iter != mVTimezone.end(); iter++)
	{
		sorted.insert(CSortedComponentMap::value_type(static_cast<CICalendarVTimezone*>((*iter).second)->GetSortKey(), (*iter).second));
	}
	
	// Now add to list in sorted order
	for(CSortedComponentMap::const_iterator iter = sorted.begin(); iter != sorted.end(); iter++)
	{
		tzids.push_back(static_cast<CICalendarVTimezone*>((*iter).second)->GetID());
	}
}

void CICalendar::SortTimezones(cdstrvect& tzids) const
{
	// Get all timezones in a list for sorting
	typedef std::multimap<int32_t, const CICalendarComponent*> CSortedComponentMap;
	CSortedComponentMap sorted;
	for(cdstrvect::const_iterator iter = tzids.begin(); iter != tzids.end(); iter++)
	{
		const CICalendarVTimezone* tz = GetTimezone(*iter);
		if (tz != NULL)
			sorted.insert(CSortedComponentMap::value_type(tz->GetSortKey(), tz));
	}
	
	// Now add to list in sorted order
	tzids.clear();
	for(CSortedComponentMap::const_iterator iter = sorted.begin(); iter != sorted.end(); iter++)
	{
		tzids.push_back(static_cast<const CICalendarVTimezone*>((*iter).second)->GetID());
	}
}

const CICalendarVTimezone* CICalendar::GetTimezone(const cdstring& tzid) const
{
	// Find timezone that matches the name (which is the same as the map key)
	CICalendarComponentDB::const_iterator found = mVTimezone.find(tzid);
	if (found != mVTimezone.end())
	{
		return static_cast<CICalendarVTimezone*>((*found).second);
	}
	else
		return NULL;
}

void CICalendar::IncludeTimezones()
{
	// Get timezone names from each component
	cdstrset tzids;
	IncludeTimezones(mVEvent, tzids);
	IncludeTimezones(mVToDo, tzids);
	IncludeTimezones(mVJournal, tzids);
	IncludeTimezones(mVFreeBusy, tzids);

	// Make sure each timezone is in current calendar
	for(cdstrset::const_iterator iter = tzids.begin(); iter != tzids.end(); iter++)
	{
		const CICalendarVTimezone* tz = GetTimezone(*iter);
		if (tz == NULL)
		{
			// Find it in the static object
			tz = getSICalendar().GetTimezone(*iter);
			if (tz != NULL)
			{
				CICalendarVTimezone* dup = new CICalendarVTimezone(*tz);
				mVTimezone.AddComponent(dup);
			}
		}
	}
}

void CICalendar::IncludeTimezones(const CICalendarComponentDB& components, cdstrset& tzids)
{
	for(CICalendarComponentDB::const_iterator iter = components.begin(); iter != components.end(); iter++)
		(*iter).second->GetTimezones(tzids);
}

bool CICalendar::ValidEDST(cdstrvect& tzids) const
{
	bool result = true;
	for(CICalendarComponentDB::const_iterator iter = mVTimezone.begin(); iter != mVTimezone.end(); iter++)
	{
		if (!static_cast<const CICalendarVTimezone*>((*iter).second)->ValidEDST())
		{
			tzids.push_back(static_cast<const CICalendarVTimezone*>((*iter).second)->GetID());
			result = false;
		}
	}
	
	return result;
}

void CICalendar::UpgradeEDST()
{
	for(CICalendarComponentDB::iterator iter = mVTimezone.begin(); iter != mVTimezone.end(); iter++)
	{
		static_cast<CICalendarVTimezone*>((*iter).second)->UpgradeEDST();
	}
}

void CICalendar::ChangedComponent(CICalendarComponent* comp)
{
	// Calendar has changed
	SetDirty();
	
	// Record change
	CICalendarComponentRecord::RecordAction(mRecordDB, comp, CICalendarComponentRecord::eChanged);
	
	// Broadcast change
	CComponentAction action(CComponentAction::eChanged, *this, *comp);
	Broadcast_Message(eBroadcast_ChangedComponent, &action);
}

void CICalendar::AddNewVEvent(CICalendarVEvent* vevent, bool moved)
{
	// Do not init props if moving
	if (!moved)
	{
		// Make sure UID is set and unique
		cdstring uid;
		vevent->SetUID(uid);
		
		// Init DTSTAMP to now
		vevent->InitDTSTAMP();
	}

	mVEvent.AddComponent(vevent);

	// Calendar has changed
	SetDirty();
	
	// Record change
	CICalendarComponentRecord::RecordAction(mRecordDB, vevent, CICalendarComponentRecord::eAdded);
	
	// Broadcast change
	CComponentAction action(CComponentAction::eAdded, *this, *vevent);
	Broadcast_Message(eBroadcast_AddedComponent, &action);
}

void CICalendar::RemoveVEvent(CICalendarVEvent* vevent, bool delete_it)
{
	// Record change  before delete occurs
	CICalendarComponentRecord::RecordAction(mRecordDB, vevent, CICalendarComponentRecord::eRemoved);

	// Remove from the map (do not delete here - wait until after broadcast)
	mVEvent.RemoveComponent(vevent, false);
	
	// Calendar has changed
	SetDirty();
	
	// Broadcast change
	CComponentAction action(CComponentAction::eRemoved, *this, *vevent);
	Broadcast_Message(eBroadcast_RemovedComponent, &action);
	
	// Delete it here after all changes
	if (delete_it)
		delete vevent;
}

void CICalendar::RemoveVEvent(const cdstring& uid, bool delete_it)
{
	// Get all matching events
	CICalendarComponentRecurs items;
	mVEvent.GetRecurrenceInstances(uid, items);
	
	for(CICalendarComponentRecurs::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		RemoveVEvent(static_cast<CICalendarVEvent*>(*iter), delete_it);
	}
}

void CICalendar::RemoveRecurringVEvent(CICalendarComponentExpandedShared& vevent, ERemoveRecurring recur)
{
	// Determine how to delete
	switch(recur)
	{
	case eRemoveAll:
		// Remove the top-level master event
		RemoveVEvent(vevent->GetTrueMaster<CICalendarVEvent>());
		break;
	case eRemoveOnlyThis:
		{
			// Simply exclude this instance from the top-level master vevent -
			// this works even if this instance is the top-level (first) one
			CICalendarVEvent* master = vevent->GetTrueMaster<CICalendarVEvent>();

			// NB the vevent will be deleted as part of this so cache the instance start before
			CICalendarDateTime exclude(vevent->GetInstanceStart());

			// The start instance is the RECURRENCE-ID to exclude
			master->ExcludeRecurrence(exclude);

			// Tell it it has changed (i.e. bump sequence)
			master->Changed();
			break;
		}
	case eRemoveThisFuture:
		{
			// Simply exclude this instance from the master vevent
			CICalendarVEvent* master = vevent->GetTrueMaster<CICalendarVEvent>();

			// NB the vevent will be deleted as part of this so cache the instance start before
			CICalendarDateTime exclude(vevent->GetInstanceStart());

			// The DTSTART specifies the recurrence that we exclude
			master->ExcludeFutureRecurrence(exclude);

			// Tell it it has changed (i.e. bump sequence)
			master->Changed();
		}
		break;
	}
	
#if 0
	// Calendar has changed
	SetDirty();
	
	// Broadcast change
	Broadcast_Message(eBroadcast_Edit, this);
#endif
}

void CICalendar::AddNewVToDo(CICalendarVToDo* vtodo, bool moved)
{
	// Do not init props if moving
	if (!moved)
	{
		// Make sure UID is set and unique
		cdstring uid;
		vtodo->SetUID(uid);
		
		// Init DTSTAMP to now
		vtodo->InitDTSTAMP();
	}

	mVToDo.AddComponent(vtodo);
	
	// Calendar has changed
	SetDirty();
	
	// Record change
	CICalendarComponentRecord::RecordAction(mRecordDB, vtodo, CICalendarComponentRecord::eAdded);
	
	// Broadcast change
	CComponentAction action(CComponentAction::eAdded, *this, *vtodo);
	Broadcast_Message(eBroadcast_AddedComponent, &action);
}

void CICalendar::RemoveVToDo(CICalendarVToDo* vtodo, bool delete_it)
{
	// Record change  before delete occurs
	CICalendarComponentRecord::RecordAction(mRecordDB, vtodo, CICalendarComponentRecord::eRemoved);

	// Remove from the map (do not delete here - wait until after broadcast)
	mVToDo.RemoveComponent(vtodo, false);
	
	// Calendar has changed
	SetDirty();
	
	// Broadcast change
	CComponentAction action(CComponentAction::eRemoved, *this, *vtodo);
	Broadcast_Message(eBroadcast_RemovedComponent, &action);
	
	// Delete it here after all changes
	if (delete_it)
		delete vtodo;
}

CICalendarComponent* CICalendar::FindComponent(const CICalendarComponent* orig, EFindComponent find) const
{
	// Based on original component type. If we have a component of one type with the same UID
	// as a component of another type something is really f*cked up!
	switch(orig->GetType())
	{
	case CICalendarComponent::eVEVENT:
		return FindComponent(mVEvent, orig, find);
	case CICalendarComponent::eVTODO:
		return FindComponent(mVToDo, orig, find);
	case CICalendarComponent::eVJOURNAL:
		return FindComponent(mVJournal, orig, find);
	case CICalendarComponent::eVFREEBUSY:
		return FindComponent(mVFreeBusy, orig, find);
	case CICalendarComponent::eVTIMEZONE:
		return FindComponent(mVTimezone, orig, find);
	default:
		return NULL;
	}
}

CICalendarComponent* CICalendar::FindComponent(const CICalendarComponentDB& db, const CICalendarComponent* orig, EFindComponent find) const
{
	CICalendarComponentDB::const_iterator found = db.find((find == eFindExact) ? orig->GetMapKey() : orig->GetMasterKey());
	if (found != db.end())
		return (*found).second;
	else
		return NULL;
}

void CICalendar::AddComponent(CICalendarComponent* comp)
{
	// Based on original component type. If we have a component of one type with the same UID
	// as a component of another type something is really f*cked up!
	switch(comp->GetType())
	{
	case CICalendarComponent::eVEVENT:
		AddComponent(mVEvent, comp);
		break;
	case CICalendarComponent::eVTODO:
		AddComponent(mVToDo, comp);
		break;
	case CICalendarComponent::eVJOURNAL:
		AddComponent(mVJournal, comp);
		break;
	case CICalendarComponent::eVFREEBUSY:
		AddComponent(mVFreeBusy, comp);
		break;
	case CICalendarComponent::eVTIMEZONE:
		AddComponent(mVTimezone, comp);
		break;
	default:
		delete comp;
		break;
	}
}

void CICalendar::AddComponent(CICalendarComponentDB& db, CICalendarComponent* comp)
{
	// Just add it without doing anything as this is a copy being made during sync'ing
	if (!db.AddComponent(comp))
		delete comp;
}

const CICalendarComponent* CICalendar::GetComponentByKey(const cdstring& mapkey) const
{
	const CICalendarComponent* result = NULL;

	result = GetComponentByKey(mVEvent, mapkey);
	if (result != NULL)
		return result;

	result = GetComponentByKey(mVToDo, mapkey);
	if (result != NULL)
		return result;

	result = GetComponentByKey(mVJournal, mapkey);
	if (result != NULL)
		return result;
	
	result = GetComponentByKey(mVFreeBusy, mapkey);
	if (result != NULL)
		return result;
	
	result = GetComponentByKey(mVTimezone, mapkey);
	if (result != NULL)
		return result;
	
	return result;
}

CICalendarComponent* CICalendar::GetComponentByKey(const cdstring& mapkey)
{
	CICalendarComponent* result = NULL;

	result = GetComponentByKey(mVEvent, mapkey);
	if (result != NULL)
		return result;

	result = GetComponentByKey(mVToDo, mapkey);
	if (result != NULL)
		return result;

	result = GetComponentByKey(mVJournal, mapkey);
	if (result != NULL)
		return result;
	
	result = GetComponentByKey(mVFreeBusy, mapkey);
	if (result != NULL)
		return result;
	
	result = GetComponentByKey(mVTimezone, mapkey);
	if (result != NULL)
		return result;
	
	return result;
}

const CICalendarComponent* CICalendar::GetComponentByKey(const CICalendarComponentDB& db, const cdstring& mapkey) const
{
	CICalendarComponentDB::const_iterator found = db.find(mapkey);
	if (found != db.end())
		return (*found).second;
	else
		return NULL;
}

CICalendarComponent* CICalendar::GetComponentByKey(CICalendarComponentDB& db, const cdstring& mapkey)
{
	CICalendarComponentDB::const_iterator found = db.find(mapkey);
	if (found != db.end())
		return (*found).second;
	else
		return NULL;
}

void CICalendar::RemoveComponentByKey(const cdstring& mapkey)
{
	if (RemoveComponentByKey(mVEvent, mapkey))
		return;

	if (RemoveComponentByKey(mVToDo, mapkey))
		return;

	if (RemoveComponentByKey(mVJournal, mapkey))
		return;

	if (RemoveComponentByKey(mVFreeBusy, mapkey))
		return;

	if (RemoveComponentByKey(mVTimezone, mapkey))
		return;
}

bool CICalendar::RemoveComponentByKey(CICalendarComponentDB& db, const cdstring& mapkey)
{
	CICalendarComponent* result = GetComponentByKey(db, mapkey);
	if (result != NULL)
	{
		db.RemoveComponent(result, true);
		return true;
	}
	else
		return false;
}

#pragma mark ____________________________Disconnected

// XML DTD
/*
	<!ELEMENT calendarstate	(etag, sync-token, recordlist) >
	<!ATTLIST calendarstate	version			CDATA	#REQUIRED >
	
	<!ELEMENT etag			(#PCDATA) >
	<!ELEMENT sync-token	(#PCDATA) >
	
	<!ELEMENT recordlist	(record*) >
	
	<!ELEMENT record		(uid, seq, rid) >
	<!ATTLIST action		(added|changed|removed|removedadded) #REQUIRED
			  mapkey		CDATA #REQUIRED >

	<!ELEMENT uid			(#PCDATA) >
	<!ELEMENT seq			(#PCDATA) >
	<!ELEMENT rid			(#PCDATA) >
	
*/

static const char* cXMLElement_calendarstate	= "calendarstate";
static const char* cXMLAttribute_version		= "version";

static const char* cXMLElement_etag				= "etag";
static const char* cXMLElement_sync_token		= "sync-token";

static const char* cXMLElement_recordlist		= "recordlist";

#if 0
static const char* cXMLElement_record			= "record";
static const char* cXMLAttribute_action		= "action";
static const char* cXMLAttribute_action_added			= "added";
static const char* cXMLAttribute_action_changed		= "changed";
static const char* cXMLAttribute_action_removed		= "removed";
static const char* cXMLAttribute_action_removedadded	= "removedadded";
static const char* cXMLAttribute_mapkey		= "mapkey";

static const char* cXMLElement_uid				= "uid";

static const char* cXMLElement_seq				= "seq";

static const char* cXMLElement_rid				= "rid";
#endif

void CICalendar::ClearSync()
{
    
	// Remove recording of old data - but leave the etag as-is
	ClearRecording();
    
	// Remove existing calendar properties then add back the defaults
	mProperties.clear();
	AddDefaultProperties();
    
	// Clean up the map items
	mVEvent.RemoveAllComponents();
	mVToDo.RemoveAllComponents();
	mVJournal.RemoveAllComponents();
	mVFreeBusy.RemoveAllComponents();
	mVTimezone.RemoveAllComponents();    
}

void CICalendar::ParseCache(std::istream& is)
{
	// Init the cached data first
	mETag = cdstring::null_str;
	mSyncToken = cdstring::null_str;
	mRecordDB.clear();

	// XML parse the data
	xmllib::XMLSAXSimple parser;
	parser.ParseStream(is);

	// See if we got any valid XML
	if (parser.Document())
	{
		// Check root node
		xmllib::XMLNode* root = parser.Document()->GetRoot();
		if (root->Name() != cXMLElement_calendarstate)
			return;
		
		// Get ETag
		const xmllib::XMLNode* etagnode = root->GetChild(cXMLElement_etag);
		if (etagnode == NULL)
			return;
		etagnode->DataValue(mETag);
		
		// Get sync-token
		const xmllib::XMLNode* synctokennode = root->GetChild(cXMLElement_sync_token);
		if (synctokennode != NULL)
			synctokennode->DataValue(mSyncToken);
		
		// Get recordlist node
		const xmllib::XMLNode* recordlistnode = root->GetChild(cXMLElement_recordlist);
		if (recordlistnode != NULL)
		{
			// Now look at each child
			for(xmllib::XMLNodeList::const_iterator iter = recordlistnode->Children().begin(); iter != recordlistnode->Children().end(); iter++)
			{
				CICalendarComponentRecord::ReadXML(mRecordDB, *iter);
			}
		}
	}
}

void CICalendar::GenerateCache(std::ostream& os) const
{
	// Create XML document object
	std::auto_ptr<xmllib::XMLDocument> doc(new xmllib::XMLDocument);
	
	// Root element is the preferences element
	doc->GetRoot()->SetName(cXMLElement_calendarstate);
	doc->GetRoot()->AddAttribute(cXMLAttribute_version, "1");
	
	// Create etag child node
	new xmllib::XMLNode(doc.get(), doc->GetRoot(), cXMLElement_etag, GetETag());
	
	// Create sync-token child node
	new xmllib::XMLNode(doc.get(), doc->GetRoot(), cXMLElement_sync_token, GetSyncToken());
	
	// Create recordlist child node
	xmllib::XMLNode* recordlistnode = new xmllib::XMLNode(doc.get(), doc->GetRoot(), cXMLElement_recordlist);
	
	// Now add each node recorded item
	for(CICalendarComponentRecordDB::const_iterator iter = mRecordDB.begin(); iter != mRecordDB.end(); iter++)
	{
		(*iter).second.WriteXML(doc.get(), recordlistnode, (*iter).first);
	}
	
	// Write to stream
	doc->Generate(os);
}
