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
	CICalendarVAlarm.h

	Author:
	Description:	<describe the CICalendarVAlarm class here>
*/

#ifndef CICalendarVAlarm_H
#define CICalendarVAlarm_H

#include "CICalendarComponent.h"
#include "CICalendarDateTime.h"
#include "CICalendarDefinitions.h"
#include "CICalendarDuration.h"

namespace iCal {

class CICalendarVAlarm: public CICalendarComponent
{
public:
	// Classes for each action encapsulating action-specific data
	class CICalendarVAlarmAction
	{
	public:
		CICalendarVAlarmAction(EAction_VAlarm type)
		{
			mType = type;
		}
		CICalendarVAlarmAction(const CICalendarVAlarmAction& copy)
		{
			mType = copy.mType;
		}
		virtual ~CICalendarVAlarmAction()
		{
		}
		
		virtual CICalendarVAlarmAction* clone() const = 0;

		virtual void Load(CICalendarVAlarm* valarm) = 0;
		virtual void Add(CICalendarVAlarm* valarm) = 0;
		virtual void Remove(CICalendarVAlarm* valarm) = 0;

		EAction_VAlarm GetType() const
		{
			return mType;
		}

	protected:
		EAction_VAlarm	mType;
	};

	class CICalendarVAlarmAudio : public CICalendarVAlarmAction
	{
	public:
		CICalendarVAlarmAudio() :
			CICalendarVAlarmAction(eAction_VAlarm_Audio)
		{
		}
		CICalendarVAlarmAudio(const cdstring& speak) :
			CICalendarVAlarmAction(eAction_VAlarm_Audio)
		{
			mSpeakText = speak;
		}
		CICalendarVAlarmAudio(const CICalendarVAlarmAudio& copy) :
			CICalendarVAlarmAction(copy)
		{
			mSpeakText = copy.mSpeakText;
		}
		virtual ~CICalendarVAlarmAudio()
		{
		}
		
		virtual CICalendarVAlarmAction* clone() const
		{
			return new CICalendarVAlarmAudio(*this);
		}

		virtual void Load(CICalendarVAlarm* valarm);
		virtual void Add(CICalendarVAlarm* valarm);
		virtual void Remove(CICalendarVAlarm* valarm);

		bool IsSpeakText() const
		{
			return !mSpeakText.empty();
		}
		const cdstring& GetSpeakText() const
		{
			return mSpeakText;
		}

	protected:
		cdstring	mSpeakText;
	};

	class CICalendarVAlarmDisplay : public CICalendarVAlarmAction
	{
	public:
		CICalendarVAlarmDisplay() :
			CICalendarVAlarmAction(eAction_VAlarm_Display)
		{
		}
		CICalendarVAlarmDisplay(const cdstring& description) :
			CICalendarVAlarmAction(eAction_VAlarm_Display)
		{
			mDescription = description;
		}
		CICalendarVAlarmDisplay(const CICalendarVAlarmDisplay& copy) :
			CICalendarVAlarmAction(copy)
		{
			mDescription = copy.mDescription;
		}
		virtual ~CICalendarVAlarmDisplay()
		{
		}
		
		virtual CICalendarVAlarmAction* clone() const
		{
			return new CICalendarVAlarmDisplay(*this);
		}

		virtual void Load(CICalendarVAlarm* valarm);
		virtual void Add(CICalendarVAlarm* valarm);
		virtual void Remove(CICalendarVAlarm* valarm);

		const cdstring& GetDescription() const
		{
			return mDescription;
		}

	protected:
		cdstring	mDescription;
	};

	class CICalendarVAlarmEmail : public CICalendarVAlarmAction
	{
	public:
		CICalendarVAlarmEmail() :
			CICalendarVAlarmAction(eAction_VAlarm_Email)
		{
		}
		CICalendarVAlarmEmail(const cdstring& description, const cdstring& summary, const cdstrvect& attendees) :
			CICalendarVAlarmAction(eAction_VAlarm_Email)
		{
			mDescription = description;
			mSummary = summary;
			mAttendees = attendees;
		}
		CICalendarVAlarmEmail(const CICalendarVAlarmEmail& copy) :
			CICalendarVAlarmAction(copy)
		{
			mDescription = copy.mDescription;
			mSummary = copy.mSummary;
			mAttendees = copy.mAttendees;
		}
		virtual ~CICalendarVAlarmEmail()
		{
		}
		
		virtual CICalendarVAlarmAction* clone() const
		{
			return new CICalendarVAlarmEmail(*this);
		}

		virtual void Load(CICalendarVAlarm* valarm);
		virtual void Add(CICalendarVAlarm* valarm);
		virtual void Remove(CICalendarVAlarm* valarm);

		const cdstring& GetDescription() const
		{
			return mDescription;
		}

		const cdstring& GetSummary() const
		{
			return mSummary;
		}

		const cdstrvect& GetAttendees() const
		{
			return mAttendees;
		}

	protected:
		cdstring	mDescription;
		cdstring	mSummary;
		cdstrvect	mAttendees;
	};

	class CICalendarVAlarmUnknown : public CICalendarVAlarmAction
	{
	public:
		CICalendarVAlarmUnknown() :
			CICalendarVAlarmAction(eAction_VAlarm_Unknown)
		{
		}
		CICalendarVAlarmUnknown(const CICalendarVAlarmUnknown& copy) :
			CICalendarVAlarmAction(copy)
		{
		}
		virtual ~CICalendarVAlarmUnknown()
		{
		}

		virtual CICalendarVAlarmAction* clone() const
		{
			return new CICalendarVAlarmUnknown(*this);
		}

		virtual void Load(CICalendarVAlarm* valarm);
		virtual void Add(CICalendarVAlarm* valarm);
		virtual void Remove(CICalendarVAlarm* valarm);

	protected:
	};

	static CICalendarComponent* Create(const CICalendarRef& calendar)
		{ return new CICalendarVAlarm(calendar); }
	static const cdstring& GetVBegin()
		{ return sBeginDelimiter; }
	static const cdstring& GetVEnd()
		{ return sEndDelimiter; }

	CICalendarVAlarm(const CICalendarRef& calendar);
	CICalendarVAlarm(const CICalendarVAlarm& copy) :
		CICalendarComponent(copy)
		{ _copy_CICalendarVAlarm(copy); }
	virtual ~CICalendarVAlarm()
	{
		delete mActionData;
	}

	virtual CICalendarComponent* clone() const
	{
		return new CICalendarVAlarm(*this);
	}

	CICalendarVAlarm& operator=(const CICalendarVAlarm& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarVAlarm(copy);
				CICalendarComponent::operator=(copy);
			}
			return *this;
		}

	virtual EComponentType GetType() const
		{ return eVALARM; }

	virtual const cdstring& GetBeginDelimiter() const
		{ return sBeginDelimiter; }
	virtual const cdstring& GetEndDelimiter() const
		{ return sEndDelimiter; }
	virtual cdstring GetMimeComponentName() const
	{
		// Cannot be sent as a separate MIME object
		return cdstring::null_str;
	}

	EAction_VAlarm GetAction() const
	{
		return mAction;
	}

	const CICalendarVAlarmAction* GetActionData() const
	{
		return mActionData;
	}

	bool IsTriggerAbsolute() const
	{
		return mTriggerAbsolute;
	}

	const CICalendarDateTime& GetTriggerOn() const
		{ return mTriggerOn; }

	const CICalendarDuration& GetTriggerDuration() const
		{ return mTriggerBy; }

	bool IsTriggerOnStart() const
	{
		return mTriggerOnStart;
	}

	int32_t GetRepeats() const
	{
		return mRepeats;
	}
	const CICalendarDuration& GetInterval() const
	{
		return mRepeatInterval;
	}
	
	virtual void Added();
	virtual void Removed();
	virtual void Changed();
	virtual void Finalise();

	void	EditStatus(EAlarm_Status status);
	void	EditAction(EAction_VAlarm action, CICalendarVAlarmAction* data);
	void	EditTrigger(const CICalendarDateTime& dt);
	void	EditTrigger(const CICalendarDuration& duration, bool trigger_start);
	void	EditRepeats(unsigned long repeat, const CICalendarDuration& interval);

	EAlarm_Status	GetAlarmStatus() const
	{
		return mAlarmStatus;
	}
	void	GetNextTrigger(CICalendarDateTime& dt) const
	{
		if (!mStatusInit)
			const_cast<CICalendarVAlarm*>(this)->InitNextTrigger();
		dt = mNextTrigger;
	}
	bool	AlarmTriggered(CICalendarDateTime& dt);

protected:
	static cdstring		sBeginDelimiter;
	static cdstring		sEndDelimiter;

	EAction_VAlarm			mAction;
	CICalendarVAlarmAction*	mActionData;
	bool					mTriggerAbsolute;
	CICalendarDateTime		mTriggerOn;
	CICalendarDuration		mTriggerBy;
	bool					mTriggerOnStart;
	int32_t					mRepeats;
	CICalendarDuration		mRepeatInterval;

	bool					mStatusInit;
	EAlarm_Status			mAlarmStatus;
	CICalendarDateTime		mLastTrigger;
	CICalendarDateTime		mNextTrigger;
	uint32_t				mDoneCount;
	

	void LoadAction();

	void InitNextTrigger();
	void GetFirstTrigger(CICalendarDateTime& dt);

private:
	void	_copy_CICalendarVAlarm(const CICalendarVAlarm& copy);
};

}	// namespace iCal

#endif	// CICalendarVAlarm_H
