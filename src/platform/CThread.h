/*
 *  CThread.h
 *  Created by aegzorz on 2007-02-09.
 *  Copyright 2007 Oxeye. All rights reserved.
 */

#ifndef _OX_CORE_CTHREAD_H_
#define _OX_CORE_CTHREAD_H_

#include <windows.h>

typedef void *( * pthread_fn )( void * );

	class CThread
	{
	public:
		CThread( pthread_fn threadFunc, void* threadParam );

		virtual ~CThread();
		
		static void sleep( const unsigned int millis );
	
	private:
		LPTHREAD_START_ROUTINE		mp_threadFunc;
		DWORD						m_threadID;
		HANDLE						m_threadHandle;
	};



#endif // _OX_CORE_CTHREAD_H_
