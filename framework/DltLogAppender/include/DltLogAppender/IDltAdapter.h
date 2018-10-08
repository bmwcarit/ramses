//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDLTADAPTER_H
#define RAMSES_IDLTADAPTER_H

#include "DltLogAppender/DltAdapterTypes.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Utils/LogLevel.h"
#include "Utils/LogMessage.h"
#include "Utils/LogContext.h"
#include <functional>

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
        virtual void logMessage(const LogMessage& msg) = 0;

        /**
         * Register new context, if context already exists no new context is created
         * @param ctx log context to use
         * @returns void ptr to the newly created dlt context
         */
        virtual void* registerContext(LogContext* ctx, bool pushLogLevel, ELogLevel logLevel) = 0;

        /**
         * Register new application. Every application has one unique name
         * @param id id string for application
         * @param desc description text with more details than the name
         */
        virtual bool registerApplication(const String& id, const String& desc) = 0;

        /**
         * Unregister Application
         */
        virtual void unregisterApplication() = 0;

        /**
         * Register injection callback
         * @param ctx the dlt context to use
         * @param sid the service id of the callback
         * @param dltInjectionCallback the function to be called trough the callback
         */
        virtual void registerInjectionCallback(LogContext* ctx,uint32_t sid,int (*dltInjectionCallback)(uint32_t service_id, void *data, uint32_t length)) = 0;

        /**
         * Transmit a file via DLT
         * @param uri the path to the file
         * @param deleteFile delete file afterwards
         */
        virtual Bool transmitFile(LogContext& ctx, const String& uri, Bool deleteFile) = 0;

        virtual void registerLogLevelChangeCallback(const std::function<void(const String&, int)>& callback) = 0;

        /**
         * Returns the application name
         * @returns name of the application
         */
        virtual const String& getApplicationName() = 0;

        /**
         * Returns the description of the application
         * @return description of the application
         */
        virtual const String& getApplicationDescription() = 0;

        /**
         * Return the state if dlt was found at runtime
         * @return if dlt was successfully initialized
         */
        virtual bool isDltInitialized() = 0;

        /**
         * Get the error status of the DltAdapter
         * DLT_ERROR_NO_ERROR indicates that no error occurred
         * @returns enum with error code and clears the current error
         */
        virtual EDltError getDltStatus() = 0;
    };
}
#endif // RAMSES_DLTADAPTERIMPL_H
