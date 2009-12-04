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
	CICalendarDefinitions.h

	Author:
	Description:	<describe the CICalendarDefinitions class here>
*/

#ifndef CICalendarDefinitions_H
#define CICalendarDefinitions_H

#include <stdint.h>


namespace iCal {

// 2445 Component Header/Footer

extern const char* cICalComponent_BEGINVCALENDAR;
extern const char* cICalComponent_ENDVCALENDAR;
extern const char* cICalComponent_BEGINVEVENT;
extern const char* cICalComponent_ENDVEVENT;
extern const char* cICalComponent_BEGINVTODO;
extern const char* cICalComponent_ENDVTODO;
extern const char* cICalComponent_BEGINVJOURNAL;
extern const char* cICalComponent_ENDVJOURNAL;
extern const char* cICalComponent_BEGINVFREEBUSY;
extern const char* cICalComponent_ENDVFREEBUSY;
extern const char* cICalComponent_BEGINVTIMEZONE;
extern const char* cICalComponent_ENDVTIMEZONE;
extern const char* cICalComponent_BEGINVALARM;
extern const char* cICalComponent_ENDVALARM;

// Pseudo components
extern const char* cICalComponent_BEGINSTANDARD;
extern const char* cICalComponent_ENDSTANDARD;
extern const char* cICalComponent_BEGINDAYLIGHT;
extern const char* cICalComponent_ENDDAYLIGHT;

// 2445 Calendar Property Atrributes

// 2445 4.2
extern const char* cICalAttribute_ALTREP;
extern const char* cICalAttribute_CN;
extern const char* cICalAttribute_CUTYPE;
extern const char* cICalAttribute_DELEGATED_FROM;
extern const char* cICalAttribute_DELEGATED_TO;
extern const char* cICalAttribute_DIR;
extern const char* cICalAttribute_ENCODING;
extern const char* cICalAttribute_FMTTYPE;
extern const char* cICalAttribute_FBTYPE;
extern const char* cICalAttribute_LANGUAGE;
extern const char* cICalAttribute_MEMBER;
extern const char* cICalAttribute_PARTSTAT;
extern const char* cICalAttribute_RANGE;
extern const char* cICalAttribute_RELATED;
extern const char* cICalAttribute_RELTYPE;
extern const char* cICalAttribute_ROLE;
extern const char* cICalAttribute_RSVP;
extern const char* cICalAttribute_RSVP_TRUE;
extern const char* cICalAttribute_RSVP_FALSE;
extern const char* cICalAttribute_SENT_BY;
extern const char* cICalAttribute_TZID;
extern const char* cICalAttribute_VALUE;

// 2445 4.2.9
extern const char* cICalAttribute_FBTYPE_FREE;
extern const char* cICalAttribute_FBTYPE_BUSY;
extern const char* cICalAttribute_FBTYPE_BUSYUNAVAILABLE;
extern const char* cICalAttribute_FBTYPE_BUSYTENTATIVE;

// 2445 4.2.12
enum EPartStat
{
	ePartStat_NeedsAction = 0,
	ePartStat_Accepted,
	ePartStat_Declined,
	ePartStat_Tentative,
	ePartStat_Delegated,
	ePartStat_Completed,
	ePartStat_InProcess
};
extern const char* cICalAttribute_PARTSTAT_NEEDSACTION;
extern const char* cICalAttribute_PARTSTAT_ACCEPTED;
extern const char* cICalAttribute_PARTSTAT_DECLINED;
extern const char* cICalAttribute_PARTSTAT_TENTATIVE;
extern const char* cICalAttribute_PARTSTAT_DELEGATED;
extern const char* cICalAttribute_PARTSTAT_COMPLETED;
extern const char* cICalAttribute_PARTSTAT_INPROCESS;

// 2445 4.2.13
extern const char* cICalAttribute_RANGE_THISANDFUTURE;
extern const char* cICalAttribute_RANGE_THISANDPRIOR;

// 2445 4.2.14
extern const char* cICalAttribute_RELATED_START;
extern const char* cICalAttribute_RELATED_END;

// 2445 4.2.16
enum EPartRole
{
	ePartRole_Chair = 0,
	ePartRole_Required,
	ePartRole_Optional,
	ePartRole_Non
};

extern const char* cICalAttribute_ROLE_CHAIR;
extern const char* cICalAttribute_ROLE_REQ_PART;
extern const char* cICalAttribute_ROLE_OPT_PART;
extern const char* cICalAttribute_ROLE_NON_PART;

// 2445 Value types

// 2445 4.3
extern const char* cICalValue_BINARY;
extern const char* cICalValue_BOOLEAN;
extern const char* cICalValue_CAL_ADDRESS;
extern const char* cICalValue_DATE;
extern const char* cICalValue_DATE_TIME;
extern const char* cICalValue_DURATION;
extern const char* cICalValue_FLOAT;
extern const char* cICalValue_INTEGER;
extern const char* cICalValue_PERIOD;
extern const char* cICalValue_RECUR;
extern const char* cICalValue_TEXT;
extern const char* cICalValue_TIME;
extern const char* cICalValue_URI;
extern const char* cICalValue_UTC_OFFSET;

// 2445 Calendar Properties

// 2445  4.7

extern const char* cICalProperty_CALSCALE;
extern const char* cICalProperty_METHOD;
extern const char* cICalProperty_PRODID;
extern const char* cICalProperty_VERSION;

// Apple Extensions
extern const char* cICalProperty_XWRCALNAME;
extern const char* cICalProperty_XWRCALDESC;

// 2445 Componenty Property names

// 2445 4.8.1
extern const char* cICalProperty_ATTACH;
extern const char* cICalProperty_CATEGORIES;
extern const char* cICalProperty_CLASS;
extern const char* cICalProperty_COMMENT;
extern const char* cICalProperty_DESCRIPTION;
extern const char* cICalProperty_GEO;
extern const char* cICalProperty_LOCATION;
extern const char* cICalProperty_PERCENT_COMPLETE;
extern const char* cICalProperty_PRIORITY;
extern const char* cICalProperty_RESOURCES;
extern const char* cICalProperty_STATUS;
extern const char* cICalProperty_SUMMARY;

// 2445 4.8.2
extern const char* cICalProperty_COMPLETED;
extern const char* cICalProperty_DTEND;
extern const char* cICalProperty_DUE;
extern const char* cICalProperty_DTSTART;
extern const char* cICalProperty_DURATION;
extern const char* cICalProperty_FREEBUSY;
extern const char* cICalProperty_TRANSP;
extern const char* cICalProperty_OPAQUE;
extern const char* cICalProperty_TRANSPARENT;

// 2445 4.8.3
extern const char* cICalProperty_TZID;
extern const char* cICalProperty_TZNAME;
extern const char* cICalProperty_TZOFFSETFROM;
extern const char* cICalProperty_TZOFFSETTO;
extern const char* cICalProperty_TZURL;

// 2445 4.8.4
extern const char* cICalProperty_ATTENDEE;
extern const char* cICalProperty_CONTACT;
extern const char* cICalProperty_ORGANIZER;
extern const char* cICalProperty_RECURRENCE_ID;
extern const char* cICalProperty_RELATED_TO;
extern const char* cICalProperty_URL;
extern const char* cICalProperty_UID;

// 2445 4.8.5
extern const char* cICalProperty_EXDATE;
extern const char* cICalProperty_EXRULE;
extern const char* cICalProperty_RDATE;
extern const char* cICalProperty_RRULE;

// 2445 4.8.6
extern const char* cICalProperty_ACTION;
extern const char* cICalProperty_REPEAT;
extern const char* cICalProperty_TRIGGER;

// 2445 4.8.7
extern const char* cICalProperty_CREATED;
extern const char* cICalProperty_DTSTAMP;
extern const char* cICalProperty_LAST_MODIFIED;
extern const char* cICalProperty_SEQUENCE;

// 2445 4.8.8.2
extern const char* cICalProperty_REQUEST_STATUS;

// Enums
// Use ascending order for sensible sorting

// 2445 4.3.10

enum ERecurrence_FREQ
{
	eRecurrence_SECONDLY,
	eRecurrence_MINUTELY,
	eRecurrence_HOURLY,
	eRecurrence_DAILY,
	eRecurrence_WEEKLY,
	eRecurrence_MONTHLY,
	eRecurrence_YEARLY

};

extern const char* cICalValue_RECUR_FREQ;
const unsigned long cICalValue_RECUR_FREQ_LEN = 5;

extern const char* cICalValue_RECUR_SECONDLY;
extern const char* cICalValue_RECUR_MINUTELY;
extern const char* cICalValue_RECUR_HOURLY;
extern const char* cICalValue_RECUR_DAILY;
extern const char* cICalValue_RECUR_WEEKLY;
extern const char* cICalValue_RECUR_MONTHLY;
extern const char* cICalValue_RECUR_YEARLY;

extern const char* cICalValue_RECUR_UNTIL;
extern const char* cICalValue_RECUR_COUNT;

extern const char* cICalValue_RECUR_INTERVAL;
extern const char* cICalValue_RECUR_BYSECOND;
extern const char* cICalValue_RECUR_BYMINUTE;
extern const char* cICalValue_RECUR_BYHOUR;
extern const char* cICalValue_RECUR_BYDAY;
extern const char* cICalValue_RECUR_BYMONTHDAY;
extern const char* cICalValue_RECUR_BYYEARDAY;
extern const char* cICalValue_RECUR_BYWEEKNO;
extern const char* cICalValue_RECUR_BYMONTH;
extern const char* cICalValue_RECUR_BYSETPOS;
extern const char* cICalValue_RECUR_WKST;

enum ERecurrence_WEEKDAY
{
	eRecurrence_WEEKDAY_SU,
	eRecurrence_WEEKDAY_MO,
	eRecurrence_WEEKDAY_TU,
	eRecurrence_WEEKDAY_WE,
	eRecurrence_WEEKDAY_TH,
	eRecurrence_WEEKDAY_FR,
	eRecurrence_WEEKDAY_SA

};

extern const char* cICalValue_RECUR_WEEKDAY_SU;
extern const char* cICalValue_RECUR_WEEKDAY_MO;
extern const char* cICalValue_RECUR_WEEKDAY_TU;
extern const char* cICalValue_RECUR_WEEKDAY_WE;
extern const char* cICalValue_RECUR_WEEKDAY_TH;
extern const char* cICalValue_RECUR_WEEKDAY_FR;
extern const char* cICalValue_RECUR_WEEKDAY_SA;

// 2445 4.8.1.11
enum EStatus_VEvent
{
	eStatus_VEvent_None,
	eStatus_VEvent_Confirmed,
	eStatus_VEvent_Tentative,
	eStatus_VEvent_Cancelled
};

enum EStatus_VToDo
{
	eStatus_VToDo_None,
	eStatus_VToDo_NeedsAction,
	eStatus_VToDo_InProcess,
	eStatus_VToDo_Completed,
	eStatus_VToDo_Cancelled
};

enum EStatus_VJournal
{
	eStatus_VJournal_None,
	eStatus_VJournal_Final,
	eStatus_VJournal_Draft,
	eStatus_VJournal_Cancelled
};

extern const char* cICalProperty_STATUS_TENTATIVE;
extern const char* cICalProperty_STATUS_CONFIRMED;
extern const char* cICalProperty_STATUS_CANCELLED;
extern const char* cICalProperty_STATUS_NEEDS_ACTION;
extern const char* cICalProperty_STATUS_COMPLETED;
extern const char* cICalProperty_STATUS_IN_PROCESS;
extern const char* cICalProperty_STATUS_DRAFT;
extern const char* cICalProperty_STATUS_FINAL;

// 2445 4.8.6.1
enum EAction_VAlarm
{
	eAction_VAlarm_Audio,
	eAction_VAlarm_Display,
	eAction_VAlarm_Email,
	eAction_VAlarm_Procedure,
	eAction_VAlarm_Unknown
};

extern const char* cICalProperty_ACTION_AUDIO;
extern const char* cICalProperty_ACTION_DISPLAY;
extern const char* cICalProperty_ACTION_EMAIL;
extern const char* cICalProperty_ACTION_PROCEDURE;


// Mulberry extensions
extern const char* cICalProperty_X_PRIVATE_METHOD;
extern const char* cICalProperty_X_PRIVATE_RURL;
extern const char* cICalProperty_X_PRIVATE_ETAG;

extern const char* cICalProperty_ACTION_X_SPEAKTEXT;
extern const char* cICalProperty_ALARM_X_LASTTRIGGER;
extern const char* cICalProperty_ALARM_X_ALARMSTATUS;

enum EAlarm_Status
{
	eAlarm_Status_Pending,
	eAlarm_Status_Completed,
	eAlarm_Status_Disabled
};
extern const char* cICalProperty_ALARM_X_ALARMSTATUS_PENDING;
extern const char* cICalProperty_ALARM_X_ALARMSTATUS_COMPLETED;
extern const char* cICalProperty_ALARM_X_ALARMSTATUS_DISABLED;

extern const char* cICalAttribute_ORGANIZER_X_IDENTITY;
extern const char* cICalAttribute_ATTENDEE_X_NEEDS_ITIP;

}	// namespace iCal

#endif	// CICalendarDefinitions_H
