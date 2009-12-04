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
	CICalendarVEvent.cpp

	Author:
	Description:	<describe the CICalendarVEvent class here>
*/

#include "CICalendarVEvent.h"

using namespace iCal;

cdstring CICalendarVEvent::sBeginDelimiter(cICalComponent_BEGINVEVENT);
cdstring CICalendarVEvent::sEndDelimiter(cICalComponent_ENDVEVENT);

CICalendarComponent* CICalendarVEvent::Create(const CICalendarRef& calendar)
{
	return new CICalendarVEvent(calendar);
}

bool CICalendarVEvent::AddComponent(CICalendarComponent* comp)
{
	// We can embed the alarm components only
	if (comp->GetType() == eVALARM)
	{
		if (mEmbedded == NULL)
			mEmbedded = new CICalendarComponentList;
		mEmbedded->push_back(comp);
		mEmbedded->back()->SetEmbedder(this);
		return true;
	}
	else
		return false;
}

void CICalendarVEvent::Finalise()
{
	// Do inherited
	CICalendarComponentRecur::Finalise();

	// Get STATUS
	{
		cdstring temp;
		if (LoadValue(cICalProperty_STATUS, temp))
		{
			if (temp == cICalProperty_STATUS_TENTATIVE)
			{
				mStatus = eStatus_VEvent_Tentative;
			}
			else if (temp == cICalProperty_STATUS_CONFIRMED)
			{
				mStatus = eStatus_VEvent_Confirmed;
			}
			else if (temp == cICalProperty_STATUS_CANCELLED)
			{
				mStatus = eStatus_VEvent_Cancelled;
			}
		}
	}
}

void CICalendarVEvent::EditStatus(EStatus_VEvent status)
{
	// Only if it is different
	if (mStatus != status)
	{
		// Updated cached values
		mStatus = status;

		// Remove existing STATUS items
		RemoveProperties(cICalProperty_STATUS);

		// Now create properties
		{
			const char* value = NULL;
			switch(status)
			{
			case eStatus_VEvent_None:
			default:
				break;
			case eStatus_VEvent_Tentative:
				value = cICalProperty_STATUS_TENTATIVE;
				break;
			case eStatus_VEvent_Confirmed:
				value = cICalProperty_STATUS_CONFIRMED;
				break;
			case eStatus_VEvent_Cancelled:
				value = cICalProperty_STATUS_CANCELLED;
				break;
			}
			if (value != NULL)
			{
				CICalendarProperty prop(cICalProperty_STATUS, value);
				AddProperty(prop);
			}
		}
	}
}
