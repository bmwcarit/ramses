//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

// Include this file to CPP files where a thread local log (log with prefixed thread ID) must always be used.
// The standard LOG macros are redefined to use the thread local versions.

#include "internal/Core/Utils/ThreadLocalLog.h"

#undef LOG_TRACE
#undef LOG_INFO
#undef LOG_DEBUG
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_FATAL
#undef LOG_TRACE_P
#undef LOG_INFO_P
#undef LOG_DEBUG_P
#undef LOG_WARN_P
#undef LOG_ERROR_P
#undef LOG_FATAL_P
#undef LOG_TRACE_F
#undef LOG_INFO_F
#undef LOG_DEBUG_F
#undef LOG_WARN_F
#undef LOG_ERROR_F
#undef LOG_FATAL_F
#undef LOG_TRACE_PF
#undef LOG_INFO_PF
#undef LOG_DEBUG_PF
#undef LOG_WARN_PF
#undef LOG_ERROR_PF
#undef LOG_FATAL_PF

#define LOG_TRACE LOG_TRACE_R
#define LOG_INFO LOG_INFO_R
#define LOG_DEBUG LOG_DEBUG_R
#define LOG_WARN LOG_WARN_R
#define LOG_ERROR LOG_ERROR_R
#define LOG_FATAL LOG_FATAL_R
#define LOG_TRACE_P LOG_TRACE_RP
#define LOG_INFO_P LOG_INFO_RP
#define LOG_DEBUG_P LOG_DEBUG_RP
#define LOG_WARN_P LOG_WARN_RP
#define LOG_ERROR_P LOG_ERROR_RP
#define LOG_FATAL_P LOG_FATAL_RP
#define LOG_TRACE_F LOG_TRACE_RF
#define LOG_INFO_F LOG_INFO_RF
#define LOG_DEBUG_F LOG_DEBUG_RF
#define LOG_WARN_F LOG_WARN_RF
#define LOG_ERROR_F LOG_ERROR_RF
#define LOG_FATAL_F LOG_FATAL_RF
#define LOG_TRACE_PF LOG_TRACE_RPF
#define LOG_INFO_PF LOG_INFO_RPF
#define LOG_DEBUG_PF LOG_DEBUG_RPF
#define LOG_WARN_PF LOG_WARN_RPF
#define LOG_ERROR_PF LOG_ERROR_RPF
#define LOG_FATAL_PF LOG_FATAL_RPF
