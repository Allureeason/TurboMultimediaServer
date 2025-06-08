#ifndef MMEDIA_BASE_LOG_H
#define MMEDIA_BASE_LOG_H

#include "base/LogStream.h"

using namespace tmms::base;

#define RTMP_DEBUG_ON 1

#if RTMP_DEBUG_ON
#define RTMPLOG_TRACE LOG_TRACE
#define RTMPLOG_DEBUG LOG_DEBUG
#define RTMPLOG_INFO LOG_INFO
#else
#define RTMPLOG_TRACE if (0) LOG_TRACE
#define RTMPLOG_DEBUG if (0) LOG_DEBUG
#define RTMPLOG_INFO if (0) LOG_INFO
#endif

#define RTMPLOG_WARN LOG_WARN
#define RTMPLOG_ERROR LOG_ERROR

#endif