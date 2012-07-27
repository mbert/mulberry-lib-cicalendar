/*
	CITIPProcessor.h

	Author:
	Description:	<describe the CITIPProcessor class here>
*/

#ifndef CITIPProcessor_H
#define CITIPProcessor_H

#include "cdstring.h"

#include "CICalendarComponent.h"
#include "CICalendarComponentRecur.h"
#include "CICalendarPeriod.h"
#include "CITIPScheduleResults.h"

#include "CCalendarAddress.h"

class CDataAttachment;
class CIdentity;
class CMessage;

namespace calstore {

class CCalendarProtocol;

}

namespace iCal {

class CICalendar;
class CICalendarVEvent;
class CICalendarVToDo;

class CITIPProcessor
{
public:

	enum EAttendeeState
	{
		eNone,
		eAllAccepted,
		eSomeAccepted,
		eSomeDeclined,
		eIHaveAccepted,
		eIHaveDeclined,
		eIHaveNotAccepted
	};

	enum EDescriptionType
	{
		ePublish,
		eRequest,
		eReplyAccepted,
		eReplyDeclined
	};

	static void	SendCalendar(const CICalendar& cal, CDataAttachment*& data);
	static void	PublishEvents(const CICalendarComponentRecurs& vevents, const CIdentity* id);
	static void	PublishComponent(const CICalendarVToDo* vtodo, const cdstring& organiser, cdstring& desc, CDataAttachment*& attach);

	static void SendRequest(const CCalendarAddressList* addrs, const CICalendarComponent* comp, const CIdentity* id);

	static void FreeBusyRequest(const CICalendarProperty* organizer,
								const CICalendarPropertyList* attendees,
								const CICalendar* cal,
								const CIdentity* id, 
								const CICalendarPeriod& period,
								CITIPScheduleResultsList& results);

	static void ProcessAttachment(const char* data, CMessage* msg);
	static void ProcessCalDAVComponent(CICalendarComponent* comp);

	static bool NeedsITIPRequest(const CICalendarComponent& comp);
	static void GetRequestDetails(const CICalendarComponent& comp, CCalendarAddressList& addrs, cdstring& id);
	static void ClearITIPRequest(CICalendarComponent& comp);

	static bool OrganiserIsMe(const CICalendarComponent& comp);
	static const CIdentity* OrganiserIdentity(const CICalendarComponent& comp);
	static const CICalendarProperty* AttendeeMe(const CICalendarComponent& comp);

	static bool AttendeeIdentity(const CICalendarProperty& attendee, const CIdentity*& id);
	static EAttendeeState GetAttendeeState(const CICalendarComponent& comp);
	static cdstring GetAttendeeDescriptor(const CICalendarProperty& attendee);
	static cdstring GetAttendeeFullDescriptor(const CICalendarProperty& attendee);
	static cdstring GetAttendeeRoleDescriptor(const CICalendarProperty& attendee);
	static cdstring GetAttendeeStatusDescriptor(const CICalendarProperty& attendee);

private:
	CITIPProcessor();
	~CITIPProcessor();

	static void SendiMIPRequest(const CCalendarAddressList* addrs, const CICalendar& cal, const CICalendarComponent* comp, const CIdentity* id);
	static void SendCalDAVRequest(const CCalendarAddressList* addrs, const CICalendar& cal, const CICalendarComponent* comp, const CIdentity* id, const calstore::CCalendarProtocol* proto);

	static void SendiMIPReply(const CICalendarComponent* comp, const cdstring& status, const CICalendarProperty& attendee, const CIdentity* id, CMessage* msg, EDescriptionType type);
	static void SendCalDAVReply(const CICalendarComponent* comp, const cdstring& status, const CICalendarProperty& attendee, const CIdentity* id, const calstore::CCalendarProtocol* proto, EDescriptionType type);

	static bool ReceivePublish(const CICalendar& cal);
	static bool ReceiveRequest(const CICalendar& cal, const calstore::CCalendarProtocol* proto = NULL, const CIdentity* tied_identity = NULL, CMessage* msg = NULL);
	static bool ReceiveRefresh(const CICalendar& cal, const calstore::CCalendarProtocol* proto = NULL, const CIdentity* tied_identity = NULL, CMessage* msg = NULL);
	static bool ReceiveCancel(const CICalendar& cal, const calstore::CCalendarProtocol* proto = NULL, const CIdentity* tied_identity = NULL, CMessage* msg = NULL);
	static bool ReceiveAdd(const CICalendar& cal, const calstore::CCalendarProtocol* proto = NULL, const CIdentity* tied_identity = NULL, CMessage* msg = NULL);
	static bool ReceiveReply(const CICalendar& cal, const calstore::CCalendarProtocol* proto = NULL, const CIdentity* tied_identity = NULL, CMessage* msg = NULL);
	static bool ReceiveCounter(const CICalendar& cal, const calstore::CCalendarProtocol* proto = NULL, const CIdentity* tied_identity = NULL, CMessage* msg = NULL);
	static bool ReceiveDeclineCounter(const CICalendar& cal, const calstore::CCalendarProtocol* proto = NULL, const CIdentity* tied_identity = NULL, CMessage* msg = NULL);
	
	enum EFoundComponent
	{
		eNotFound,
		eFoundExact,
		eFoundMaster
	};

	static EFoundComponent LookForComponentInCalendars(const CICalendarComponent* orig, CICalendarComponent*& found);
	
	static void GenerateMIMEData(const CICalendar* cal, const CICalendarComponent* comp, const cdstring& mime_method, cdstring& desc, CDataAttachment*& attach, EDescriptionType type);

	static void DescribeComponent(const CICalendarComponent* comp, cdstring& desc, EDescriptionType type);
	static void DescribeComponent(const CICalendarVEvent* vevent, cdstring& desc, EDescriptionType type);
	static void DescribeComponent(const CICalendarVToDo* vtodo, cdstring& desc, EDescriptionType type);
	
	// REQUEST processing
	enum ECompareComponents
	{
		eSEQNewer		= 1L << 0,
		eDTSTAMPNewer	= 1L << 1,
		eSEQOlder		= 1L << 2,
		eDTSTAMPOlder	= 1L << 3,
		eTheSame		= 1L << 4
	};
	static ECompareComponents Compare(const CICalendarVEvent* comp_old, const CICalendarVEvent* comp_new);

	static bool CheckForReschedule(const CICalendarVEvent* comp_old, const CICalendarVEvent* comp_new);
	static bool CheckForConflicts(const CICalendarVEvent* comp);
	static void DetermineITIPBusyPeriods(const CICalendarVEvent* comp, CICalendarPeriodList& busy);
	static void GetAttendeeForIdentity(const CICalendarComponent& comp, const CIdentity*& id, const CICalendarProperty*& attendee);
	
	static bool GetOrganiserAddress(const CICalendarComponent* comp, CCalendarAddress& addr);
	static bool GetMatchingAttendee(CICalendarComponent* comp, const CICalendarProperty& attendee, CICalendarProperty*& found);
};

}	// namespace iCal

#endif	// CICalendar_H
