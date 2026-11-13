/**
    Entry point for cross compilation.

    This is ugly, yes. But it will change in the "half near" future
    to a more correct system of solving the cross-platform entry
    point "problem" */

#define _SECURE_SCL 0

#include "platform/log.h"
#include "NinecraftApp.h"
#define MAIN_CLASS NinecraftApp

#ifndef STANDALONE_SERVER
	#if defined(PLATFORM_ANDROID)
		#ifdef PRE_ANDROID23
			#include "main_android_java.h"
		#else
			#include "main_android.h"
		#endif
	#elif defined(PLATFORM_RPI)
		#include "main_rpi.h"
	#elif defined(PLATFORM_DESKTOP) || defined(PLATFORM_WEB)
		#include "main_glfw.h"
	#elif defined(WIN32)
		// Native Win32 (Visual Studio project) build.
		#include "main_win32.h"
	#endif
#endif