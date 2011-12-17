/*
	CICalendarInit.cpp

	Author:
	Description:	<describe the CICalendar class here>
*/

#include "CICalendar.h"

#include "CICalendarComponent.h"
#include "CICalendarProperty.h"

using namespace iCal;

CICalendarProperty::CValueTypeMap CICalendarProperty::sDefaultValueTypeMap;
CICalendarProperty::CValueTypeMap CICalendarProperty::sValueTypeMap;
CICalendarProperty::CTypeValueMap CICalendarProperty::sTypeValueMap;
CICalendarProperty::CMultiValues CICalendarProperty::sMultiValues;

CICalendar::CICalendarRefMap CICalendar::sICalendars;
//CICalendar CICalendar::sICalendar;
CICalendarRef CICalendar::sICalendarRefCtr = 1;
