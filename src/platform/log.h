#ifndef LOG_H__
#define LOG_H__

#ifdef __cplusplus
	#include <cstdio>
#else
	#include <stdio.h>
#endif

#define __LOG_PUBLISH(...) do { __VA_ARGS__; } while(0)

#ifdef PUBLISH
    #define LOGV(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
    #define LOGI(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
    #define LOGW(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
    #define LOGE(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
#else
	#define LOGV(...) (printf(__VA_ARGS__))
	#define LOGI(...) (printf(__VA_ARGS__))
	#define LOGW(...) (printf(__VA_ARGS__))
	#define LOGE(...) (printf(__VA_ARGS__))
#endif

#ifdef _DEBUG
	#define LOGVV LOGV
#else
	#define LOGVV(fmt, ...) __LOG_PUBLISH(__VA_ARGS__)
#endif

#endif /*LOG_H__*/
