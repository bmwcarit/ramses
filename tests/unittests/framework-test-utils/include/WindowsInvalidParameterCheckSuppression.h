//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses::internal
{
    /*
     * On windows some system functions have extended error checking that can result
     * in exceptions instead of the usual error return value. This is bad for testing
     * error cases.
     * This class temporarily disable these exceptions and make the affected functions
     * return normal error return values.
     *
     * See
     * https://docs.microsoft.com/en-us/cpp/c-runtime-library/parameter-validation?view=msvc-160
     */
    class WindowsInvalidParameterCheckSuppression
    {
    public:
        WindowsInvalidParameterCheckSuppression();
        ~WindowsInvalidParameterCheckSuppression(); // NOLINT(performance-trivially-destructible)

    private:
        void* m_previousHandler{nullptr};
    };
}
