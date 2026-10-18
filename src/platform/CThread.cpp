/*
 *  CThread.cpp
 *  oxeye
 *
 *  Created by aegzorz on 2007-02-09.
 *  Copyright 2007 Mojang AB. All rights reserved.
 *
 */

#include "CThread.h"



	CThread::CThread( pthread_fn threadFunc, void* threadParam )
	{
		mp_threadFunc = (LPTHREAD_START_ROUTINE) threadFunc;
		
		m_threadHandle = CreateThread(
			NULL,				// pointer to security attributes
			NULL,               // initial thread stack size
			mp_threadFunc,		// pointer to thread function
			threadParam,        // argument for new thread
			NULL,               // creation flags
			&m_threadID        // pointer to receive thread ID
		);
	}
	
	void CThread::sleep( const unsigned int millis )
	{
		Sleep( millis );
	}

	CThread::~CThread()
	{
		TerminateThread(m_threadHandle, 0);
	}


