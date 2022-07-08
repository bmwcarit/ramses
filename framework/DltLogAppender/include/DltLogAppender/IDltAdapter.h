//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDLTADAPTER_H
#define RAMSES_IDLTADAPTER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LogLevel.h"
#include "Utils/LogMessage.h"
#include "Utils/LogContext.h"
#include <functional>
#include <vector>

namespace ramses_internal
{
    class String;

    /**
     * Interface for DltAdapter
     */
    class IDltAdapter
    {
    public:

        /**
        * Destructor
        */
        virtual ~IDltAdapter() {};

        /**
         * Send log message to dlt
         * @param msg a string containing the log message
         */
        virtual bool logMessage(const LogMessage& msg) = 0;

        virtual bool initialize(const String& id, const String& description, bool registerApplication,
                                const std::function<void(const String&, int)>& logLevelChangeCallback,
                                const std::vector<LogContext*>& contexts, bool pushLogLevelsToDaemon) = 0;
        virtual void uninitialize() = 0;

        /**
         * Register injection callback
         * @param ctx the dlt context to use
         * @param sid the service id of the callback
         * @param dltInjectionCallback the function to be called trough the callback
         */
        virtual bool registerInjectionCallback(LogContext* ctx,uint32_t sid,int (*dltInjectionCallback)(uint32_t service_id, void *data, uint32_t length)) = 0;

        /**
         * Transmit a file via DLT
         * The file may not be completely transmitted when the function returns
         * @param uri the path to the file
         * @param deleteFile delete file afterwards
         */
        virtual bool transmitFile(LogContext& ctx, const String& uri, bool deleteFile) = 0;

        /**
         * Return the state if dlt was found at runtime
         * @return if dlt was successfully initialized
         */
        virtual bool isInitialized() = 0;
    };
}
#endif // RAMSES_DLTADAPTERIMPL_H
