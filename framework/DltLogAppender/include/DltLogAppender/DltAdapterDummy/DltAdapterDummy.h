//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DLTADAPTERDUMMY_H
#define RAMSES_DLTADAPTERDUMMY_H

#include "DltLogAppender/IDltAdapter.h"
#include "Collections/String.h"

namespace ramses_internal
{
    /**
    * Implementation for a dummy DltAdapter doing nothing
    */
    class DltAdapterDummy : public IDltAdapter
    {
    public:
        /**
        * Get the singleton instance
        * @returns the DltAdapter singleton
        */
        static DltAdapterDummy* getDltAdapter()
        {
            static DltAdapterDummy dltAdapter;
            return &dltAdapter;
        }

        static bool IsDummyAdapter()
        {
            return true;
        }

        bool logMessage(const LogMessage&) override
        {
            return true;
        }

        bool initialize(const String& /*id*/, const String& /*description*/, bool /*registerApplication*/,
                        const std::function<void(const String&, int)>& /*logLevelChangeCallback*/,
                        const std::vector<LogContext*>& /*contexts*/, bool /*pushLogLevelsToDaemon*/) override
        {
            return true;
        }

        void uninitialize() override {}

        bool registerInjectionCallback(LogContext*, uint32_t, int(*)(uint32_t, void*, uint32_t)) override
        {
            return true;
        }

        bool transmitFile(LogContext&, const String&, bool) override
        {
            return true;
        }

        bool isInitialized() override
        {
            return true;
        }

    private:
        DltAdapterDummy() {};
        ~DltAdapterDummy() override {};
    };
}
#endif // RAMSES_DLTADAPTERDUMMY_H
