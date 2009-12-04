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
	CICalendarDefinitions.cpp

	Author:
	Description:	<describe the CICalendarDefinitions class here>
*/

#include "CICalendarDefinitions.h"

namespace iCal
{

// 2445 Component Header/Footer

const char* cICalComponent_BEGINVCALENDAR = "BEGIN:VCALENDAR";
const char* cICalComponent_ENDVCALENDAR = "END:VCALENDAR";
const char* cICalComponent_BEGINVEVENT = "BEGIN:VEVENT";
const char* cICalComponent_ENDVEVENT = "END:VEVENT";
const char* cICalComponent_BEGINVTODO = "BEGIN:VTODO";
const char* cICalComponent_ENDVTODO = "END:VTODO";
const char* cICalComponent_BEGINVJOURNAL = "BEGIN:VJOURNAL";
const char* cICalComponent_ENDVJOURNAL = "END:VJOURNAL";
const char* cICalComponent_BEGINVFREEBUSY = "BEGIN:VFREEBUSY";
const char* cICalComponent_ENDVFREEBUSY = "END:VFREEBUSY";
const char* cICalComponent_BEGINVTIMEZONE = "BEGIN:VTIMEZONE";
const char* cICalComponent_ENDVTIMEZONE = "END:VTIMEZONE";
const char* cICalComponent_BEGINVALARM = "BEGIN:VALARM";
const char* cICalComponent_ENDVALARM = "END:VALARM";


// Pseudo components
const char* cICalComponent_BEGINSTANDARD = "BEGIN:STANDARD";
const char* cICalComponent_ENDSTANDARD = "END:STANDARD";
const char* cICalComponent_BEGINDAYLIGHT = "BEGIN:DAYLIGHT";
const char* cICalComponent_ENDDAYLIGHT = "END:DAYLIGHT";

// 2445 Calendar Property Atrributes

// 2445 4.2
const char* cICalAttribute_ALTREP = "ALTREP";
const char* cICalAttribute_CN = "CN";
const char* cICalAttribute_CUTYPE = "CUTYPE";
const char* cICalAttribute_DELEGATED_FROM = "DELEGATED-FROM";
const char* cICalAttribute_DELEGATED_TO = "DELEGATED-TO";
const char* cICalAttribute_DIR = "DIR";
const char* cICalAttribute_ENCODING = "ENCODING";
const char* cICalAttribute_FMTTYPE = "FMTTYPE";
const char* cICalAttribute_FBTYPE = "FBTYPE";
const char* cICalAttribute_LANGUAGE = "LANGUAGE";
const char* cICalAttribute_MEMBER = "MEMBER";
const char* cICalAttribute_PARTSTAT = "PARTSTAT";
const char* cICalAttribute_RANGE = "RANGE";
const char* cICalAttribute_RELATED = "RELATED";
const char* cICalAttribute_RELTYPE = "RELTYPE";
const char* cICalAttribute_ROLE = "ROLE";
const char* cICalAttribute_RSVP = "RSVP";
const char* cICalAttribute_RSVP_TRUE = "TRUE";
const char* cICalAttribute_RSVP_FALSE = "FALSE";
const char* cICalAttribute_SENT_BY = "SENT-BY";
const char* cICalAttribute_TZID = "TZID";
const char* cICalAttribute_VALUE = "VALUE";

// 2445 4.2.9
const char* cICalAttribute_FBTYPE_FREE = "FREE";
const char* cICalAttribute_FBTYPE_BUSY = "BUSY";
const char* cICalAttribute_FBTYPE_BUSYUNAVAILABLE = "BUSY-UNAVAILABLE";
const char* cICalAttribute_FBTYPE_BUSYTENTATIVE = "BUSY-TENTATIVE";

// 2445 4.2.12
const char* cICalAttribute_PARTSTAT_NEEDSACTION = "NEEDS-ACTION";
const char* cICalAttribute_PARTSTAT_ACCEPTED = "ACCEPTED";
const char* cICalAttribute_PARTSTAT_DECLINED = "DECLINED";
const char* cICalAttribute_PARTSTAT_TENTATIVE = "TENTATIVE";
const char* cICalAttribute_PARTSTAT_DELEGATED = "DELEGATED";
const char* cICalAttribute_PARTSTAT_COMPLETED = "COMPLETE";
const char* cICalAttribute_PARTSTAT_INPROCESS = "IN-PROCESS";

// 2445 4.2.13
const char* cICalAttribute_RANGE_THISANDFUTURE = "THISANDFUTURE";
const char* cICalAttribute_RANGE_THISANDPRIOR = "THISANDPRIOR";

// 2445 4.2.14
const char* cICalAttribute_RELATED_START = "START";
const char* cICalAttribute_RELATED_END = "END";

// 2445 4.2.16
const char* cICalAttribute_ROLE_CHAIR = "CHAIR";
const char* cICalAttribute_ROLE_REQ_PART = "REQ-PARTICIPANT";
const char* cICalAttribute_ROLE_OPT_PART = "OPT-PARTICIPANT";
const char* cICalAttribute_ROLE_NON_PART = "NON-PARTICIPANT";

// 2445 Value types

// 2445 4.3
const char* cICalValue_BINARY = "BINARY";
const char* cICalValue_BOOLEAN = "BOOLEAN";
const char* cICalValue_CAL_ADDRESS = "CAL-ADDRESS";
const char* cICalValue_DATE = "DATE";
const char* cICalValue_DATE_TIME = "DATE-TIME";
const char* cICalValue_DURATION = "DURATION";
const char* cICalValue_FLOAT = "FLOAT";
const char* cICalValue_INTEGER = "INTEGER";
const char* cICalValue_PERIOD = "PERIOD";
const char* cICalValue_RECUR = "RECUR";
const char* cICalValue_TEXT = "TEXT";
const char* cICalValue_TIME = "TIME";
const char* cICalValue_URI = "URI";
const char* cICalValue_UTC_OFFSET = "UTC-OFFSET";

// 2445 Calendar Properties

// 2445  4.7

const char* cICalProperty_CALSCALE = "CALSCALE";
const char* cICalProperty_METHOD = "METHOD";
const char* cICalProperty_PRODID = "PRODID";
const char* cICalProperty_VERSION = "VERSION";

// Apple Extensions
const char* cICalProperty_XWRCALNAME = "X-WR-CALNAME";
const char* cICalProperty_XWRCALDESC = "X-WR-CALDESC";

// 2445 Component Property names

// 2445 4.8.1
const char* cICalProperty_ATTACH = "ATTACH";
const char* cICalProperty_CATEGORIES = "CATEGORIES";
const char* cICalProperty_CLASS = "CLASS";
const char* cICalProperty_COMMENT = "COMMENT";
const char* cICalProperty_DESCRIPTION = "DESCRIPTION";
const char* cICalProperty_GEO = "GEO";
const char* cICalProperty_LOCATION = "LOCATION";
const char* cICalProperty_PERCENT_COMPLETE = "PERCENT-COMPLETE";
const char* cICalProperty_PRIORITY = "PRIORITY";
const char* cICalProperty_RESOURCES = "RESOURCES";
const char* cICalProperty_STATUS = "STATUS";
const char* cICalProperty_SUMMARY = "SUMMARY";

// 2445 4.8.2
const char* cICalProperty_COMPLETED = "COMPLETED";
const char* cICalProperty_DTEND = "DTEND";
const char* cICalProperty_DUE = "DUE";
const char* cICalProperty_DTSTART = "DTSTART";
const char* cICalProperty_DURATION = "DURATION";
const char* cICalProperty_FREEBUSY = "FREEBUSY";
const char* cICalProperty_TRANSP = "TRANSP";
const char* cICalProperty_OPAQUE = "OPAQUE";
const char* cICalProperty_TRANSPARENT = "TRANSPARENT";

// 2445 4.8.3
const char* cICalProperty_TZID = "TZID";
const char* cICalProperty_TZNAME = "TZNAME";
const char* cICalProperty_TZOFFSETFROM = "TZOFFSETFROM";
const char* cICalProperty_TZOFFSETTO = "TZOFFSETTO";
const char* cICalProperty_TZURL = "TZURL";

// 2445 4.8.4
const char* cICalProperty_ATTENDEE = "ATTENDEE";
const char* cICalProperty_CONTACT = "CONTACT";
const char* cICalProperty_ORGANIZER = "ORGANIZER";
const char* cICalProperty_RECURRENCE_ID = "RECURRENCE-ID";
const char* cICalProperty_RELATED_TO = "RELATED-TO";
const char* cICalProperty_URL = "URL";
const char* cICalProperty_UID = "UID";

// 2445 4.8.5
const char* cICalProperty_EXDATE = "EXDATE";
const char* cICalProperty_EXRULE = "EXRULE";
const char* cICalProperty_RDATE = "RDATE";
const char* cICalProperty_RRULE = "RRULE";

// 2445 4.8.6
const char* cICalProperty_ACTION = "ACTION";
const char* cICalProperty_REPEAT = "REPEAT";
const char* cICalProperty_TRIGGER = "TRIGGER";

// 2445 4.8.7
const char* cICalProperty_CREATED = "CREATED";
const char* cICalProperty_DTSTAMP = "DTSTAMP";
const char* cICalProperty_LAST_MODIFIED = "LAST-MODIFIED";
const char* cICalProperty_SEQUENCE = "SEQUENCE";

// 2445 4.8.8.2
const char* cICalProperty_REQUEST_STATUS = "REQUEST-STATUS";

// Enums

// 2445 4.3.10

const char* cICalValue_RECUR_FREQ = "FREQ=";

const char* cICalValue_RECUR_SECONDLY = "SECONDLY";
const char* cICalValue_RECUR_MINUTELY = "MINUTELY";
const char* cICalValue_RECUR_HOURLY = "HOURLY";
const char* cICalValue_RECUR_DAILY = "DAILY";
const char* cICalValue_RECUR_WEEKLY = "WEEKLY";
const char* cICalValue_RECUR_MONTHLY = "MONTHLY";
const char* cICalValue_RECUR_YEARLY = "YEARLY";

const char* cICalValue_RECUR_UNTIL = "UNTIL=";
const char* cICalValue_RECUR_COUNT = "COUNT=";

const char* cICalValue_RECUR_INTERVAL = "INTERVAL=";
const char* cICalValue_RECUR_BYSECOND = "BYSECOND=";
const char* cICalValue_RECUR_BYMINUTE = "BYMINUTE=";
const char* cICalValue_RECUR_BYHOUR = "BYHOUR=";
const char* cICalValue_RECUR_BYDAY = "BYDAY=";
const char* cICalValue_RECUR_BYMONTHDAY = "BYMONTHDAY=";
const char* cICalValue_RECUR_BYYEARDAY = "BYYEARDAY=";
const char* cICalValue_RECUR_BYWEEKNO = "BYWEEKNO=";
const char* cICalValue_RECUR_BYMONTH = "BYMONTH=";
const char* cICalValue_RECUR_BYSETPOS = "BYSETPOS=";
const char* cICalValue_RECUR_WKST = "WKST=";

const char* cICalValue_RECUR_WEEKDAY_SU = "SU";
const char* cICalValue_RECUR_WEEKDAY_MO = "MO";
const char* cICalValue_RECUR_WEEKDAY_TU = "TU";
const char* cICalValue_RECUR_WEEKDAY_WE = "WE";
const char* cICalValue_RECUR_WEEKDAY_TH = "TH";
const char* cICalValue_RECUR_WEEKDAY_FR = "FR";
const char* cICalValue_RECUR_WEEKDAY_SA = "SA";

// 2445 4.8.1.11
const char* cICalProperty_STATUS_TENTATIVE = "TENTATIVE";
const char* cICalProperty_STATUS_CONFIRMED = "CONFIRMED";
const char* cICalProperty_STATUS_CANCELLED = "CANCELLED";
const char* cICalProperty_STATUS_NEEDS_ACTION = "NEEDS-ACTION";
const char* cICalProperty_STATUS_COMPLETED = "COMPLETED";
const char* cICalProperty_STATUS_IN_PROCESS = "IN-PROCESS";
const char* cICalProperty_STATUS_DRAFT = "DRAFT";
const char* cICalProperty_STATUS_FINAL = "FINAL";

// 2445 4.8.6.1
const char* cICalProperty_ACTION_AUDIO = "AUDIO";
const char* cICalProperty_ACTION_DISPLAY = "DISPLAY";
const char* cICalProperty_ACTION_EMAIL = "EMAIL";
const char* cICalProperty_ACTION_PROCEDURE = "PROCEDURE";

// Mulberry extensions
const char* cICalProperty_X_PRIVATE_METHOD = "X-MULBERRY-PRIVATE-METHOD";
const char* cICalProperty_X_PRIVATE_RURL = "X-MULBERRY-PRIVATE-RURL";
const char* cICalProperty_X_PRIVATE_ETAG = "X-MULBERRY-PRIVATE-ETAG";

const char* cICalProperty_ACTION_X_SPEAKTEXT = "X-MULBERRY-SPEAK-TEXT";
const char* cICalProperty_ALARM_X_LASTTRIGGER = "X-MULBERRY-LAST-TRIGGER";

const char* cICalProperty_ALARM_X_ALARMSTATUS = "X-MULBERRY-ALARM-STATUS";
const char* cICalProperty_ALARM_X_ALARMSTATUS_PENDING = "PENDING";
const char* cICalProperty_ALARM_X_ALARMSTATUS_COMPLETED = "COMPLETED";
const char* cICalProperty_ALARM_X_ALARMSTATUS_DISABLED = "DISABLED";

const char* cICalAttribute_ORGANIZER_X_IDENTITY = "X-MULBERRY-IDENTITY";
const char* cICalAttribute_ATTENDEE_X_NEEDS_ITIP = "X-MULBERRY-NEEDS-ITIP";

};
