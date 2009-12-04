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
	CICalendarVAlarm.cpp

	Author:
	Description:	<describe the CICalendarVAlarm class here>
*/

#include "CICalendarVAlarm.h"

#include "CCalendarNotifier.h"
#include "CICalendarCalAddressValue.h"
#include "CICalendarComponentRecur.h"
#include "CICalendarDefinitions.h"

using namespace iCal;

cdstring CICalendarVAlarm::sBeginDelimiter(cICalComponent_BEGINVALARM);
cdstring CICalendarVAlarm::sEndDelimiter(cICalComponent_ENDVALARM);

CICalendarVAlarm::CICalendarVAlarm(const CICalendarRef& calendar) :
	CICalendarComponent(calendar)
{
	mAction = eAction_VAlarm_Display;
	mTriggerAbsolute = false;
	mTriggerOnStart = true;
	
	// Set duration default to 1 hour
	mTriggerBy.SetDuration(60 * 60);

	// Does not repeat by default	
	mRepeats = 0;
	mRepeatInterval.SetDuration(5 * 60);	// Five minutes

	// Status
	mStatusInit = false;
	mAlarmStatus = eAlarm_Status_Pending;
	mDoneCount = 0;

	// Create action data
	mActionData = new CICalendarVAlarmDisplay(cdstring::null_str);
}

void CICalendarVAlarm::_copy_CICalendarVAlarm(const CICalendarVAlarm& copy)
{
	mAction = copy.mAction;
	mTriggerAbsolute = copy.mTriggerAbsolute;
	mTriggerOn = copy.mTriggerOn;
	mTriggerBy = copy.mTriggerBy;
	mTriggerOnStart = copy.mTriggerOnStart;

	mRepeats = copy.mRepeats;
	mRepeatInterval = copy.mRepeatInterval;
	
	mAlarmStatus = copy.mAlarmStatus;
	mLastTrigger = copy.mLastTrigger;
	mNextTrigger = copy.mNextTrigger;
	mDoneCount = copy.mDoneCount;

	mActionData = copy.mActionData->clone();
}

void CICalendarVAlarm::Added()
{
	// Added to calendar so add to calendar notifier
	calstore::CCalendarNotifier::sCalendarNotifier.AddAlarm(this);
	
	// Do inherited
	CICalendarComponent::Added();
}

void CICalendarVAlarm::Removed()
{
	// Removed from calendar so add to calendar notifier
	calstore::CCalendarNotifier::sCalendarNotifier.RemoveAlarm(this);
	
	// Do inherited
	CICalendarComponent::Removed();
}

void CICalendarVAlarm::Changed()
{
	// Always force recalc of trigger status
	mStatusInit = false;

	// Changed in calendar so change in calendar notifier
	calstore::CCalendarNotifier::sCalendarNotifier.ChangedAlarm(this);
	
	// Do not do inherited as this is always a sub-component and we do not 
	// do top-level component changes
	//CICalendarComponent::Changed();
}

void CICalendarVAlarm::Finalise()
{
	// Do inherited
	CICalendarComponent::Finalise();
	
	// Get the ACTION
	{
		cdstring temp;
		if (LoadValue(cICalProperty_ACTION, temp))
		{
			if (temp == cICalProperty_ACTION_AUDIO)
			{
				mAction = eAction_VAlarm_Audio;
			}
			else if (temp == cICalProperty_ACTION_DISPLAY)
			{
				mAction = eAction_VAlarm_Display;
			}
			else if (temp == cICalProperty_ACTION_EMAIL)
			{
				mAction = eAction_VAlarm_Email;
			}
			else if (temp == cICalProperty_ACTION_PROCEDURE)
			{
				mAction = eAction_VAlarm_Procedure;
			}
			else
			{
				mAction = eAction_VAlarm_Unknown;
			}
			LoadAction();
		}
	}
	
	// Get the trigger
	if (GetProperties().count(cICalProperty_TRIGGER))
	{
		// Determine the type of the value
		if (LoadValue(cICalProperty_TRIGGER, mTriggerOn))
		{
			mTriggerAbsolute = true;
		}
		else if (LoadValue(cICalProperty_TRIGGER, mTriggerBy))
		{
			mTriggerAbsolute = false;
			
			// Get the property
			const CICalendarProperty& prop = (*GetProperties().find(cICalProperty_TRIGGER)).second;

			// Look for RELATED attribute
			if (prop.HasAttribute(cICalAttribute_RELATED))
			{
				const cdstring& temp = prop.GetAttributeValue(cICalAttribute_RELATED);
				if (temp == cICalAttribute_RELATED_START)
				{
					mTriggerOnStart = true;
				}
				else if (temp == cICalAttribute_RELATED_END)
				{
					mTriggerOnStart = false;
				}
			}
			else
				mTriggerOnStart = true;
		}
	}
	
	// Get repeat & interval
	LoadValue(cICalProperty_REPEAT, mRepeats);
	LoadValue(cICalProperty_DURATION, mRepeatInterval);
	
	// Alarm status - private to Mulberry
	cdstring status;
	if (LoadValue(cICalProperty_ALARM_X_ALARMSTATUS, status))
	{
		if (status == cICalProperty_ALARM_X_ALARMSTATUS_PENDING)
			mAlarmStatus = eAlarm_Status_Pending;
		else if (status == cICalProperty_ALARM_X_ALARMSTATUS_COMPLETED)
			mAlarmStatus = eAlarm_Status_Completed;
		else if (status == cICalProperty_ALARM_X_ALARMSTATUS_DISABLED)
			mAlarmStatus = eAlarm_Status_Disabled;
		else
			mAlarmStatus = eAlarm_Status_Pending;
	}

	// Last trigger time - private to Mulberry
	LoadValue(cICalProperty_ALARM_X_LASTTRIGGER, mLastTrigger);
}

void CICalendarVAlarm::LoadAction()
{
	// Delete current one
	delete mActionData;
	mActionData = NULL;
	switch(mAction)
	{
	case eAction_VAlarm_Audio:
		mActionData = new CICalendarVAlarmAudio;
		mActionData->Load(this);
		break;
	case eAction_VAlarm_Display:
		mActionData = new CICalendarVAlarmDisplay;
		mActionData->Load(this);
		break;
	case eAction_VAlarm_Email:
		mActionData = new CICalendarVAlarmEmail;
		mActionData->Load(this);
		break;
	default:
		mActionData = new CICalendarVAlarmUnknown;
		mActionData->Load(this);
		break;
	}
}

void CICalendarVAlarm::EditStatus(EAlarm_Status status)
{
	// Remove existing
	RemoveProperties(cICalProperty_ALARM_X_ALARMSTATUS);
	
	// Updated cached values
	mAlarmStatus = status;

	// Add new
	cdstring status_txt;
	if (mAlarmStatus == eAlarm_Status_Pending)
		status_txt = cICalProperty_ALARM_X_ALARMSTATUS_PENDING;
	else if (mAlarmStatus == eAlarm_Status_Completed)
		status_txt = cICalProperty_ALARM_X_ALARMSTATUS_COMPLETED;
	else if (mAlarmStatus == eAlarm_Status_Disabled)
		status_txt = cICalProperty_ALARM_X_ALARMSTATUS_DISABLED;
	AddProperty(CICalendarProperty(cICalProperty_ALARM_X_ALARMSTATUS, status_txt));
}

void CICalendarVAlarm::EditAction(EAction_VAlarm action, CICalendarVAlarmAction* data)
{
	// Remove existing
	RemoveProperties(cICalProperty_ACTION);
	mActionData->Remove(this);
	delete mActionData;
	mActionData = NULL;

	// Updated cached values
	mAction = action;
	mActionData = data;
	
	// Add new properties to alarm
	cdstring action_txt;
	switch(mAction)
	{
	case eAction_VAlarm_Audio:
		action_txt = cICalProperty_ACTION_AUDIO;
		break;
	case eAction_VAlarm_Display:
		action_txt = cICalProperty_ACTION_DISPLAY;
		break;
	case eAction_VAlarm_Email:
		action_txt = cICalProperty_ACTION_EMAIL;
		break;
	default:
		action_txt = cICalProperty_ACTION_PROCEDURE;
		break;
	}
	CICalendarProperty prop(cICalProperty_ACTION, action_txt);
	AddProperty(prop);

	mActionData->Add(this);
}

void CICalendarVAlarm::EditTrigger(const CICalendarDateTime& dt)
{
	// Remove existing
	RemoveProperties(cICalProperty_TRIGGER);

	// Updated cached values
	mTriggerAbsolute = true;
	mTriggerOn = dt;

	// Add new
	CICalendarProperty prop(cICalProperty_TRIGGER, dt);
	AddProperty(prop);
}

void CICalendarVAlarm::EditTrigger(const CICalendarDuration& duration, bool trigger_start)
{
	// Remove existing
	RemoveProperties(cICalProperty_TRIGGER);

	// Updated cached values
	mTriggerAbsolute = false;
	mTriggerBy = duration;
	mTriggerOnStart = trigger_start;

	// Add new (with attribute)
	CICalendarProperty prop(cICalProperty_TRIGGER, duration);
	CICalendarAttribute attr(cICalAttribute_RELATED, trigger_start ? cICalAttribute_RELATED_START : cICalAttribute_RELATED_END);
	prop.AddAttribute(attr);
	AddProperty(prop);
}

void CICalendarVAlarm::EditRepeats(unsigned long repeat, const CICalendarDuration& interval)
{
	// Remove existing
	RemoveProperties(cICalProperty_REPEAT);
	RemoveProperties(cICalProperty_DURATION);
	
	// Updated cached values
	mRepeats = repeat;
	mRepeatInterval = interval;

	// Add new
	if (mRepeats > 0)
	{
		AddProperty(CICalendarProperty(cICalProperty_REPEAT, repeat));
		AddProperty(CICalendarProperty(cICalProperty_DURATION, interval));
	}
}

// Setup cache of trigger details
void CICalendarVAlarm::InitNextTrigger()
{
	// Do not bother if its completed
	if (mAlarmStatus == eAlarm_Status_Completed)
		return;
	mStatusInit = true;

	// Look for trigger immediately preceeding or equal to utc now
	CICalendarDateTime nowutc = CICalendarDateTime::GetNowUTC();
	
	// Init done counter
	mDoneCount = 0;

	// Determine the first trigger
	CICalendarDateTime trigger;
	GetFirstTrigger(trigger);

	while(mDoneCount < mRepeats)
	{
		// See if next trigger is later than now
		CICalendarDateTime next_trigger = trigger + mRepeatInterval;
		if (next_trigger > nowutc)
			break;
		mDoneCount++;
		trigger = next_trigger;
	}
	
	// Check for completion
	if ((trigger == mLastTrigger) || (nowutc - trigger).GetTotalSeconds() > (24 * 60 * 60))
	{
		if (mDoneCount == mRepeats)
		{
			mAlarmStatus = eAlarm_Status_Completed;
			return;
		}
		else
		{
			trigger = trigger + mRepeatInterval;
			mDoneCount++;
		}
	}
	
	mNextTrigger = trigger;
}

// Get the very first trigger for this alarm
void CICalendarVAlarm::GetFirstTrigger(CICalendarDateTime& dt)
{
	// If absolute trigger, use that
	if (IsTriggerAbsolute())
	{
		// Get the trigger on
		dt = GetTriggerOn();
	}
	else
	{
		// Get the parent embedder class (must be CICalendarComponentRecur type)
		const CICalendarComponentRecur* owner = dynamic_cast<const iCal::CICalendarComponentRecur*>(GetEmbedder());
		if (owner != NULL)
		{
			// Determine time at which alarm will trigger
			iCal::CICalendarDateTime trigger = IsTriggerOnStart() ? owner->GetStart() : owner->GetEnd();
			
			// Offset by duration
			dt = trigger + GetTriggerDuration();
		}
	}
}

// return true and update dt to the next time the alarm should be triggered
// if its repeating
bool CICalendarVAlarm::AlarmTriggered(CICalendarDateTime& dt)
{
	// Remove existing
	RemoveProperties(cICalProperty_ALARM_X_LASTTRIGGER);
	RemoveProperties(cICalProperty_ALARM_X_ALARMSTATUS);
	
	// Updated cached values
	mLastTrigger = dt;

	if (mDoneCount < mRepeats)
	{
		dt = mNextTrigger = mNextTrigger + mRepeatInterval;
		mDoneCount++;
		mAlarmStatus = eAlarm_Status_Pending;
	}
	else
		mAlarmStatus = eAlarm_Status_Completed;

	// Add new
	AddProperty(CICalendarProperty(cICalProperty_ALARM_X_LASTTRIGGER, dt));
	cdstring status;
	if (mAlarmStatus == eAlarm_Status_Pending)
		status = cICalProperty_ALARM_X_ALARMSTATUS_PENDING;
	else if (mAlarmStatus == eAlarm_Status_Completed)
		status = cICalProperty_ALARM_X_ALARMSTATUS_COMPLETED;
	else if (mAlarmStatus == eAlarm_Status_Disabled)
		status = cICalProperty_ALARM_X_ALARMSTATUS_DISABLED;
	AddProperty(CICalendarProperty(cICalProperty_ALARM_X_ALARMSTATUS, status));
	
	// Now update dt to the next alarm time
	return mAlarmStatus == eAlarm_Status_Pending;
}

void CICalendarVAlarm::CICalendarVAlarmAudio::Load(CICalendarVAlarm* valarm)
{
	// Get properties
	valarm->LoadValue(cICalProperty_ACTION_X_SPEAKTEXT, mSpeakText);
}

void CICalendarVAlarm::CICalendarVAlarmAudio::Add(CICalendarVAlarm* valarm)
{
	// Delete existing then add
	Remove(valarm);
	
	CICalendarProperty prop(cICalProperty_ACTION_X_SPEAKTEXT, mSpeakText);
	valarm->AddProperty(prop);
}

void CICalendarVAlarm::CICalendarVAlarmAudio::Remove(CICalendarVAlarm* valarm)
{
	valarm->RemoveProperties(cICalProperty_ACTION_X_SPEAKTEXT);
}

void CICalendarVAlarm::CICalendarVAlarmDisplay::Load(CICalendarVAlarm* valarm)
{
	// Get properties
	valarm->LoadValue(cICalProperty_DESCRIPTION, mDescription);
}

void CICalendarVAlarm::CICalendarVAlarmDisplay::Add(CICalendarVAlarm* valarm)
{
	// Delete existing then add
	Remove(valarm);
	
	CICalendarProperty prop(cICalProperty_DESCRIPTION, mDescription);
	valarm->AddProperty(prop);
}

void CICalendarVAlarm::CICalendarVAlarmDisplay::Remove(CICalendarVAlarm* valarm)
{
	valarm->RemoveProperties(cICalProperty_DESCRIPTION);
}

void CICalendarVAlarm::CICalendarVAlarmEmail::Load(CICalendarVAlarm* valarm)
{
	// Get properties
	valarm->LoadValue(cICalProperty_DESCRIPTION, mDescription);
	valarm->LoadValue(cICalProperty_SUMMARY, mSummary);

	mAttendees.clear();
	if (valarm->HasProperty(cICalProperty_ATTENDEE))
	{
		// Get each attendee
		std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> range = valarm->GetProperties().equal_range(cICalProperty_ATTENDEE);
		for(CICalendarPropertyMap::const_iterator iter = range.first; iter != range.second; iter++)
		{
			// Get the attendee value
			const CICalendarCalAddressValue* attendee = (*iter).second.GetCalAddressValue();
			if (attendee != NULL)
			{
				mAttendees.push_back(attendee->GetValue());
			}
		}
	}
}

void CICalendarVAlarm::CICalendarVAlarmEmail::Add(CICalendarVAlarm* valarm)
{
	// Delete existing then add
	Remove(valarm);
	
	{
		CICalendarProperty prop(cICalProperty_DESCRIPTION, mDescription);
		valarm->AddProperty(prop);
	}
	
	{
		CICalendarProperty prop(cICalProperty_SUMMARY, mSummary);
		valarm->AddProperty(prop);
	}
	
	for(cdstrvect::const_iterator iter = mAttendees.begin(); iter != mAttendees.end(); iter++)
	{
		CICalendarProperty prop(cICalProperty_ATTENDEE, *iter, CICalendarValue::eValueType_CalAddress);
		valarm->AddProperty(prop);
	}
}

void CICalendarVAlarm::CICalendarVAlarmEmail::Remove(CICalendarVAlarm* valarm)
{
	valarm->RemoveProperties(cICalProperty_DESCRIPTION);
	valarm->RemoveProperties(cICalProperty_SUMMARY);
	valarm->RemoveProperties(cICalProperty_ATTENDEE);
}

void CICalendarVAlarm::CICalendarVAlarmUnknown::Load(CICalendarVAlarm* valarm)
{
}

void CICalendarVAlarm::CICalendarVAlarmUnknown::Add(CICalendarVAlarm* valarm)
{
}

void CICalendarVAlarm::CICalendarVAlarmUnknown::Remove(CICalendarVAlarm* valarm)
{
}
