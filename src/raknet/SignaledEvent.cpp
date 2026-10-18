#include "SignaledEvent.h"
#include "RakAssert.h"
#include "RakSleep.h"

using namespace RakNet;

SignaledEvent::SignaledEvent()
{
	eventList=INVALID_HANDLE_VALUE;
}
SignaledEvent::~SignaledEvent()
{
	// Intentionally do not close event, so it doesn't close twice on linux
}

void SignaledEvent::InitEvent(void)
{
	eventList=CreateEvent(0, false, false, 0);
}

void SignaledEvent::CloseEvent(void)
{
	if (eventList!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(eventList);
		eventList=INVALID_HANDLE_VALUE;
	}
}

void SignaledEvent::SetEvent(void)
{
	::SetEvent(eventList);
}

void SignaledEvent::WaitOnEvent(int timeoutMs)
{
//	WaitForMultipleObjects(
//		2,
//		eventList,
//		false,
//		timeoutMs);
	WaitForSingleObject(eventList,timeoutMs);
}
