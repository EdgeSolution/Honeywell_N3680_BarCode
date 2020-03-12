// ============================================================================
// $Id: logs.h 158 2018-12-06 17:58:56Z e411776 $
// Copyright Honeywell International Inc. 2018
// ============================================================================
/** Provide LOG macros for NDK code.
	\file */
//=============================================================================

#include <android/log.h>

// You need to provide the kTAG string, so the right info shows up in the logs.
// Example:
// static const char* kTAG = "appname-modulename";

#define LOGD(...) \
  ((void)__android_log_print(ANDROID_LOG_DEBUG, kTAG, __VA_ARGS__))
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))
#define LOGF(...) \
  ((void)__android_log_print(ANDROID_LOG_FATAL, kTAG, __VA_ARGS__))


#define TRACE LOGD