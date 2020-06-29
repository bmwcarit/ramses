//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DltLogAppender/DltAdapterImpl/DltAdapterImpl.h"
#include "Utils/Warnings.h"

#ifdef DLT_HAS_FILETRANSFER
extern "C"
{
#include <dlt/dlt_filetransfer.h>
}
#endif
#include "Utils/LogLevel.h"
#include "Utils/LogMessage.h"
#include "Utils/InplaceStringTokenizer.h"
#include <cassert>

namespace ramses_internal
{
    DltAdapterImpl::DltAdapterImpl()
        : m_logLevelChangeCallback([](const String&, int) {})
    {
    }

    void DltAdapterImpl::DltLogLevelChangedCallback(char context_id[DLT_ID_SIZE], uint8_t log_level, uint8_t /*trace_status*/)
    {
        const String contextIDString(context_id, 0, DLT_ID_SIZE - 1); //set end value as passed char-array is not null-terminated
        getDltAdapter()->m_logLevelChangeCallback(contextIDString, log_level);
    }

    DltAdapterImpl::~DltAdapterImpl()
    {
#ifndef _WIN32
        //DLTAdapter is a static object that is destructed on process or DLL exit.
        //In case of DLL destruction Windows already destroys all threads before executing
        //static destructors.
        //Do not unregister application to prevent a hang which is cause by the dlt internal
        //thread handling which is invalid at this point.
        //see Windows documentation here:
        //"If the process is terminating(the lpvReserved parameter is non - NULL),
        //all threads in the process except the current thread either have exited
        //already or have been explicitly terminated by a call to the ExitProcess
        //function."
        //https://msdn.microsoft.com/en-us/library/windows/desktop/ms682583(v=vs.85).aspx
        DltAdapterImpl::uninitialize();
#endif
    }

    DltAdapterImpl* DltAdapterImpl::getDltAdapter()
    {
        static DltAdapterImpl dltAdapter;
        return &dltAdapter;
    }

    bool DltAdapterImpl::logMessage(const LogMessage& msg)
    {
        if(!m_initialized)
        {
            return false;
        }

        DltContext* dltContext = static_cast<DltContext*>(msg.getContext().getUserData());
        if (dltContext == nullptr)
        {
            fprintf(stderr, "DltAdapterImpl::logMessage: missing dlt context\n");
            return false;
        }

        //DLT_LOG_OFF indicates that message is not logged
        //DLT_LOG_DEFAULT is unused here

        auto ll = DLT_LOG_OFF;

        switch(msg.getLogLevel())
        {
        case ELogLevel::Trace:
            ll = DLT_LOG_VERBOSE;
            break;
        case ELogLevel::Debug:
            ll = DLT_LOG_DEBUG;
            break;
        case ELogLevel::Info:
            ll = DLT_LOG_INFO;
            break;
        case ELogLevel::Warn:
            ll = DLT_LOG_WARN;
            break;
        case ELogLevel::Error:
            ll = DLT_LOG_ERROR;
            break;
        case ELogLevel::Fatal:
            ll = DLT_LOG_FATAL;
            break;
        case ELogLevel::Off:
            // no need to continue
            return true;
        }

        UInt maxLineCapacity = 130u;
#ifdef DLT_MESSAGE_MAX_SIZE
        maxLineCapacity = DLT_MESSAGE_MAX_SIZE;
#elif DLT_USER_BUF_MAX_SIZE
        maxLineCapacity = DLT_USER_BUF_MAX_SIZE;
#endif
        // 21 bytes are needed for meta-data: 4 (standard header) + 10 (extended header) + 4 (argument type) + 2 (text length) + 1 (0-terminated string)
        maxLineCapacity -= 30u; //30 subtracted to have some buffer

        const char* msgData = msg.getStream().c_str();
        uint32_t msgLength = msg.getStream().size();
        const char* msgDataEnd = msgData + msgLength;

        // check if shortcut is possible: short enough line and no linebreaks
        if (msgLength <= maxLineCapacity &&
            std::find(msgData, msgDataEnd, '\n') == msgDataEnd)
        {
            WARNINGS_PUSH
            WARNING_DISABLE_LINUX(-Wold-style-cast)
            DLT_LOG1((*dltContext), ll, DLT_STRING(msgData));
            WARNINGS_POP
        }
        else
        {
            // create modifyable copy of msg
            String s(msgData, 0, msgLength);
            InplaceStringTokenizer::TokenizeToCStrings(s, maxLineCapacity, '\n',
                [&](const char* tok) {
                    if (tok && *tok != 0)
                    {
                        WARNINGS_PUSH
                        WARNING_DISABLE_LINUX(-Wold-style-cast)
                        DLT_LOG1((*dltContext), ll, DLT_STRING(tok));
                        WARNINGS_POP
                    }
                });
        }
        return true;
    }

    bool DltAdapterImpl::initialize(const String& appId, const String& appDescription, bool registerApplication,
                                    const std::function<void(const String&, int)>& logLevelChangeCallback,
                                    const std::vector<LogContext*>& contexts, bool pushLogLevelsToDaemon)
    {
        if (m_initialized)
        {
            fprintf(stderr, "DltAdapterImpl::initialize: already initialized\n");
            return false;
        }

        if (registerApplication && (appId.size() < 1 || appId.size() > 4))
        {
            fprintf(stderr, "DltAdapterImpl::initialize: dlt app id must be set\n");
            return false;
        }

        if (contexts.empty())
        {
            fprintf(stderr, "DltAdapterImpl::initialize: contexts may not be empty\n");
            return false;
        }

        // register application if requested
        if (registerApplication)
        {
            DLT_REGISTER_APP(appId.c_str(), appDescription.c_str());
            m_appRegistered = true;
        }

        // register callback before creating contexts (they might call it already
        m_logLevelChangeCallback = logLevelChangeCallback;

        // register contexts
        for (auto ctx : contexts)
        {
            assert(!ctx->getUserData());
            DltContext* dltContext = new DltContext;
            ctx->setUserData(dltContext);

            if (pushLogLevelsToDaemon)
            {
                dlt_register_context_ll_ts(dltContext,
                                           ctx->getContextId(),
                                           ctx->getContextName(),
                                           static_cast<int>(ctx->getLogLevel()),
                                           DLT_TRACE_STATUS_OFF);
            }
            else
            {
                DLT_REGISTER_CONTEXT((*dltContext), ctx->getContextId(), ctx->getContextName());
            }

            if (dlt_register_log_level_changed_callback(dltContext, &DltAdapterImpl::DltLogLevelChangedCallback) < 0)
            {
                fprintf(stderr, "DltAdapterImpl::initialize: set loglevel changed callback failure\n");
            }
        }

        m_contexts = contexts;
        m_initialized = true;
        return true;
    }

    void DltAdapterImpl::uninitialize()
    {
        if(!m_initialized)
        {
            return;
        }

        for (auto ctx : m_contexts)
        {
            DltContext* dltContext = static_cast<DltContext*>(ctx->getUserData());
            ctx->setUserData(nullptr);

            DLT_UNREGISTER_CONTEXT((*dltContext));
            delete dltContext;
        }
        m_contexts.clear();

        if (m_appRegistered)
        {
            DLT_UNREGISTER_APP();
        }

        m_logLevelChangeCallback = [](const String&, int) {};
        m_appRegistered = false;
        m_initialized = false;
    }

    bool DltAdapterImpl::registerInjectionCallback(LogContext* ctx, uint32_t sid, int (*dlt_injection_callback)(uint32_t service_id, void *data, uint32_t length))
    {
        if(!m_initialized)
        {
            return false;
        }
        DltContext* dltContext = static_cast<DltContext*>(ctx->getUserData());
        if (!dltContext)
        {
            fprintf(stderr, "DltAdapterImpl::registerInjectionCallback: no dlt context\n");
            return false;
        }
        if (dlt_register_injection_callback(dltContext, sid, dlt_injection_callback) < 0)
        {
            fprintf(stderr, "DltAdapterImpl::registerInjectionCallback: failed\n");
            return false;
        }
        return true;
    }

    bool DltAdapterImpl::transmitFile(LogContext& ctx, const String& uri, bool deleteFile)
    {
        if(!m_initialized || uri.size() == 0 || ctx.getUserData() == nullptr)
        {
            return false;
        }
#ifndef DLT_HAS_FILETRANSFER
        UNUSED(deleteFile);
        return false;
#else
        // Sending files per dlt is done by chunking the binary file data in very small chunks (~1024 bytes) and send these per regular DLT_LOG(..)
        // after each chunk will sleep for the timeout so the FIFO of dlt will not be flooded with to many messages in a short period of time.
        // The dlt implementation recommends a timeout of 20 ms, unfortunately due the very small chunk size a huge number of messages has to be sent and
        // this will cause a transfer time of ~30 seconds for a file of 1.5 MB.
        // To prevent these long delays we simply use a smaller timeout which did not cause any troubles in our tests.
        const int timeoutInMS = 1;
        const int deleteFileAsInt = deleteFile ? 1 : 0;

        DltContext* fileContext = static_cast<DltContext*>(ctx.getUserData());
        if (dlt_user_log_file_complete(fileContext, uri.c_str(), deleteFileAsInt, timeoutInMS) < 0)
        {
            return false;
        }
        return true;
#endif
    }

    bool DltAdapterImpl::isInitialized()
    {
        return m_initialized;
    }
}
