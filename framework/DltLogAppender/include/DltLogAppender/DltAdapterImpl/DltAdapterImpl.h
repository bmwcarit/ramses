//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DLTADAPTERIMPL_H
#define RAMSES_DLTADAPTERIMPL_H

#include "DltLogAppender/IDltAdapter.h"

#include "Utils/LogContext.h"
#include "Collections/String.h"
#include "Collections/Vector.h"
#include "Collections/Pair.h"
#include "Utils/Warnings.h"

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Wold-style-cast)

#include <dlt_common_api.h>

WARNINGS_POP


namespace ramses_internal
{
    /**
     * Implementation for DltAdapter
     */
    class DltAdapterImpl : public IDltAdapter
    {
    public:
        /**
        * Get the singleton instance
        * @returns the DltAdapter singleton
        */
        static DltAdapterImpl* getDltAdapter();

        virtual void logMessage(const LogMessage& msg) override;
        virtual void* registerContext(LogContext* ctx, bool pushLogLevel, ELogLevel logLevel) override;
        virtual bool registerApplication(const String& id, const String& desc) override;
        virtual void unregisterApplication() override;
        virtual void registerInjectionCallback(LogContext* ctx,uint32_t sid,int (*dltInjectionCallback)(uint32_t service_id, void *data, uint32_t length)) override;
        virtual Bool transmitFile(LogContext& ctx, const String& uri, Bool deleteFile) override;

        virtual void registerLogLevelChangeCallback(const std::function<void(const String&, int)>& callback) override;

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
            return m_dltInitialized;
        }

        virtual EDltError getDltStatus() override
        {
            EDltError cur = m_dltError;
            m_dltError = EDltError_NO_ERROR;
            return cur;
        }

    private:

        static void DltLogLevelChangedCallback(char context_id[DLT_ID_SIZE], uint8_t log_level, uint8_t trace_status);

        /**
        * Private constructor
        */
        DltAdapterImpl();

        /**
        * Private destructor
        */
        ~DltAdapterImpl();

        /**
         * Name of the application,4 char string
         */
        String m_appName;

        /**
         * More detailed description of the application
         */
        String m_appDesc;

        /**
         * Bool to check if DLT was initialized correctly
         * if dlt libs are not there applications will not start due to linker failure
         * if dlt libs are there but dlt-daemon is not running dlt_init return negative
         * values, in which case this variable stays false
         */
        bool m_dltInitialized;

        /**
         * Internal flag to track errors
         */
        EDltError m_dltError;

        typedef std::pair<DltContext*, LogContext*> ContextPair;
        /**
         * Vector of all created contexts.
         */
        std::vector< ContextPair > m_dltContextList;

        std::function<void(const String&, int)> m_logLevelChangeCallback;
    };
}
#endif // RAMSES_DLTADAPTERIMPL_H
