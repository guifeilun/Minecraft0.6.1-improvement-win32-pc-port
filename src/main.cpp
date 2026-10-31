/**
    Entry point for the Windows (win32) build.
*/

#define _SECURE_SCL 0

#include "platform/log.h"
#include "NinecraftApp.h"
#define MAIN_CLASS NinecraftApp

#ifndef STANDALONE_SERVER
	#if defined(PLATFORM_DESKTOP)
		#include "main_glfw.h"
	#elif defined(WIN32)
		#include "main_win32.h"
	#endif
#endif
