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

        bool initialize(const String& id, const String& description, bool registerApplication,
                        const std::function<void(const String&, int)>& logLevelChangeCallback,
                        const std::vector<LogContext*>& contexts, bool pushLogLevelsToDaemon) override;
        void uninitialize() override;

        virtual bool logMessage(const LogMessage& msg) override;
        virtual bool registerInjectionCallback(LogContext* ctx, uint32_t sid, int (*dltInjectionCallback)(uint32_t service_id, void *data, uint32_t length)) override;
        virtual bool transmitFile(LogContext& ctx, const String& uri, bool deleteFile) override;

        virtual bool isInitialized() override;

        static bool IsDummyAdapter()
        {
            return false;
        }

    private:
        static void DltLogLevelChangedCallback(char context_id[DLT_ID_SIZE], uint8_t log_level, uint8_t trace_status);

        DltAdapterImpl();
        ~DltAdapterImpl();

        bool m_initialized = false;
        bool m_appRegistered = false;

        std::vector<LogContext*> m_contexts;
        std::function<void(const String&, int)> m_logLevelChangeCallback;
    };
}
#endif // RAMSES_DLTADAPTERIMPL_H
