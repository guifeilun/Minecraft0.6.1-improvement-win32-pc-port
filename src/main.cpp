/**
    Entry point for the Windows (win32) build.
*/

#define _SECURE_SCL 0

#include "platform/log.h"

#include "NinecraftApp.h"
#define MAIN_CLASS NinecraftApp

#if defined(PLATFORM_DESKTOP)
	// Desktop (GLFW) build — the current win32 build path.
	#include "main_glfw.h"
#elif defined(WIN32)
	// Legacy win32 (EGL) build.
	#include "main_win32.h"
#endif
