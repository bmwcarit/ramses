//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ASIOWRAPPER_H
#define RAMSES_ASIOWRAPPER_H

#include "Utils/Warnings.h"

// disable warnings temporarily and set configuration defines
WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Wold-style-cast)
WARNING_DISABLE_GCC(-Wsuggest-override)
WARNING_DISABLE_CLANG(-Wunused-private-field)

// specify exactly what we expect to exist compiler and library wise. Ensures
// consistenzt configuration and works around missing feature detection for
// integrity compiler.
#define ASIO_STANDALONE
#define ASIO_HAS_THREADS
#define ASIO_HAS_STD_TYPE_TRAITS
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_CXX11_ALLOCATORS
#define ASIO_NO_EXCEPTIONS
#define ASIO_HAS_STD_ATOMIC
#define ASIO_HAS_STD_SYSTEM_ERROR
#define ASIO_HAS_STD_CHRONO
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_STD_FUNCTION
#define ASIO_HAS_STD_MUTEX_AND_CONDVAR
#define ASIO_HAS_MOVE
#define ASIO_HAS_NULLPTR
#define ASIO_HAS_VARIADIC_TEMPLATES
#define ASIO_HAS_CHRONO
#define ASIO_DELETED
#define ASIO_HAS_CONSTEXPR
#define ASIO_HAS_DECLTYPE
#define ASIO_HAS_ALIAS_TEMPLATES
#define ASIO_HAS_STD_ALLOCATOR_ARG
#define ASIO_HAS_STD_THREAD
#define ASIO_HAS_STD_CALL_ONCE
#define ASIO_DISABLE_STD_FUTURE
#define ASIO_DISABLE_STD_STRING_VIEW
#define ASIO_DISABLE_STD_EXPERIMENTAL_STRING_VIEW
#define ASIO_DISABLE_STD_INVOKE_RESULT

#if defined(__INTEGRITY)
// no CLOCK_MONOTONIC, fall back on realtime
#define CLOCK_MONOTONIC CLOCK_REALTIME

// provice dummy function for changing condvar clock
inline int pthread_condattr_setclock(pthread_condattr_t *, clockid_t)
{
    return 0;
}
#endif

#include "asio/io_service.hpp"
#include "asio/steady_timer.hpp"
#include "asio/ip/address.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/connect.hpp"
#include "asio/read.hpp"
#include "asio/write.hpp"

#if defined(__INTEGRITY)
#undef CLOCK_MONOTONIC
#endif

WARNINGS_POP

// provide exception throwing function for use with exceptions disabled
namespace asio
{
    namespace detail
    {
        template <typename Exception>
        void throw_exception(const Exception& /*e*/)
        {
            // for now simply abort
            abort();
        }
    }
}

#endif
