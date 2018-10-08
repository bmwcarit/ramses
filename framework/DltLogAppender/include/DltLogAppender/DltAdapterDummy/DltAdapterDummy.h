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

        virtual void logMessage(const LogMessage&) override {};

        virtual void* registerContext(LogContext*, bool, ELogLevel) override
        {
            return nullptr;
        };

        virtual bool registerApplication(const String& id, const String& desc) override
        {
            if (id.getLength() < 1 || id.getLength() > 4)
            {
                return false;
            }

            m_appName = id;
            m_appDesc = desc;
            return true;
        };

        virtual void unregisterApplication() override
        {
            m_appName.truncate(0);
            m_appDesc.truncate(0);
        };

        virtual void registerInjectionCallback(LogContext*, uint32_t, int(*)(uint32_t, void*, uint32_t)) override {};

        virtual Bool transmitFile(LogContext&, const String&, Bool) override
        {
            return true;
        }

        virtual void registerLogLevelChangeCallback(const std::function<void(const String&, int)>&) override
        {
        }

        virtual const String& getApplicationName() override
        {
            return m_appName;
        }

        virtual const String& getApplicationDescription() override
        {
            return m_appDesc;
        }

        virtual bool isDltInitialized() override
        {
            return true;
        }

        virtual EDltError getDltStatus() override
        {
            return EDltError_NO_ERROR;
        }

    private:
        DltAdapterDummy() {};
        ~DltAdapterDummy() {};

        /**
        * Name of the application,4 char string
        */
        String m_appName;

        /**
        * More detailed description of the application
        */
        String m_appDesc;
    };
}
#endif // RAMSES_DLTADAPTERDUMMY_H
