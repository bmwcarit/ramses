//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WindowsInvalidParameterCheckSuppression.h"
#include <cstdio>
#include <cstdlib>

namespace ramses::internal
{
#ifdef _WIN32

    static void handler(const wchar_t* /*expression*/, const wchar_t* /*function*/, const wchar_t* /*file*/,
        unsigned int /*line*/, uintptr_t /*pReserved*/)
    {
    }

    WindowsInvalidParameterCheckSuppression::WindowsInvalidParameterCheckSuppression()
    {
        m_previousHandler = _set_invalid_parameter_handler(handler);
    }

    WindowsInvalidParameterCheckSuppression::~WindowsInvalidParameterCheckSuppression()
    {
        _set_invalid_parameter_handler((_invalid_parameter_handler)m_previousHandler);
    }

#else

    WindowsInvalidParameterCheckSuppression::WindowsInvalidParameterCheckSuppression()
    {
        (void)m_previousHandler;
    }

    WindowsInvalidParameterCheckSuppression::~WindowsInvalidParameterCheckSuppression() = default;

#endif
}
