//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMABSTRACTION_MACROS_H
#define RAMSES_PLATFORMABSTRACTION_MACROS_H

#if defined(__clang__)
#  define RFALLTHROUGH [[clang::fallthrough]]
#else
#  define RFALLTHROUGH
#endif

#if defined(__GNUC__)
#  define RNODISCARD __attribute__((warn_unused_result))
#elif _MSC_VER >= 1925
#  define RNODISCARD [[nodiscard]]
#else
#  define RNODISCARD
#endif

#if defined(__GNUC__)
#  define RFORMATCHECK(fmtstr_idx, vararg_idx) __attribute__ ((format (printf, fmtstr_idx, vararg_idx)))
#else
#  define RFORMATCHECK(fmtstr_idx, vararg_idx)
#endif

#endif
