#ifndef __SIGNALED_EVENT_H
#define __SIGNALED_EVENT_H

#include <windows.h>

#include "Export.h"

namespace RakNet
{

class RAK_DLL_EXPORT SignaledEvent
{
public:
	SignaledEvent();
	~SignaledEvent();

	void InitEvent(void);
	void CloseEvent(void);
	void SetEvent(void);
	void WaitOnEvent(int timeoutMs);

protected:
	HANDLE eventList;
};

} // namespace RakNet

#endif
