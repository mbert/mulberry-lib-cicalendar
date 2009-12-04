/*
	CITIPProcessor.cpp

	Author:
	Description:	<describe the CITIPProcessor class here>
*/

#include "CITIPProcessor.h"

#include "CActionManager.h"
#include "CAddressList.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CDataAttachment.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CICalendar.h"
#include "CICalendarCalAddressValue.h"
#include "CICalendarDefinitions.h"
#include "CICalendarManager.h"
#include "CICalendarVEvent.h"
#include "CICalendarVFreeBusy.h"
#include "CICalendarVJournal.h"
#include "CICalendarVTimezone.h"
#include "CICalendarVTimezoneDaylight.h"
#include "CICalendarVTimezoneStandard.h"
#include "CICalendarVToDo.h"
#include "CITIPDefinitions.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessage.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CURL.h"

#include "CXStringResources.h"

#include <strstream>

using namespace iCal;

#define ALLOW_CALDAV	1
#define ALWAYS_RSVP		1

void CITIPProcessor::SendCalendar(const CICalendar& cal, CDataAttachment*& attach)
{
	// Generate data
	std::ostrstream ostr;
	cal.Generate(ostr);
	ostr << std::ends;

	// iCal uses \n endls we want local endls
	cdstring data;
	data.steal(ostr.str());
	data.ConvertEndl();

	// Create text/calendar part
	attach = new CDataAttachment(data.release());

	attach->GetContent().SetContent(eContentText, eContentSubCalendar);
	attach->GetContent().SetCharset(i18n::eUTF8);
	if (!cal.GetName().empty())
	{
		cdstring name = cal.GetName();
		if (!name.compare_end(".ics", true))
			name += ".ics";
		attach->GetContent().SetContentParameter(cMIMEParameter[eName], name);
	}
}

void CITIPProcessor::PublishEvents(const CICalendarComponentRecurs& vevents, const CIdentity* id)
{
	CCalendarAddressList caladdress;
	CCalendarAddress::FromIdentityText(id->GetCalendarAddress(), caladdress);
	cdstring description;

	// Create a calendar to hold the component
	CICalendar cal;
	
	// Add the METHOD property
	cal.AddProperty(CICalendarProperty(cICalProperty_METHOD, cICalMethod_PUBLISH));
	
	// Copy the events and process for PUBLISH method
	for(CICalendarComponentRecurs::const_iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		CICalendarVEvent* copy = new CICalendarVEvent(*static_cast<CICalendarVEvent*>(*iter));
		copy->SetCalendar(cal.GetRef());
		cal.AddNewVEvent(copy, true);

		// Apply iTIP requirements to VEVENT component

		// iTIP requires ORGANIZER be present
		if (!copy->HasProperty(cICalProperty_ORGANIZER))
		{
			// Create the property
			CICalendarProperty prop(cICalProperty_ORGANIZER, caladdress.front()->GetAddress(), CICalendarValue::eValueType_CalAddress);
			
			// If a full name is present, create the CN= attribute
			if (!caladdress.front()->GetName().empty())
			{
				prop.AddAttribute(CICalendarAttribute(cICalAttribute_CN, caladdress.front()->GetName()));
			}

			// Now add the property
			copy->AddProperty(prop);
		}

		// iTIP requires the following NOT be present
		copy->RemoveProperties(cICalProperty_ATTENDEE);
		copy->RemoveProperties(cICalProperty_REQUEST_STATUS);

		// Create friendly descriptor of text
		cdstring temp;
		DescribeComponent(copy, temp, ePublish);
		description += temp;
	}

	// Now generate data
	std::ostrstream ostr;
	cal.Generate(ostr);
	ostr << std::ends;
	
	// iCal uses \n endls we want local endls
	cdstring data;
	data.steal(ostr.str());
	data.ConvertEndl();

	// Create text/calendar part
	CDataAttachment* attach = new CDataAttachment(data.release());

	attach->GetContent().SetContent(eContentText, eContentSubCalendar);
	attach->GetContent().SetCharset(i18n::eUTF8);
	attach->GetContent().SetContentParameter(cMIMEParameter[eMethod], cICalMIMEMethod_PUBLISH);
	attach->GetContent().SetContentParameter(cMIMEParameter[eComponent], cICalMIMEComponent_VEVENT);
	attach->GetContent().SetContentParameter(cMIMEParameter[eName], "event.ics");
	
	// Create a new draft with this component as a text/calendar part
	CActionManager::NewDraftiTIP(cdstring::null_str, description, attach, id, NULL);
}

void CITIPProcessor::DescribeComponent(const CICalendarVEvent* vevent, cdstring& desc, EDescriptionType type)
{
	std::ostrstream ostr;
	
	// Header first
	switch(type)
	{
	case ePublish:
		ostr << rsrc::GetString("EventDescription::HeaderPublish");
		break;
	case eRequest:
		ostr << rsrc::GetString("EventDescription::HeaderRequest") << os_endl2;
		ostr << rsrc::GetString("EventDescription::Invitation");
		break;
	case eReplyAccepted:
		ostr << rsrc::GetString("EventDescription::HeaderReplyAccepted") << os_endl2;
		ostr << rsrc::GetString("EventDescription::Accepted");
		break;
	case eReplyDeclined:
		ostr << rsrc::GetString("EventDescription::HeaderReplyDeclined") << os_endl2;
		ostr << rsrc::GetString("EventDescription::Declined");
		break;
	}
	ostr << os_endl2;

	// Organizer
	CCalendarAddress organiser;
	if (GetOrganiserAddress(vevent, organiser))
	{
		ostr << rsrc::GetString("EventDescription::Organizer") << organiser.GetFullAddress() << os_endl;
	}

	// Standard event items
	ostr << rsrc::GetString("EventDescription::Summary") << vevent->GetSummary() << os_endl;

	CICalendarPeriod period(vevent->GetStart(), vevent->GetEnd());
	if (vevent->GetStart().IsDateOnly())
	{
		ostr << rsrc::GetString("EventDescription::All Day Event") << os_endl;

		// Display start/end (end only if more than one day)
		CICalendarDateTime temp(vevent->GetEnd());
		temp.OffsetDay(-1);
		
		if (vevent->GetStart() == temp)
		{
			ostr << rsrc::GetString("EventDescription::On") << vevent->GetStart().GetLocaleDate(CICalendarDateTime::eFullDate) << os_endl;
			ostr << rsrc::GetString("EventDescription::Duration") << period.DescribeDuration() << os_endl;
		}
		else
		{
			ostr << rsrc::GetString("EventDescription::Starts") << vevent->GetStart().GetLocaleDate(CICalendarDateTime::eFullDate) << os_endl;
			ostr << rsrc::GetString("EventDescription::Ends") << temp.GetLocaleDate(CICalendarDateTime::eFullDate) << os_endl;
			ostr << rsrc::GetString("EventDescription::Duration") << period.DescribeDuration() << os_endl;
		}
	}
	else
	{
		ostr << rsrc::GetString("EventDescription::Starts") << vevent->GetStart().GetLocaleDateTime(CICalendarDateTime::eFullDate, false, true, true) << os_endl;
		ostr << rsrc::GetString("EventDescription::Ends") << vevent->GetEnd().GetLocaleDateTime(CICalendarDateTime::eFullDate, false, true, true) << os_endl;
		ostr << rsrc::GetString("EventDescription::Duration") << period.DescribeDuration() << os_endl;
	}
	if (vevent->IsRecurring())
	{
		ostr << rsrc::GetString("EventDescription::Recurring") << os_endl;
	}

	// Other useful info
	if (!vevent->GetLocation().empty())
		ostr << rsrc::GetString("EventDescription::Location") << vevent->GetLocation() << os_endl;
	if (!vevent->GetDescription().empty())
		ostr << rsrc::GetString("EventDescription::Description") << vevent->GetDescription() << os_endl;
	
	// Attendees
	if (vevent->HasProperty(cICalProperty_ATTENDEE))
	{
		// Write out each attendee
		std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> range = vevent->GetProperties().equal_range(cICalProperty_ATTENDEE);
		for(CICalendarPropertyMap::const_iterator iter = range.first; iter != range.second; iter++)
		{
			// Get the organizer value
			const CICalendarCalAddressValue* attendee = (*iter).second.GetCalAddressValue();
			if (attendee != NULL)
			{
				ostr << rsrc::GetString("EventDescription::Attendee") << attendee->GetValue() << os_endl;
			}
		}
	}

	// Footer first
	ostr << os_endl;
	switch(type)
	{
	case ePublish:
		ostr << rsrc::GetString("EventDescription::FooterPublish");
		break;
	case eRequest:
		ostr << rsrc::GetString("EventDescription::FooterRequest");
		break;
	case eReplyAccepted:
		ostr << rsrc::GetString("EventDescription::FooterReplyAccepted");
		break;
	case eReplyDeclined:
		ostr << rsrc::GetString("EventDescription::FooterReplyDeclined");
		break;
	}
	ostr << os_endl2;

	ostr << std::ends;
	desc.steal(ostr.str());
}

void CITIPProcessor::PublishComponent(const CICalendarVToDo* vtodo, const cdstring& organiser, cdstring& desc, CDataAttachment*& attach)
{
	
}

void CITIPProcessor::DescribeComponent(const CICalendarVToDo* vtodo, cdstring& desc, EDescriptionType type)
{
	
}

void CITIPProcessor::DescribeComponent(const CICalendarComponent* comp, cdstring& desc, EDescriptionType type)
{
	switch(comp->GetType())
	{
	case CICalendarComponent::eVEVENT:
		DescribeComponent(static_cast<const CICalendarVEvent*>(comp), desc, type);
		break;
	case CICalendarComponent::eVTODO:
		DescribeComponent(static_cast<const CICalendarVToDo*>(comp), desc, type);
		break;
	default:;
	}
}

void CITIPProcessor::SendRequest(const CCalendarAddressList* addrs, const CICalendarComponent* comp, const CIdentity* id)
{
	cdstring subject;
	if (!comp->GetProperty(cICalProperty_SUMMARY, subject))
		subject.FromResource("CITIPProcessor::RequestSubject");
	
	// Create a calendar to hold the component
	CICalendar cal;
	
	// Add the METHOD property
	cal.AddProperty(CICalendarProperty(cICalProperty_METHOD, cICalMethod_REQUEST));
	
	// Copy the component and process for REQUEST method
	CICalendarComponent* copy = comp->clone();
	copy->SetCalendar(cal.GetRef());
	cal.AddComponent(copy);

	// Apply iTIP requirements to component

	// iTIP requires the following NOT be present
	copy->RemoveProperties(cICalProperty_REQUEST_STATUS);

#ifdef ALLOW_CALDAV
	const CICalendarRef calref = comp->GetCalendar();
	const CICalendar* calp = CICalendar::GetICalendar(calref);
	const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(calp);
	const calstore::CCalendarProtocol* proto = node->GetProtocol();

	if (false)
#endif
		SendiMIPRequest(addrs, cal, copy, id);
#ifdef ALLOW_CALDAV
	else
		SendCalDAVRequest(addrs, cal, copy, id, proto);
#endif
}

void CITIPProcessor::SendiMIPRequest(const CCalendarAddressList* addrs, const CICalendar& cal, const CICalendarComponent* comp, const CIdentity* id)
{
	// Generate MIME info
	cdstring description;
	CDataAttachment* attach;
	GenerateMIMEData(&cal, comp, cICalMethod_REQUEST, description, attach, eRequest);
	
	cdstring subject;
	if (!comp->GetProperty(cICalProperty_SUMMARY, subject))
		subject.FromResource("CITIPProcessor::RequestSubject");
	
	// Convert calendar addresses into email addresses
	CAddressList eaddrs;
	for(CCalendarAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		CURL url((*iter)->GetAddress());
		if (url.SchemeType() == CURL::eMailto)
		{
			eaddrs.push_back(new CAddress(url.Server(), (*iter)->GetName()));
		}
	}

	// Create a new draft with this component as a text/calendar part
	CActionManager::NewDraftiTIP(&eaddrs, NULL, NULL, subject, description, attach, id, NULL);
}

void CITIPProcessor::SendCalDAVRequest(const CCalendarAddressList* addrs, const CICalendar& cal, const CICalendarComponent* comp, const CIdentity* id, const calstore::CCalendarProtocol* proto)
{
#ifdef ALLOW_CALDAV
	// Get Outbox URL for CalDAV server
	cdstring inbox;
	cdstring outbox;
	const_cast<calstore::CCalendarProtocol*>(proto)->GetScheduleInboxOutbox(inbox, outbox);
	
	// Must have outbox
	if (outbox.empty())
		return;
	
	// Get originator
	CCalendarAddressList caddrs;
	CCalendarAddress::FromIdentityText(id->GetCalendarAddress(), caddrs);
	cdstring originator = caddrs.front()->GetAddress();

	// Get recipients
	cdstrvect recipients;
	for(CCalendarAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		recipients.push_back((*iter)->GetAddress());
	}

	// Send schedule request
	CITIPScheduleResultsList results;
	const_cast<calstore::CCalendarProtocol*>(proto)->Schedule(outbox, originator, recipients, cal, results);
#endif
}

void CITIPProcessor::SendiMIPReply(const CICalendarComponent* comp, const cdstring& status, const CICalendarProperty& attendee, const CIdentity* id, CMessage* msg, EDescriptionType type)
{
	cdstring subject = "Re: ";
	cdstring summary;
	if ((msg != NULL) && !msg->GetEnvelope()->GetSubject().empty())
	{
		const cdstring& osubject = msg->GetEnvelope()->GetSubject();
		if (((osubject[0UL]!='R') && (osubject[0UL]!='r')) ||
			((osubject[1UL]!='E') && (osubject[1UL]!='e')) ||
			(osubject[2UL]!=':'))
			subject += osubject;
		else
			subject = osubject;
	}
	else if (comp->GetProperty(cICalProperty_SUMMARY, summary))
	{
		subject += summary;
	}
	else
		subject.AppendResource("CITIPProcessor::ReplySubject");
	
	cdstring description;
	CDataAttachment* attach;
	
	// Create a calendar to hold the component
	CICalendar cal;
	
	// Add the METHOD property
	cal.AddProperty(CICalendarProperty(cICalProperty_METHOD, cICalMethod_REPLY));
	
	// Copy the component and process for REQUEST method
	CICalendarComponent* copy = comp->clone();
	copy->SetCalendar(cal.GetRef());
	cal.AddComponent(copy);

	// Apply iTIP requirements to component

	// Set proper status
	copy->RemoveProperties(cICalProperty_REQUEST_STATUS);
	copy->AddProperty(CICalendarProperty(cICalProperty_REQUEST_STATUS, status));

	// Remove all attendees and then add back in the one that represents this reply
	copy->RemoveProperties(cICalProperty_ATTENDEE);
	copy->AddProperty(attendee);

	// Generate MIME info
	GenerateMIMEData(&cal, copy, cICalMethod_REPLY, description, attach, type);

	// Look for ORGANISER
	CAddressList addrs;
	CCalendarAddress organiser;
	if (GetOrganiserAddress(comp, organiser))
	{
		// Must have mailto address
		CURL url(organiser.GetAddress());
		if (url.SchemeType() == CURL::eMailto)
			addrs.push_back(new CAddress(url.Server(), organiser.GetName()));
	}

	// Create a new draft with this component as a text/calendar part
	if (addrs.size() != 0)
		CActionManager::NewDraftiTIP(&addrs, NULL, NULL, subject, description, attach, id, msg);
}

void CITIPProcessor::SendCalDAVReply(const CICalendarComponent* comp, const cdstring& status, const CICalendarProperty& attendee, const CIdentity* id, const calstore::CCalendarProtocol* proto, EDescriptionType type)
{
	// Get Outbox URL for CalDAV server
	cdstring inbox;
	cdstring outbox;
	const_cast<calstore::CCalendarProtocol*>(proto)->GetScheduleInboxOutbox(inbox, outbox);
	
	// Must have outbox
	if (outbox.empty())
		return;
	
	// Get originator
	CCalendarAddressList addrs;
	CCalendarAddress::FromIdentityText(id->GetCalendarAddress(), addrs);
	cdstring originator = addrs.front()->GetAddress();
	
	// Get recipients
	cdstrvect recipients;
	CCalendarAddress organiser;
	if (GetOrganiserAddress(comp, organiser))
	{
		recipients.push_back(organiser.GetAddress());
	}

	// Create a calendar to hold the component
	CICalendar cal;
	
	// Add the METHOD property
	cal.AddProperty(CICalendarProperty(cICalProperty_METHOD, cICalMethod_REPLY));
	
	// Copy the component and process for REQUEST method
	CICalendarComponent* copy = comp->clone();
	copy->SetCalendar(cal.GetRef());
	cal.AddComponent(copy);
	
	// Apply iTIP requirements to component
	
	// Set proper status
	copy->RemoveProperties(cICalProperty_REQUEST_STATUS);
	copy->AddProperty(CICalendarProperty(cICalProperty_REQUEST_STATUS, status));
	
	// Remove all attendees and then add back in the one that represents this reply
	copy->RemoveProperties(cICalProperty_ATTENDEE);
	copy->AddProperty(attendee);
	
	// Send schedule request
	CITIPScheduleResultsList results;
	const_cast<calstore::CCalendarProtocol*>(proto)->Schedule(outbox, originator, recipients, cal, results);	
}

void CITIPProcessor::FreeBusyRequest(const CICalendarProperty* organizer,
									 const CICalendarPropertyList* attendees,
									 const CICalendar* cal,
									 const CIdentity* id,
									 const CICalendarPeriod& period,
									 CITIPScheduleResultsList& results)
{
	// Create a calendar to hold the component
	CICalendar itipcal;
	
	// Get originator - we are actually going to use this instead of the ORGANIZER.
	// That is OK except for delegation/proxy lookups.
	CCalendarAddressList addrs;
	CCalendarAddress::FromIdentityText(id->GetCalendarAddress(), addrs);
	cdstring originator = addrs.front()->GetAddress();

	CICalendarProperty actual_organizer(cICalProperty_ORGANIZER, originator, CICalendarValue::eValueType_CalAddress);

	// Add the METHOD property
	itipcal.AddProperty(CICalendarProperty(cICalProperty_METHOD, cICalMethod_REQUEST));
	
	// Create new VFREEBUSY component
	CICalendarVFreeBusy* vfb = static_cast<CICalendarVFreeBusy*>(CICalendarVFreeBusy::Create(itipcal.GetRef()));
	
	// Init timing parameters
	vfb->EditTiming(period.GetStart(), period.GetEnd());

	// Set organizer and attendees
	vfb->AddProperty(actual_organizer);

	for(CICalendarPropertyList::const_iterator iter = attendees->begin(); iter != attendees->end(); iter++)
	{
		// Copy the property
		vfb->AddProperty(*iter);
	}
	
	// Make sure UID is set and unique
	cdstring uid;
	vfb->SetUID(uid);
	
	// Init DTSTAMP to now
	vfb->InitDTSTAMP();

	itipcal.AddComponent(vfb);
	
	// Apply iTIP requirements to component
	
	const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(cal);
	const calstore::CCalendarProtocol* proto = node->GetProtocol();

	// Get Outbox URL for CalDAV server
	cdstring inbox;
	cdstring outbox;
	const_cast<calstore::CCalendarProtocol*>(proto)->GetScheduleInboxOutbox(inbox, outbox);
	
	// Must have outbox
	if (outbox.empty())
		return;
	
	// Get recipients
	cdstrvect recipients;
	for(CICalendarPropertyList::const_iterator iter = attendees->begin(); iter != attendees->end(); iter++)
	{
		recipients.push_back((*iter).GetCalAddressValue()->GetValue());
	}
	
	// Send schedule request
	const_cast<calstore::CCalendarProtocol*>(proto)->Schedule(outbox, originator, recipients, itipcal, results);
}

void CITIPProcessor::GenerateMIMEData(const CICalendar* cal, const CICalendarComponent* comp, const cdstring& mime_method, cdstring& desc, CDataAttachment*& attach, EDescriptionType type)
{
	// Create friendly descriptor of text
	cdstring temp;
	DescribeComponent(comp, temp, type);
	desc += temp;

	// Now generate data
	std::ostrstream ostr;
	cal->Generate(ostr);
	ostr << std::ends;
	
	// iCal uses \n endls we want local endls
	cdstring data;
	data.steal(ostr.str());
	data.ConvertEndl();

	// Create text/calendar part
	attach = new CDataAttachment(data.release());

	attach->GetContent().SetContent(eContentText, eContentSubCalendar);
	attach->GetContent().SetCharset(i18n::eUTF8);
	attach->GetContent().SetContentParameter(cMIMEParameter[eMethod], mime_method);
	if (!comp->GetMimeComponentName().empty())
		attach->GetContent().SetContentParameter(cMIMEParameter[eComponent], comp->GetMimeComponentName());
}

void CITIPProcessor::ProcessAttachment(const char* data, CMessage* msg)
{
	// Create input stream to parse from
	std::istrstream sin(data);

	// Parse as a calendar - fail if nothing found
	CICalendar cal;
	if (!cal.Parse(sin))
		return;
	
	// Get tied identity
	const CIdentity* tied_identity = msg->GetMbox()->GetTiedIdentity();
	if (tied_identity == NULL)
		tied_identity = &msg->GetMbox()->GetProtocol()->GetMailAccount()->GetAccountIdentity();

	// Now look for iTIP method
	cdstring method = cal.GetMethod();
	
	// Dispatch based on method
	if (method.empty() || (::strcmpnocase(method, cICalMethod_PUBLISH) == 0))
	{
		ReceivePublish(cal);
	}
	else if (::strcmpnocase(method, cICalMIMEMethod_REQUEST) == 0)
	{
		ReceiveRequest(cal, NULL, tied_identity, msg);
	}
	else if (::strcmpnocase(method, cICalMIMEMethod_REFRESH) == 0)
	{
		ReceiveRefresh(cal, NULL, tied_identity, msg);
	}
	else if (::strcmpnocase(method, cICalMIMEMethod_CANCEL) == 0)
	{
		ReceiveCancel(cal, NULL, tied_identity, msg);
	}
	else if (::strcmpnocase(method, cICalMIMEMethod_ADD) == 0)
	{
		ReceiveAdd(cal, NULL, tied_identity, msg);
	}
	else if (::strcmpnocase(method, cICalMIMEMethod_REPLY) == 0)
	{
		ReceiveReply(cal, NULL, tied_identity, msg);
	}
	else if (::strcmpnocase(method, cICalMIMEMethod_COUNTER) == 0)
	{
		ReceiveCounter(cal, NULL, tied_identity, msg);
	}
	else if (::strcmpnocase(method, cICalMIMEMethod_DECLINECOUNTER) == 0)
	{
		ReceiveDeclineCounter(cal, NULL, tied_identity, msg);
	}
	else
	{
		ReceivePublish(cal);
	}
}

void CITIPProcessor::ProcessCalDAVComponent(CICalendarComponent* comp)
{
	// Create a calendar to hold the component
	CICalendar cal;
	
	// Add the METHOD property copied from X-METHOD
	CICalendarProperty method(comp->GetProperties().find(cICalProperty_X_PRIVATE_METHOD)->second);
	method.SetName(cICalProperty_METHOD);
	cal.AddProperty(method);
	
	// Copy the component and process for REQUEST method
	CICalendarComponent* copy = comp->clone();
	copy->SetCalendar(cal.GetRef());
	copy->RemoveProperties(cICalProperty_X_PRIVATE_METHOD);
	
	// If the component is on anb existing calendar then we are sending an update to an existing event, rather than processing
	// a  new one. To make sure out processing logic works we need to bump the DTSTAMP on the copy so that it is greater than
	// the one for the existing component. That will ensure we overwrite the existing one.
	copy->InitDTSTAMP();
	
	// Now store in the calendar we are goint to process
	cal.AddComponent(copy);
	
	const CICalendarRef calref = comp->GetCalendar();
	const CICalendar* calp = CICalendar::GetICalendar(calref);
	const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(calp);
	const calstore::CCalendarProtocol* proto = node->GetProtocol();
	const CIdentity* tied_identity = calstore::CCalendarStoreManager::sCalendarStoreManager->GetTiedIdentity(calp);

	bool processed = false;

	// Now look for iTIP method
	cdstring methodtype = cal.GetMethod();
	
	// Dispatch based on method
	if (methodtype.empty() || (::strcmpnocase(methodtype, cICalMethod_PUBLISH) == 0))
	{
		processed = ReceivePublish(cal);
	}
	else if (::strcmpnocase(methodtype, cICalMIMEMethod_REQUEST) == 0)
	{
		processed = ReceiveRequest(cal, proto, tied_identity, NULL);
	}
	else if (::strcmpnocase(methodtype, cICalMIMEMethod_REFRESH) == 0)
	{
		processed = ReceiveRefresh(cal, proto, tied_identity, NULL);
	}
	else if (::strcmpnocase(methodtype, cICalMIMEMethod_CANCEL) == 0)
	{
		processed = ReceiveCancel(cal, proto, tied_identity, NULL);
	}
	else if (::strcmpnocase(methodtype, cICalMIMEMethod_ADD) == 0)
	{
		processed = ReceiveAdd(cal, proto, tied_identity, NULL);
	}
	else if (::strcmpnocase(methodtype, cICalMIMEMethod_REPLY) == 0)
	{
		processed = ReceiveReply(cal, proto, tied_identity, NULL);
	}
	else if (::strcmpnocase(methodtype, cICalMIMEMethod_COUNTER) == 0)
	{
		processed = ReceiveCounter(cal, proto, tied_identity, NULL);
	}
	else if (::strcmpnocase(methodtype, cICalMIMEMethod_DECLINECOUNTER) == 0)
	{
		processed = ReceiveDeclineCounter(cal, proto, tied_identity, NULL);
	}
	else
	{
		processed = ReceivePublish(cal);
	}
	
	// Delete from Inbox if processed
	if (processed && node->IsInbox())
	{
		if (dynamic_cast<CICalendarVEvent*>(comp) != NULL)
			const_cast<CICalendar*>(calp)->RemoveVEvent(static_cast<CICalendarVEvent*>(comp));
		else if (dynamic_cast<CICalendarVToDo*>(comp) != NULL)
			const_cast<CICalendar*>(calp)->RemoveVToDo(static_cast<CICalendarVToDo*>(comp));
	}
}

bool CITIPProcessor::NeedsITIPRequest(const CICalendarComponent& comp)
{
	// Look for ATTENDEES
	const CICalendarPropertyMap& props = comp.GetProperties();
	std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> result = props.equal_range(cICalProperty_ATTENDEE);
	for(CICalendarPropertyMap::const_iterator iter = result.first; iter != result.second; iter++)
	{
		const CICalendarProperty& prop = (*iter).second;

		// Must have PARTSTAT=NEEDS-ACTION (default without PARTSTAT is NEEDS-ACTION)
		if (prop.HasAttribute(cICalAttribute_PARTSTAT) &&
				(prop.GetAttributeValue(cICalAttribute_PARTSTAT).compare(cICalAttribute_PARTSTAT_NEEDSACTION, true) != 0))
			continue;

		// Must have RSVP=TRUE (default without RSVP is FALSE)
		if (!prop.HasAttribute(cICalAttribute_RSVP) ||
				(prop.GetAttributeValue(cICalAttribute_RSVP).compare(cICalAttribute_RSVP_TRUE, true) != 0))
			continue;

		// Check to see whether X-MULBERRY-NEEDS-ITIP=TRUE is present
		if (prop.HasAttribute(cICalAttribute_ATTENDEE_X_NEEDS_ITIP) &&
			(prop.GetAttributeValue(cICalAttribute_ATTENDEE_X_NEEDS_ITIP).compare(cICalAttribute_RSVP_TRUE, true) == 0))
			return true;
	}
	
	return false;
}

void CITIPProcessor::GetRequestDetails(const CICalendarComponent& comp, CCalendarAddressList& addrs, cdstring& id)
{
	// Look for ORGANISER
	const CIdentity* identity = OrganiserIdentity(comp);
	if (identity != NULL)
		id = identity->GetIdentity();

	// Look for ATTENDEES
	const CICalendarPropertyMap& props = comp.GetProperties();
	std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> result = props.equal_range(cICalProperty_ATTENDEE);
	for(CICalendarPropertyMap::const_iterator iter = result.first; iter != result.second; iter++)
	{
		// Check to see whether X-MULBERRY-NEEDS-ITIP is present
		const CICalendarProperty& prop = (*iter).second;
		if (prop.HasAttribute(cICalAttribute_ATTENDEE_X_NEEDS_ITIP))
		{
			// Look for those that need the iTIP request
			const cdstring& itip = prop.GetAttributeValue(cICalAttribute_ATTENDEE_X_NEEDS_ITIP);
			if (itip.compare(cICalAttribute_RSVP_TRUE, true) == 0)
			{
				const CICalendarCalAddressValue* value = prop.GetCalAddressValue();
				const cdstring& uri = value->GetValue();
				cdstring name;
				
				// Look for CN= and add to address
				if (prop.HasAttribute(cICalAttribute_CN))
				{
					name = prop.GetAttributeValue(cICalAttribute_CN);
				}
				addrs.push_back(new CCalendarAddress(uri, name));
			}
		}
	}
}

void CITIPProcessor::ClearITIPRequest(CICalendarComponent& comp)
{
	// Look for ATTENDEES
	const CICalendarPropertyMap& props = comp.GetProperties();
	std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> result = props.equal_range(cICalProperty_ATTENDEE);
	for(CICalendarPropertyMap::const_iterator iter = result.first; iter != result.second; iter++)
	{
		// Check to see whether X-MULBERRY-NEEDS-ITIP is present
		const CICalendarProperty& prop = (*iter).second;
		if (prop.HasAttribute(cICalAttribute_ATTENDEE_X_NEEDS_ITIP))
		{
			// Look for those that need the iTIP request
			const cdstring& itip = prop.GetAttributeValue(cICalAttribute_ATTENDEE_X_NEEDS_ITIP);
			if (itip.compare(cICalAttribute_RSVP_TRUE, true) == 0)
			{
				// Remove the attribute
				CICalendarAttribute& attr_mod = const_cast<CICalendarAttribute&>((*prop.GetAttributes().find(cICalAttribute_ATTENDEE_X_NEEDS_ITIP)).second);
				attr_mod.GetValues().clear();
				attr_mod.AddValue(cICalAttribute_RSVP_FALSE);
			}
		}
	}
}

bool CITIPProcessor::ReceivePublish(const CICalendar& cal)
{
	bool result = false;

	// IMPORTANT
	// We really need to check that the ORGANISER in the request matches any out-of-band sender
	// information (e.g. email From address, SMIME/PGP users id etc. calendar user address). This
	// is needed to prevent spoofing.

	// Check for no calendars
	if (calstore::CCalendarStoreManager::sCalendarStoreManager->GetReceivableCalendars().size() == 0)
	{
		CErrorHandler::PutStopAlertRsrc("CITIPProcessor::NoCalendars");
		return result;
	}

	// Must have something useful
	if (cal.GetVEvents().empty())
	{
		CErrorHandler::PutStopAlertRsrc("CITIPProcessor::NoData");
		return result;
	}

	// Process each component
	uint32_t updated_seq = 0;
	uint32_t ignored_seq = 0;
	bool update_always = false;
	CICalendar* caladd = NULL;
	for(CICalendarComponentDB::const_iterator iter = cal.GetVEvents().begin(); iter != cal.GetVEvents().end(); iter++)
	{
		// Get the real component
		const CICalendarVEvent* vevent = static_cast<CICalendarVEvent*>((*iter).second);
		
		// First see if this component already exists
		CICalendarComponent* found;
		EFoundComponent find = LookForComponentInCalendars(vevent, found);
		
		// Check for an exact (UID & RID) match
		if (find == eFoundExact)
		{
			CICalendarVEvent* fevent = dynamic_cast<CICalendarVEvent*>(found);

			// Is the new one an update
			if ((fevent != NULL) && ((Compare(fevent, vevent) & (eSEQOlder | eDTSTAMPOlder)) != 0))
			{
				CICalendarRef calref = found->GetCalendar();
				CICalendar* calcopy = CICalendar::GetICalendar(calref);

				// Ask the user whether they want to accept it
				bool change_it = true;
				if (!update_always)
				{
					cdstring calname;
					if (calcopy != NULL)
						calname = calcopy->GetName();
					
					cdstring summary = fevent->GetSummary();

					cdstring temp;
					temp.FromResource("CITIPProcessor::ChangedEvent");
					size_t slen = temp.length() + summary.length() + calname.length() + 1;
					cdstring txt;
					txt.reserve(slen);
					::snprintf(txt.c_str_mod(), slen, temp.c_str(), summary.c_str(), calname.c_str());

					if (cal.GetVEvents().size() > 1)
						change_it = (CErrorHandler::PutCautionAlert(true, txt, "CITIPProcessor::ChangedEventDontShow", &update_always) == CErrorHandler::Ok);
					else
						change_it = (CErrorHandler::PutCautionAlert(true, txt) == CErrorHandler::Ok);
					
				}
				
				// Change if requested
				if (change_it)
				{
					// Remove old one
					calcopy->RemoveVEvent(fevent);

					// Copy one from iTIP
					CICalendarComponent* new_comp = vevent->clone();
					new_comp->SetCalendar(calcopy->GetRef());
					calcopy->AddComponent(new_comp);
				}

				updated_seq++;
			}
			else
			{
				// We ignore this component as its old or the same as one we already have
				
				ignored_seq++;
			}
			result = true;
		}

		// Check for an partial (UID only) match
		else if (find == eFoundMaster)
		{
			// A partial match means we are getting an update to an instance of the master
		}

		// No match to existing items
		else
		{
			// Have a new component so allow user to pick the calendar to add it to
			if (caladd == NULL)
				caladd = calstore::CCalendarStoreManager::sCalendarStoreManager->PickCalendar(vevent);
			
			if (caladd)
			{
				// Duplicate the event and add to new calendar
				CICalendarVEvent* copy = new CICalendarVEvent(*vevent);
				copy->SetCalendar(caladd->GetRef());
				caladd->AddNewVEvent(copy, true);
				result = true;
			}
		}
	}
	
	if (ignored_seq > 0)
	{
		cdstring num(ignored_seq);
		CErrorHandler::PutNoteAlertRsrcStr("CITIPProcessor::IgnoredItems", num);
	}

	return result;
}

bool CITIPProcessor::ReceiveRequest(const CICalendar& cal, const calstore::CCalendarProtocol* proto, const CIdentity* tied_identity, CMessage* msg)
{
	bool result = false;

	// IMPORTANT
	// We really need to check that the ORGANISER in the request matches any out-of-band sender
	// information (e.g. email From address, SMIME/PGP users id etc. calendar user address). This
	// is needed to prevent spoofing.

	// Check for no calendars
	if (calstore::CCalendarStoreManager::sCalendarStoreManager->GetReceivableCalendars().size() == 0)
	{
		CErrorHandler::PutStopAlertRsrc("CITIPProcessor::NoCalendars");
		return result;
	}

	// Must have something useful
	if (cal.GetVEvents().empty())
	{
		CErrorHandler::PutStopAlertRsrc("CITIPProcessor::NoData");
		return result;
	}

	// Process each component
	uint32_t updated_seq = 0;
	uint32_t ignored_seq = 0;
	bool update_always = false;
	for(CICalendarComponentDB::const_iterator iter = cal.GetVEvents().begin(); iter != cal.GetVEvents().end(); iter++)
	{
		// Get the real component
		const CICalendarVEvent* vevent = static_cast<CICalendarVEvent*>((*iter).second);
		
		// First see if this component already exists
		CICalendarComponent* found = NULL;
		EFoundComponent find = LookForComponentInCalendars(vevent, found);
		
		// Check for an exact (UID & RID) match
		if (find == eFoundExact)
		{
			CICalendarVEvent* fevent = dynamic_cast<CICalendarVEvent*>(found);

			// Is the new one an update
			if ((fevent != NULL) && ((Compare(fevent, vevent) & (eSEQOlder | eDTSTAMPOlder)) != 0))
			{
				CICalendarRef calref = found->GetCalendar();
				CICalendar* calcopy = CICalendar::GetICalendar(calref);

				// Ask the user whether they want to accept it
				bool change_it = true;
				
				// See if the new one is a reschedule as we may need to reprocess it for timing
				if (!CheckForReschedule(fevent, vevent))
				{
					// The iTIP component is a simple update to the existing one,
					// so replace the old with the new and we are done

					// Remove old one
					calcopy->RemoveVEvent(fevent);

					// Copy one from iTIP
					CICalendarComponent* new_comp = vevent->clone();
					new_comp->SetCalendar(calcopy->GetRef());
					calcopy->AddComponent(new_comp);
					
					continue;
				}
				else
				{
					// Remove the old event as the organiser is trying to replace it.
					// NB it is important to remove it even if the user rejects the new request,
					// as the old event has been changed

					// Remove old one
					calcopy->RemoveVEvent(fevent);
				}

				updated_seq++;
				result = true;
			}
			else
			{
				// We ignore this component as its old or the same as one we already have
				
				ignored_seq++;
				result = true;
				continue;
			}
		}
		// Check for an partial (UID only) match
		else if (find == eFoundMaster)
		{
			// A partial match means we are getting an update to an instance of the master
		}

		// Now need to determine which ATTENDEE in the component matches the current user,
		// based on identity.
		const CIdentity* id = tied_identity;

		// Check attendee properties
		CICalendarProperty attendee;
		bool has_attendee = false;
#ifdef ALWAYS_RSVP
		bool rsvp = true;
#else
		bool rsvp = false;
#endif
		if (id != NULL)
		{
			const CICalendarPropertyMap& props = vevent->GetProperties();
			std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> attendees = props.equal_range(cICalProperty_ATTENDEE);
			for(CICalendarPropertyMap::const_iterator iter = attendees.first; iter != attendees.second; iter++)
			{
				// See if attendee maps to an identity
				const CIdentity* attendee_id = NULL;
				if (AttendeeIdentity((*iter).second, attendee_id) && (attendee_id == id))
				{
					// RSVP may be needed
					if ((*iter).second.HasAttribute(cICalAttribute_RSVP))
					{
						const cdstring& partstat = (*iter).second.GetAttributeValue(cICalAttribute_RSVP);
						rsvp = (partstat.compare(cICalAttribute_RSVP_TRUE, true) == 0);
					}

					has_attendee = true;
					attendee = (*iter).second;
					break;
				}
			}
		}
		else
		{
			const CICalendarPropertyMap& props = vevent->GetProperties();
			std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> attendees = props.equal_range(cICalProperty_ATTENDEE);
			for(CICalendarPropertyMap::const_iterator iter = attendees.first; iter != attendees.second; iter++)
			{
				// See if attendee maps to an identity
				if (AttendeeIdentity((*iter).second, id))
				{
					// The PARTSTAT must be NEEDS-ACTION (or not present)
					if ((*iter).second.HasAttribute(cICalAttribute_PARTSTAT))
					{
						const cdstring& partstat = (*iter).second.GetAttributeValue(cICalAttribute_PARTSTAT);
						if (partstat.compare(cICalAttribute_PARTSTAT_NEEDSACTION, true) != 0)
							continue;
					}
					
					// RSVP may be needed
					if ((*iter).second.HasAttribute(cICalAttribute_RSVP))
					{
						const cdstring& partstat = (*iter).second.GetAttributeValue(cICalAttribute_RSVP);
						rsvp = (partstat.compare(cICalAttribute_RSVP_TRUE, true) == 0);
					}

					has_attendee = true;
					attendee = (*iter).second;
					break;
				}
			}
		}
		
		// If we get here we have a new or an update event. We need to present the iTIP
		// dialog to the user. The dialog will prompt the user to take appropriate action, based
		// one what is known about the event. To help the user we first determine whether the request
		// conflicts with any events on active calendars.
		//
		// The user's options are accept the event or reject the event. We currently do not allow
		// the user to suggest an alternative time through iTIP.
		
		bool conflict = CheckForConflicts(vevent);

		// Prompt the user: there are several possible options:
		//
		// 1. conflict
		// 2. attendee
		// 3. RSVP
		//
		// Possible actions:
		//
		// A. Accept with RSVP (valid for attendee && rsvp)
		// B. Accept without RSVP (valid for all)
		// C. Reject (valid for attendee && rsvp)
		// D. Ignore (always valid)
		//
		// Possible titles:
		// 
		// E. No conflict
		// F. Has conflict
		//
		//	Combinations of buttons
		//
		// 	1	2	3		A	B	C	D	E	F
		//	f	f	f		f	t	f	t	t	f
		//	f	f	t		f	t	f	t	t	f
		//  f	t	f		f	t	f	t	t	f
		//  f	t	t		t	t	t	t	t	f
		//	t	f	f		f	t	f	t	f	t
		//	t	f	t		f	t	f	t	f	t
		//  t	t	f		f	t	f	t	f	t
		//  t	t	t		t	t	t	t	f	t
		//
		//	Allowed combinations
		//
		//	B & D & E
		//  A & B & C & D & E
		//	B & D & F
		//  A & B & C & D & F
		//  
		
		CErrorDialog::EDialogResult dresult = CErrorDialog::eBtn1;
		
		if (has_attendee && rsvp)
		{
			// A & B & C & D
			dresult = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
										"ErrorDialog::Btn::iTIPAcceptRSVP", 
										"ErrorDialog::Btn::iTIPAcceptNoRSVP", 
										"ErrorDialog::Btn::iTIPReject", 
										"ErrorDialog::Btn::iTIPIgnore", 
										conflict ? "ErrorDialog::Text::iTIPHandleRequestABCDF" : "ErrorDialog::Text::iTIPHandleRequestABCDE",
										4);
		}
		else
		{
			// B & D
			dresult = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
										"ErrorDialog::Btn::iTIPAccept", 
										"ErrorDialog::Btn::iTIPIgnore", 
										NULL, 
										NULL, 
										conflict ? "ErrorDialog::Text::iTIPHandleRequestBDF" : "ErrorDialog::Text::iTIPHandleRequestBDE",
										2);
			
			// Normalise result to ABCD choices
			if (dresult == CErrorDialog::eBtn1)
				dresult = CErrorDialog::eBtn2;
			else if (dresult == CErrorDialog::eBtn2)
				dresult = CErrorDialog::eBtn4;
		}
										
		if ((dresult == CErrorDialog::eBtn1) || (dresult == CErrorDialog::eBtn2))
		{
			// Change PARTSTAT for the component being added to the calendar
			if (has_attendee)
			{
				attendee.RemoveAttributes(cICalAttribute_PARTSTAT);
				attendee.AddAttribute(CICalendarAttribute(cICalAttribute_PARTSTAT, cICalAttribute_PARTSTAT_ACCEPTED));
			}
			
			CICalendar* caladd = NULL;

			// If its a replacement for an existing event, then use the calendar for the existing one
			if (found != NULL)
			{
				CICalendarRef calref = found->GetCalendar();
				caladd = CICalendar::GetICalendar(calref);
			}
			
			// Allow user to pick the calendar if not already set
			if (caladd == NULL)
				caladd = calstore::CCalendarStoreManager::sCalendarStoreManager->PickCalendar(vevent);
			
			if (caladd != NULL)
			{
				// Duplicate the event and add to new calendar
				CICalendarVEvent* copy = new CICalendarVEvent(*vevent);
				copy->SetCalendar(caladd->GetRef());
				caladd->AddNewVEvent(copy, true);
			}
			
			if (dresult == CErrorDialog::eBtn1)
			{
				// Send REPLY: accept
				attendee.RemoveAttributes(cICalAttribute_PARTSTAT);
				attendee.AddAttribute(CICalendarAttribute(cICalAttribute_PARTSTAT, cICalAttribute_PARTSTAT_ACCEPTED));

				if (msg != NULL)
					SendiMIPReply(vevent, "2.0;Success", attendee, id, msg, eReplyAccepted);
				else if (proto != NULL)
					SendCalDAVReply(vevent, "2.0;Success", attendee, id, proto, eReplyAccepted);
			}
			result = true;
		}
		else if (dresult == CErrorDialog::eBtn3)
		{
			// Send REPLY: decline
			attendee.RemoveAttributes(cICalAttribute_PARTSTAT);
			attendee.AddAttribute(CICalendarAttribute(cICalAttribute_PARTSTAT, cICalAttribute_PARTSTAT_DECLINED));
			
			
			if (msg != NULL)
				SendiMIPReply(vevent, "2.0;Success", attendee, id, msg, eReplyDeclined);
			else if (proto != NULL)
				SendCalDAVReply(vevent, "2.0;Success", attendee, id, proto, eReplyDeclined);
			result = true;
		}
		else
		{
			// Just ignore it!
		}
	}
	
	if (ignored_seq > 0)
	{
		cdstring num(ignored_seq);
		CErrorHandler::PutNoteAlertRsrcStr("CITIPProcessor::IgnoredItems", num);
	}
	
	return result;
}

bool CITIPProcessor::ReceiveRefresh(const CICalendar& cal, const calstore::CCalendarProtocol* proto, const CIdentity* tied_identity, CMessage* msg)
{
	return false;
}

bool CITIPProcessor::ReceiveCancel(const CICalendar& cal, const calstore::CCalendarProtocol* proto, const CIdentity* tied_identity, CMessage* msg)
{
	bool result = false;

	// IMPORTANT
	// We really need to check that the ORGANISER in the request matches any out-of-band sender
	// information (e.g. email From address, SMIME/PGP users id etc. calendar user address). This
	// is needed to prevent spoofing.

	// Check for no calendars
	if (calstore::CCalendarStoreManager::sCalendarStoreManager->GetReceivableCalendars().size() == 0)
	{
		CErrorHandler::PutStopAlertRsrc("CITIPProcessor::NoCalendars");
		return result;
	}

	// Must have something useful
	if (cal.GetVEvents().empty())
	{
		CErrorHandler::PutStopAlertRsrc("CITIPProcessor::NoData");
		return result;
	}

	// Process each component
	uint32_t updated_seq = 0;
	uint32_t ignored_seq = 0;
	bool update_always = false;
	for(CICalendarComponentDB::const_iterator iter = cal.GetVEvents().begin(); iter != cal.GetVEvents().end(); iter++)
	{
		// Get the real component
		const CICalendarVEvent* vevent = static_cast<CICalendarVEvent*>((*iter).second);
		
		// Does it have a recurrence ID
		bool cancel_master = vevent->IsRecurrenceInstance();
		
		// First see if this component already exists
		CICalendarComponent* found = NULL;
		EFoundComponent find = LookForComponentInCalendars(vevent, found);
		
		// Check for an exact (UID & RID) match
		if (find == eFoundExact)
		{
			CICalendarVEvent* fevent = dynamic_cast<CICalendarVEvent*>(found);

			// Is the new one an update
			if ((fevent != NULL) && ((Compare(fevent, vevent) & (eSEQOlder | eDTSTAMPOlder)) != 0))
			{
				CICalendarRef calref = found->GetCalendar();
				CICalendar* calcopy = CICalendar::GetICalendar(calref);

				{
					// See if we are cancelling the entire set or just one instance
					if (vevent->IsRecurrenceInstance())
					{
						// Remove all events with this UID
						calcopy->RemoveVEvent(vevent->GetUID());
					}
					else
					{
						// EXDATE the master and remove matching instance
						CICalendarVEvent* master = (CICalendarVEvent*)fevent->GetMaster();
						if (master)
							master->ExcludeRecurrence(vevent->GetRecurrenceID());
						calcopy->RemoveVEvent(fevent);
					}
				}

				updated_seq++;
				result = true;
			}
			else
			{
				// We ignore this component as its old or the same as one we already have
				
				ignored_seq++;
				result = true;
				continue;
			}
		}
		// Check for an partial (UID only) match
		else if (find == eFoundMaster)
		{
			// Cancel the entire event
			CICalendarVEvent* fevent = dynamic_cast<CICalendarVEvent*>(found);

			// Is the new one an update
			if ((fevent != NULL) && ((Compare(fevent, vevent) & (eSEQOlder | eDTSTAMPOlder)) != 0))
			{
				// Remove instance from master
				fevent->ExcludeRecurrence(vevent->GetRecurrenceID());
				result = true;
			}
			else
			{
				// We ignore this component as its old or the same as one we already have
				
				ignored_seq++;
				result = true;
				continue;
			}
		}
		else
		{
			// We ignore this component as we have nothing to cancel
			
			ignored_seq++;
			result = true;
			continue;
		}
	}
	
	if (ignored_seq > 0)
	{
		cdstring num(ignored_seq);
		CErrorHandler::PutNoteAlertRsrcStr("CITIPProcessor::IgnoredItems", num);
	}
	
	return result;
}

bool CITIPProcessor::ReceiveAdd(const CICalendar& cal, const calstore::CCalendarProtocol* proto, const CIdentity* tied_identity, CMessage* msg)
{
	return false;
}

bool CITIPProcessor::ReceiveReply(const CICalendar& cal, const calstore::CCalendarProtocol* proto, const CIdentity* tied_identity, CMessage* msg)
{
	bool result = false;

	// IMPORTANT
	// We really need to check that the ORGANISER in the request matches any out-of-band sender
	// information (e.g. email From address, SMIME/PGP users id etc. calendar user address). This
	// is needed to prevent spoofing.

	// Check for no calendars
	if (calstore::CCalendarStoreManager::sCalendarStoreManager->GetReceivableCalendars().size() == 0)
	{
		CErrorHandler::PutStopAlertRsrc("CITIPProcessor::NoCalendars");
		return result;
	}

	// Must have something useful
	if (cal.GetVEvents().empty())
	{
		CErrorHandler::PutStopAlertRsrc("CITIPProcessor::NoData");
		return result;
	}

	// Process each component
	uint32_t ignored_seq = 0;
	bool update_always = false;
	for(CICalendarComponentDB::const_iterator iter = cal.GetVEvents().begin(); iter != cal.GetVEvents().end(); iter++)
	{
		// Get the real component
		const CICalendarVEvent* vevent = static_cast<CICalendarVEvent*>((*iter).second);
		CICalendarComponent* found;
		EFoundComponent find = LookForComponentInCalendars(vevent, found);
		
		// Check for an exact (UID & RID) match
		if (find == eFoundExact)
		{
			CICalendarVEvent* fevent = dynamic_cast<CICalendarVEvent*>(found);

			// Is the reply in reply to the one we currently have.
			// Ultimately we need to track old SEQ's for proper REPLY behaviour
			if ((fevent != NULL) && ((Compare(fevent, vevent) & (eDTSTAMPOlder | eTheSame)) != 0))
			{
				CICalendarRef calref = found->GetCalendar();
				CICalendar* calcopy = CICalendar::GetICalendar(calref);

				// Determine response status
				cdstring request_status;
				if (vevent->GetProperty(cICalProperty_REQUEST_STATUS, request_status))
				{
					// Look at first digit
					request_status.trim();
					if (request_status[0UL] != '2')
					{
						CErrorHandler::PutStopAlertRsrcTxt("CITIPProcessor::RequestStatusError", request_status);
						continue;
					}
				}
				
				// Make sure organiser matches one in original request
				CCalendarAddress org_old;
				CCalendarAddress org_new;
				if (!GetOrganiserAddress(fevent, org_old) || !GetOrganiserAddress(vevent, org_new) || !(org_old == org_new))
				{
					CErrorHandler::PutStopAlertRsrcTxt("CITIPProcessor::OrganiserChangedError", request_status);
					continue;
				}
				
				// Check for single attendee
				if (vevent->CountProperty(cICalProperty_ATTENDEE) != 1)
				{
					CErrorHandler::PutStopAlertRsrcTxt("CITIPProcessor::AttendeeCountError", request_status);
					continue;
				}
				
				// Make sure attendee is one of the ones in the original request.
				// At some point we need to take care of delegation.
				const CICalendarProperty& attendee_new = vevent->GetProperties().find(cICalProperty_ATTENDEE)->second;
				CICalendarProperty* attendee_old = NULL;
				if (!GetMatchingAttendee(fevent, attendee_new, attendee_old))
				{
					CErrorHandler::PutStopAlertRsrcTxt("CITIPProcessor::AttendeeMismatchError", request_status);
					continue;
				}
				
				// Use CN= attribute if present, otherwise cal-address
				cdstring attendee_name;
				if (attendee_old->HasAttribute(cICalAttribute_CN))
				{
					attendee_name = attendee_old->GetAttributeValue(cICalAttribute_CN);
				}
				else if (attendee_new.HasAttribute(cICalAttribute_CN))
				{
					attendee_name = attendee_new.GetAttributeValue(cICalAttribute_CN);
				}
				else
				{
					attendee_name = attendee_old->GetCalAddressValue()->GetValue();
				}

				// Get partstats
				cdstring partstat_old = cICalAttribute_PARTSTAT_NEEDSACTION;
				cdstring partstat_new = cICalAttribute_PARTSTAT_NEEDSACTION;

				if (attendee_old->HasAttribute(cICalAttribute_PARTSTAT))
				{
					partstat_old = attendee_old->GetAttributeValue(cICalAttribute_PARTSTAT);
				}
				if (attendee_new.HasAttribute(cICalAttribute_PARTSTAT))
				{
					partstat_new = attendee_new.GetAttributeValue(cICalAttribute_PARTSTAT);
				}
				
				// Check for a change and apply the change to the existing component
				if (partstat_old != partstat_new)
				{
					// Compose note string (before changing values)
					cdstring partstatold_desc = GetAttendeeStatusDescriptor(*attendee_old);
					cdstring partstatnew_desc = GetAttendeeStatusDescriptor(attendee_new);
					cdstring note;
					note.FromResource("CITIPProcessor::AttendeeStatusChange");

					cdstring txt;
					size_t txt_reserve = note.length() + partstatold_desc.length() + partstatnew_desc.length() + 1;
					txt.reserve(txt_reserve);
					::snprintf(txt.c_str_mod(), txt_reserve, note.c_str(), partstatold_desc.c_str(), partstatnew_desc.c_str());

					// Change status of component on the calendar
					attendee_old->RemoveAttributes(cICalAttribute_PARTSTAT);
					attendee_old->RemoveAttributes(cICalAttribute_RSVP);
					attendee_old->RemoveAttributes(cICalAttribute_ATTENDEE_X_NEEDS_ITIP);
					attendee_old->AddAttribute(CICalendarAttribute(cICalAttribute_PARTSTAT, partstat_new));
					calcopy->ChangedComponent(found);

					// Display something to the user
					CErrorHandler::PutNoteAlertTxt(txt, attendee_name);
				}
				else
				{
					// Display something to the user
					CErrorHandler::PutNoteAlertRsrcTxt("CITIPProcessor::AttendeeNoStatusChange", attendee_name);
				}
				result = true;
			}
			else
			{
				// We ignore this component as its old or the same as one we already have
				
				ignored_seq++;
				result = true;
				continue;
			}
		}
		
		// If not an exact match or no match
		else
		{
			// We ignore this component as it cannot be matched
			ignored_seq++;
			result = true;
			continue;
		}
	}

	if (ignored_seq > 0)
	{
		cdstring num(ignored_seq);
		CErrorHandler::PutNoteAlertRsrcStr("CITIPProcessor::IgnoredItems", num);
	}
	
	return result;
}

bool CITIPProcessor::ReceiveCounter(const CICalendar& cal, const calstore::CCalendarProtocol* proto, const CIdentity* tied_identity, CMessage* msg)
{
	return false;
}

bool CITIPProcessor::ReceiveDeclineCounter(const CICalendar& cal, const calstore::CCalendarProtocol* proto, const CIdentity* tied_identity, CMessage* msg)
{
	return false;
}

CITIPProcessor::EFoundComponent CITIPProcessor::LookForComponentInCalendars(const CICalendarComponent* orig, CICalendarComponent*& found)
{
	// Scan active calendars that can receive new items
	
	const CICalendarList& callist = calstore::CCalendarStoreManager::sCalendarStoreManager->GetReceivableCalendars();
	for(CICalendarList::const_iterator iter = callist.begin(); iter != callist.end(); iter++)
	{
		// First look for exact match to UID & RID
		const CICalendarComponent* find = (*iter)->FindComponent(orig, CICalendar::eFindExact);
		if (find)
		{
			found = const_cast<CICalendarComponent*>(find);
			return eFoundExact;
		}
		
		// Now look for UID (master) match only
		find = (*iter)->FindComponent(orig, CICalendar::eFindMaster);
		if (find)
		{
			found = const_cast<CICalendarComponent*>(find);
			return eFoundMaster;
		}
	}
	
	return eNotFound;
}

CITIPProcessor::ECompareComponents CITIPProcessor::Compare(const CICalendarVEvent* comp_old, const CICalendarVEvent* comp_new)
{
	// We assume that the UIDs & RIDs already match
	
	// Compare the sequence
	if (comp_old->GetSeq() != comp_new->GetSeq())
	{
		return (comp_old->GetSeq() > comp_new->GetSeq()) ? eSEQNewer : eSEQOlder;
	}
	else if (comp_old->HasStamp() && comp_new->HasStamp())
	{
		if (comp_old->GetStamp() != comp_new->GetStamp())
			return (comp_old->GetStamp() > comp_new->GetStamp()) ? eDTSTAMPNewer : eDTSTAMPOlder;
		else
			return eTheSame;
	}
	else if (comp_old->HasStamp())
		return eDTSTAMPNewer;
	else
		return eDTSTAMPOlder;
}

bool CITIPProcessor::CheckForReschedule(const CICalendarVEvent* comp_old, const CICalendarVEvent* comp_new)
{
	// Compare time-based properties
	
	// DTSTART
	if (comp_old->GetStart() != comp_new->GetStart())
		return false;
	
	// DTEND/DURATION
	if (comp_old->GetEnd() != comp_new->GetEnd())
		return false;
	
	// Check recurrence
	if (comp_old->IsRecurring() ^ comp_new->IsRecurring())
		return false;
	
	// If no recurrence then we match
	else if (!comp_old->IsRecurring())
		return true;
	
	// Compare recurrences
	if (!comp_old->GetRecurrenceSet()->Equals(comp_new->GetRecurrenceSet()))
		return false;
	
	// Compare instance
	if (comp_old->IsRecurrenceInstance() ^ comp_new->IsRecurrenceInstance())
		return false;
	else if (comp_old->IsRecurrenceInstance() && (comp_old->GetRecurrenceID() != comp_new->GetRecurrenceID()))
		return false;
	
	return true;
}

bool CITIPProcessor::CheckForConflicts(const CICalendarVEvent* comp)
{
	// Get a range of periods from the comp that correspond to all instances,
	// or in the case of an unbounded recurrence, all instances up to one year after
	// the initial instance of 'now' whichever is later.
	
	CICalendarPeriodList busy;
	DetermineITIPBusyPeriods(comp, busy);
	
	// Now see if there is any overlap with active calendars
	const CICalendarList& callist = calstore::CCalendarStoreManager::sCalendarStoreManager->GetReceivableCalendars();
	for(CICalendarList::const_iterator iter1 = callist.begin(); iter1 != callist.end(); iter1++)
	{
		// Check busy for each period - if any items are in the list, then we have a conflict
		for(CICalendarPeriodList::const_iterator iter2 = busy.begin(); iter2 != busy.end(); iter2++)
		{
			CICalendarFreeBusyList busy;
			(*iter1)->GetFreeBusy(*iter2, busy);
			if (busy.size() != 0)
				return true;
		}
	}
	
	// No conflict if we get here
	return false;
}

void CITIPProcessor::DetermineITIPBusyPeriods(const CICalendarVEvent* comp, CICalendarPeriodList& busy)
{
	// Get a range of periods from the comp that correspond to all instances,
	// or in the case of an unbounded recurrence, all instances up to one year after
	// the initial instance of 'now' whichever is later.
	
	// Check recurrence state
	if (comp->IsRecurring())
	{
		
	}
	else
	{
		// Simple non-recurring item
		busy.push_back(CICalendarPeriod(comp->GetStart(), comp->GetEnd()));
	}
}

bool CITIPProcessor::OrganiserIsMe(const CICalendarComponent& comp)
{
	return OrganiserIdentity(comp) != NULL;
}

const CIdentity* CITIPProcessor::OrganiserIdentity(const CICalendarComponent& comp)
{
	// Try tied identity first
//	const CICalendarRef calref = comp.GetCalendar();
//	const CICalendar* calp = CICalendar::GetICalendar(calref);
//	const CIdentity* tied_identity = calstore::CCalendarStoreManager::sCalendarStoreManager->GetTiedIdentity(calp);
//	if (tied_identity)
//		return tied_identity;

	// Fall back to looking for a match to the ORGANIZER address
	CCalendarAddress caddr;
	if (GetOrganiserAddress(&comp, caddr))
	{
		return CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(caddr);
	}
	else
		return NULL;
}

const CICalendarProperty* CITIPProcessor::AttendeeMe(const CICalendarComponent& comp)
{
	// First check if we are the orgnaizer
	if (CITIPProcessor::OrganiserIsMe(comp))
	{
		// Fall back to looking for a match to the ORGANIZER address
		CCalendarAddress orgcaddr;
		if (GetOrganiserAddress(&comp, orgcaddr))
		{
			// Scan attendees in the component looking for one that matches the Organizer
			const CICalendarPropertyMap& props = comp.GetProperties();
			std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> attendees = props.equal_range(cICalProperty_ATTENDEE);
			for(CICalendarPropertyMap::const_iterator iter = attendees.first; iter != attendees.second; iter++)
			{
				const CICalendarCalAddressValue* value = (*iter).second.GetCalAddressValue();
				CCalendarAddress attendeeaddr(value->GetValue());
				if (attendeeaddr == orgcaddr)
					return &((*iter).second);
			}
		}
	}

	// Scan attendees in the component looking for one that matches the Organizer
	const CICalendarPropertyMap& props = comp.GetProperties();
	std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> attendees = props.equal_range(cICalProperty_ATTENDEE);
	for(CICalendarPropertyMap::const_iterator iter = attendees.first; iter != attendees.second; iter++)
	{
		const CIdentity* id = NULL;
		if (AttendeeIdentity((*iter).second, id))
		{
			return &(*iter).second;
		}
	}
	
	return NULL;
}

CITIPProcessor::EAttendeeState CITIPProcessor::GetAttendeeState(const CICalendarComponent& comp)
{
	// Check organiser state
	EAttendeeState result = eNone;
	if ((comp.CountProperty(cICalProperty_ORGANIZER) == 0) ||
		(comp.CountProperty(cICalProperty_ATTENDEE) == 0))
		return result;

	if (OrganiserIsMe(comp))
	{
		// Scan attendees in the component looking for PARTSTAT values
		result = eAllAccepted;
		const CICalendarPropertyMap& props = comp.GetProperties();
		std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> attendees = props.equal_range(cICalProperty_ATTENDEE);
		for(CICalendarPropertyMap::const_iterator iter = attendees.first; iter != attendees.second; iter++)
		{
			if ((*iter).second.HasAttribute(cICalAttribute_PARTSTAT))
			{
				const cdstring& partstat = (*iter).second.GetAttributeValue(cICalAttribute_PARTSTAT);
				if (partstat.compare(cICalAttribute_PARTSTAT_NEEDSACTION, true) == 0)
					result = eSomeAccepted;
				else if (partstat.compare(cICalAttribute_PARTSTAT_DECLINED, true) == 0)
				{
					result = eSomeDeclined;
					break;
				}
				else if (partstat.compare(cICalAttribute_PARTSTAT_TENTATIVE, true) == 0)
					result = eSomeAccepted;
			}
			else
				// The default is NEEDS-ACTION
				result = eSomeAccepted;
		}
	}
	else
	{
		const CICalendarRef calref = comp.GetCalendar();
		const CICalendar* calp = CICalendar::GetICalendar(calref);
		const CIdentity* tied_identity = calstore::CCalendarStoreManager::sCalendarStoreManager->GetTiedIdentity(calp);
		const CICalendarProperty* attendee = NULL;
		GetAttendeeForIdentity(comp, tied_identity, attendee);
		if (attendee)
		{
			// Check PARTSTAT
			result = eIHaveNotAccepted;
			if (attendee->HasAttribute(cICalAttribute_PARTSTAT))
			{
				const cdstring& partstat = attendee->GetAttributeValue(cICalAttribute_PARTSTAT);
				if (partstat.compare(cICalAttribute_PARTSTAT_NEEDSACTION, true) != 0)
					result = eIHaveAccepted;
			}
		}
	}
	
	return result;
}

void CITIPProcessor::GetAttendeeForIdentity(const CICalendarComponent& comp, const CIdentity*& id, const CICalendarProperty*& attendee)
{
	if (id != NULL)
	{
		const CICalendarPropertyMap& props = comp.GetProperties();
		std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> attendees = props.equal_range(cICalProperty_ATTENDEE);
		for(CICalendarPropertyMap::const_iterator iter = attendees.first; iter != attendees.second; iter++)
		{
			// See if attendee maps to an identity
			const CIdentity* attendee_id = NULL;
			if (AttendeeIdentity((*iter).second, attendee_id) && (attendee_id == id))
			{
				attendee = &(*iter).second;
				return;
			}
		}
	}
	else
	{
		const CICalendarPropertyMap& props = comp.GetProperties();
		std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> attendees = props.equal_range(cICalProperty_ATTENDEE);
		for(CICalendarPropertyMap::const_iterator iter = attendees.first; iter != attendees.second; iter++)
		{
			// See if attendee maps to an identity
			if (AttendeeIdentity((*iter).second, id))
			{
				// The PARTSTAT must be NEEDS-ACTION (or not present)
				if ((*iter).second.HasAttribute(cICalAttribute_PARTSTAT))
				{
					const cdstring& partstat = (*iter).second.GetAttributeValue(cICalAttribute_PARTSTAT);
					if (partstat.compare(cICalAttribute_PARTSTAT_NEEDSACTION, true) != 0)
						continue;
				}
				
				attendee = &(*iter).second;
				return;
			}
		}
	}
	
	attendee = NULL;
	return;
}

bool CITIPProcessor::AttendeeIdentity(const CICalendarProperty& attendee, const CIdentity*& id)
{
	const CICalendarCalAddressValue* value = attendee.GetCalAddressValue();
	cdstring txt = value->GetValue();
	::strlower(txt.c_str_mod());
	CCalendarAddress caddr(txt, cdstring::null_str);
	id = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(caddr);
	return id != NULL;
}

bool CITIPProcessor::GetOrganiserAddress(const CICalendarComponent* comp, CCalendarAddress& caddr)
{
	// Look for ORGANISER
	cdstring org;
	if (comp->GetProperty(cICalProperty_ORGANIZER, org))
	{
		caddr.SetAddress(org);

		const CICalendarProperty& prop = comp->GetProperties().find(cICalProperty_ORGANIZER)->second;
		if (prop.HasAttribute(cICalAttribute_CN))
		{
			cdstring name = prop.GetAttributeValue(cICalAttribute_CN);
			caddr.SetName(name);
		}
		
		return true;
	}
	
	return false;
}

bool CITIPProcessor::GetMatchingAttendee(CICalendarComponent* comp, const CICalendarProperty& attendee, CICalendarProperty*& found)
{
	// Extract caladdress from attendee to test
	const CICalendarCalAddressValue* value1 = attendee.GetCalAddressValue();
	CURL caladdr1(value1->GetValue());
	
	// Scan attendees in the component looking for a match
	CICalendarPropertyMap& props = comp->GetProperties();
	std::pair<CICalendarPropertyMap::iterator, CICalendarPropertyMap::iterator> attendees = props.equal_range(cICalProperty_ATTENDEE);
	for(CICalendarPropertyMap::iterator iter = attendees.first; iter != attendees.second; iter++)
	{
		const CICalendarCalAddressValue* value2 = (*iter).second.GetCalAddressValue();
		CURL caladdr2(value2->GetValue());

		// See if they are the same
		if (caladdr1.Equal(caladdr2))
		{
			found = &(*iter).second;
			return true;
		}
	}
	
	return false;
}

cdstring CITIPProcessor::GetAttendeeDescriptor(const CICalendarProperty& attendee)
{
	cdstring result;
	
	// Use CN= attribute if present, otherwise cal-address
	if (attendee.HasAttribute(cICalAttribute_CN))
	{
		result = attendee.GetAttributeValue(cICalAttribute_CN);
	}
	
	if (result.empty())
	{
		result = "<";
		result += attendee.GetCalAddressValue()->GetValue();
		result += ">";
	}
	
	return result;
}

cdstring CITIPProcessor::GetAttendeeFullDescriptor(const CICalendarProperty& attendee)
{
	cdstring result;
	
	// Use CN= attribute if present, otherwise cal-address
	if (attendee.HasAttribute(cICalAttribute_CN))
	{
		result = attendee.GetAttributeValue(cICalAttribute_CN);
	}
	
	if (!result.empty())
		result += " ";
	result += "<";
	result += attendee.GetCalAddressValue()->GetValue();
	result += ">";
	
	return result;
}

cdstring CITIPProcessor::GetAttendeeRoleDescriptor(const CICalendarProperty& attendee)
{
	// Determine role
	cdstring result;
	if (attendee.HasAttribute(cICalAttribute_ROLE))
	{
		const cdstring& role = attendee.GetAttributeValue(cICalAttribute_ROLE);
		if (role.compare(cICalAttribute_ROLE_CHAIR, true) == 0)
			result.FromResource("CITIPProcessor::Role::Chair");
		else if (role.compare(cICalAttribute_ROLE_REQ_PART, true) == 0)
			result.FromResource("CITIPProcessor::Role::ReqPart");
		else if (role.compare(cICalAttribute_ROLE_OPT_PART, true) == 0)
			result.FromResource("CITIPProcessor::Role::OptPart");
		else if (role.compare(cICalAttribute_ROLE_NON_PART, true) == 0)
			result.FromResource("CITIPProcessor::Role::NonPart");
		else
			result.FromResource("CITIPProcessor::Role::Unknown");
	}
	else
		// 2445 default
		result.FromResource("CITIPProcessor::Role::ReqPart");
	
	return result;
}

cdstring CITIPProcessor::GetAttendeeStatusDescriptor(const CICalendarProperty& attendee)
{
	// Determine participation status
	cdstring result;
	if (attendee.HasAttribute(cICalAttribute_PARTSTAT))
	{
		const cdstring& partstat = attendee.GetAttributeValue(cICalAttribute_PARTSTAT);
		if (partstat.compare(cICalAttribute_PARTSTAT_NEEDSACTION, true) == 0)
			result.FromResource("CITIPProcessor::PartStat::NeedsAction");
		else if (partstat.compare(cICalAttribute_PARTSTAT_ACCEPTED, true) == 0)
			result.FromResource("CITIPProcessor::PartStat::Accepted");
		else if (partstat.compare(cICalAttribute_PARTSTAT_DECLINED, true) == 0)
			result.FromResource("CITIPProcessor::PartStat::Declined");
		else if (partstat.compare(cICalAttribute_PARTSTAT_TENTATIVE, true) == 0)
			result.FromResource("CITIPProcessor::PartStat::Tentative");
		else if (partstat.compare(cICalAttribute_PARTSTAT_DELEGATED, true) == 0)
			result.FromResource("CITIPProcessor::PartStat::Delegated");
		else if (partstat.compare(cICalAttribute_PARTSTAT_COMPLETED, true) == 0)
			result.FromResource("CITIPProcessor::PartStat::Complete");
		else if (partstat.compare(cICalAttribute_PARTSTAT_INPROCESS, true) == 0)
			result.FromResource("CITIPProcessor::PartStat::InProcess");
		else
			result.FromResource("CITIPProcessor::PartStat::Unknown");
	}
	else
		// 2445 default
		result.FromResource("CITIPProcessor::PartStat::NeedsAction");
	
	return result;
}
