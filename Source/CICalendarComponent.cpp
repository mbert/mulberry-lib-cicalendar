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
	CICalendarComponent.cpp

	Author:
	Description:	<describe the CICalendarComponent class here>
*/

#include "CICalendarComponent.h"

#include "CICalendar.h"
#include "CICalendarDateTimeValue.h"
#include "CICalendarDefinitions.h"

#ifdef __MULBERRY
#include "CTCPSocket.h"
#endif

#include <algorithm>

using namespace iCal;

CICalendarComponent::~CICalendarComponent()
{
	// Delete any embedded components
	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::iterator iter = mEmbedded->begin(); iter !=mEmbedded->end(); iter++)
			delete *iter;
		mEmbedded->clear();
		delete mEmbedded;
		mEmbedded = NULL;
	}
}

void CICalendarComponent::_copy_CICalendarComponent(const CICalendarComponent& copy)
{
	mCalendarRef = copy.mCalendarRef;
	mUID = copy.mUID;
	mSeq = copy.mSeq;
	mOriginalSeq = copy.mOriginalSeq;

	if (copy.mEmbedded != NULL)
	{
		// Do deep copy of element list
		mEmbedded = new CICalendarComponentList;
		for(CICalendarComponentList::const_iterator iter = copy.mEmbedded->begin(); iter != copy.mEmbedded->end(); iter++)
		{
			mEmbedded->push_back((*iter)->clone());
			mEmbedded->back()->SetEmbedder(this);
		}
	}
	
	mETag = copy.mETag;
	mRURL = copy.mRURL;
	mChanged = copy.mChanged;
}

bool CICalendarComponent::AddComponent(CICalendarComponent* comp)
{
	// Sub-classes decide what can be embedded
	return false;
}

void CICalendarComponent::RemoveComponent(CICalendarComponent* comp)
{
	if (mEmbedded != NULL)
	{
		mEmbedded->erase(std::remove(mEmbedded->begin(), mEmbedded->end(), comp), mEmbedded->end());
	}
}

bool CICalendarComponent::HasEmbeddedComponent(EComponentType type) const
{
	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::const_iterator iter = mEmbedded->begin(); iter != mEmbedded->end(); iter++)
		{
			if ((*iter)->GetType() == type)
				return true;
		}
	}
	
	return false;
}

CICalendarComponent* CICalendarComponent::GetFirstEmbeddedComponent(EComponentType type) const
{
	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::const_iterator iter = mEmbedded->begin(); iter != mEmbedded->end(); iter++)
		{
			if ((*iter)->GetType() == type)
				return *iter;
		}
	}
	
	return NULL;
}

void CICalendarComponent::SetUID(const cdstring& uid)
{
	if (uid.length())
		mUID = uid;
	else
	{
		// Get left-side of UID (first 24 chars of MD5 digest of time, clock and ctr)
		static unsigned long ctr = 1;
		cdstring lhs_txt;
		lhs_txt.reserve(256);
		::snprintf(lhs_txt.c_str_mod(), 256, "%lu.%lu.%lu", (time_t) clock(), time(NULL), ctr++);
		cdstring lhs;
		lhs_txt.md5(lhs);
		lhs[(cdstring::size_type)24] = 0;

		// Get right side (domain) of message-id
		cdstring rhs;
#ifdef __MULBERRY
		cdstring host = CTCPSocket::TCPGetLocalHostName();
		host.trimspace();
		if (host.length())
		{
			// Must put IP numbers inside [..]
			if (CTCPSocket::TCPIsHostName(host))
				rhs = host;
			else
			{
				rhs = "[";
				rhs += host;
				rhs += "]";
			}
		}
		else
#endif
		{
			// Use app name
			cdstring domain("mulberry");
			domain += cdstring(ctr);
				
			// Use first 24 chars of MD5 digest of the domain as the right-side of message-id
			domain.md5(rhs);
			rhs[(cdstring::size_type)24] = 0;
		}

		// Generate the UID string
		cdstring uid;
		uid += lhs;
		uid += "@";
		uid += rhs;

		mUID = uid;
	}

	RemoveProperties(cICalProperty_UID);

	CICalendarProperty prop(cICalProperty_UID, mUID);
	AddProperty(prop);
}

void CICalendarComponent::SetSeq(const int32_t& seq)
{
	mSeq = seq;

	RemoveProperties(cICalProperty_SEQUENCE);

	CICalendarProperty prop(cICalProperty_SEQUENCE, mSeq);
	AddProperty(prop);
}

void CICalendarComponent::GenerateRURL()
{
	// Format is:
	//
	// <<hash code>> *("-0"> .ics
	if (mRURL.empty())
	{
		// Generate hash code
		cdstring hash;
		hash += GetMapKey();
		hash += ":";
		hash += cdstring((long)GetSeq());
		hash += ":";

		CICalendarDateTime dt;
		if (LoadValue(cICalProperty_DTSTAMP, dt))
		{
			hash += dt.GetText();
		}
		
		hash.md5(mRURL);
		
		// Truncate at 16 chars
		hash.erase(16);
	}
	else
	{
		// Strip off .ics
		mRURL.erase(mRURL.rfind(".ics", cdstring::npos, 4, true));
	}
	
	// Add trailer
	mRURL += "-0.ics";
}

void CICalendarComponent::InitDTSTAMP()
{
	RemoveProperties(cICalProperty_DTSTAMP);

	CICalendarProperty prop(cICalProperty_DTSTAMP, CICalendarDateTime::GetNowUTC());
	AddProperty(prop);
}

void CICalendarComponent::UpdateLastModified()
{
	RemoveProperties(cICalProperty_LAST_MODIFIED);

	CICalendarProperty prop(cICalProperty_LAST_MODIFIED, CICalendarDateTime::GetNowUTC());
	AddProperty(prop);
}

void CICalendarComponent::Added()
{
	// Also add sub-components if present
	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::iterator iter = mEmbedded->begin(); iter !=mEmbedded->end(); iter++)
			(*iter)->Added();
	}
	
	mChanged = true;
}

void CICalendarComponent::Removed()
{
	// Also remove sub-components
	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::iterator iter = mEmbedded->begin(); iter !=mEmbedded->end(); iter++)
			(*iter)->Removed();
	}
	
	mChanged = true;
}

void CICalendarComponent::Duplicated()
{
	// Remove SEQ, UID, DTSTAMP
	// These will be re-created when it is added to the calendar
	RemoveProperties(cICalProperty_UID);
	RemoveProperties(cICalProperty_SEQUENCE);
	RemoveProperties(cICalProperty_DTSTAMP);

	// Remove the cached values as well
	mUID.clear();
	mSeq = 0;
	mOriginalSeq = 0;
	
	// Also duplicate sub-components
	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::iterator iter = mEmbedded->begin(); iter !=mEmbedded->end(); iter++)
			(*iter)->Duplicated();
	}
	
	// Reset CalDAV items
	mETag = cdstring::null_str;
	mRURL = cdstring::null_str;
	mChanged = true;
}

void CICalendarComponent::Changed()
{
	// Bump the sequence
	SetSeq(GetSeq() + 1);

	// Update last-modified
	UpdateLastModified();

	// Also change sub-components
	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::iterator iter = mEmbedded->begin(); iter !=mEmbedded->end(); iter++)
			(*iter)->Changed();
	}

	mChanged = true;

	// Mark calendar as dirty
	CICalendar* cal = CICalendar::GetICalendar(GetCalendar());
	if (cal != NULL)
		cal->ChangedComponent(this);
}

void CICalendarComponent::Finalise()
{
	// Get UID
	LoadValue(cICalProperty_UID, mUID);

	// Get SEQ
	LoadValue(cICalProperty_SEQUENCE, mSeq);
	
	// Cache the original sequence when the component is read in.
	// This will be used to synchronise changes between two instances of the same calendar
	mOriginalSeq = mSeq;
	
	// Get CalDAV info if present
	LoadPrivateValue(cICalProperty_X_PRIVATE_RURL, mRURL);
	LoadPrivateValue(cICalProperty_X_PRIVATE_ETAG, mETag);
}

void CICalendarComponent::GetTimezones(cdstrset& tzids) const
{
	// Look for all date-time properties
	for(CICalendarPropertyMap::const_iterator iter = mProperties.begin(); iter != mProperties.end(); iter++)
	{
		// Try to get a date-time value from the property
		const CICalendarDateTimeValue* dtv = (*iter).second.GetDateTimeValue();
		if ((dtv != NULL) && !dtv->GetValue().IsDateOnly())
		{
			// Add timezone id if appropriate
			if (dtv->GetValue().GetTimezone().HasTZID())
				tzids.insert(dtv->GetValue().GetTimezone().GetTimezoneID());
		}
	}
}

void CICalendarComponent::Generate(std::ostream& os, bool for_cache) const
{
	// Header
	os << GetBeginDelimiter() << net_endl;

	// Write each property
	WriteProperties(os);
	
	// Do private properties if caching
	if (for_cache)
	{
		if (!mRURL.empty())
			WritePrivateProperty(os, cICalProperty_X_PRIVATE_RURL, mRURL);
		if (!mETag.empty())
			WritePrivateProperty(os, cICalProperty_X_PRIVATE_ETAG, mETag);
	}

	// Write each embedded component
	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::const_iterator iter = mEmbedded->begin(); iter != mEmbedded->end(); iter++)
			(*iter)->Generate(os, for_cache);
	}

	// Footer
	os << GetEndDelimiter() << net_endl;
}
