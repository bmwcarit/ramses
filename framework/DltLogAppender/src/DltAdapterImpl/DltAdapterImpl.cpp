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

namespace ramses_internal
{
    DltAdapterImpl::DltAdapterImpl()
        : m_appName()
        , m_appDesc()
        , m_dltInitialized(false)
        , m_dltError(EDltError_NO_ERROR)
        , m_logLevelChangeCallback([](const String&, int) {})
    {
    }

    void DltAdapterImpl::DltLogLevelChangedCallback(char context_id[DLT_ID_SIZE], uint8_t log_level, uint8_t /*trace_status*/)
    {
        const String contextIDString(context_id, 0, DLT_ID_SIZE - 1); //set end value as passed char-array is not null-terminated
        getDltAdapter()->m_logLevelChangeCallback(contextIDString, log_level);
    }

    DltAdapterImpl::~DltAdapterImpl()
    {
#ifndef OS_WINDOWS
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
        DltAdapterImpl::unregisterApplication();
#endif
    }

    DltAdapterImpl* DltAdapterImpl::getDltAdapter()
    {
        static DltAdapterImpl dltAdapter;
        return &dltAdapter;
    }

    void DltAdapterImpl::logMessage(const LogMessage& msg)
    {
        if(!m_dltInitialized)
        {
            return;
        }

        if((&msg.getContext()) == nullptr)
        {
            m_dltError = EDltError_CONTEXT_INVALID;
            return;
        }

        DltContext* dltContext = static_cast<DltContext*>(msg.getContext().getUserData());
        if (dltContext == nullptr)
        {
            m_dltError = EDltError_MISSING_DLT_CONTEXT;
            return;
        }

        //DLT_LOG_OFF indicates that message is not logged
        //DLT_LOG_DEFAULT is unused here

        DltLogLevelType ll = DLT_LOG_OFF;

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
        default:
            //LL_OFF remains -> DLT_LOG_OFF is already set above
            break;
        }

        if(ll == DLT_LOG_OFF)
        {
            return; // no need to continue
        }
        WARNINGS_PUSH

        // DISABLE WARNING
        WARNING_DISABLE_LINUX(-Wold-style-cast)


        UInt maxLineCapacity = 130u;
#ifdef DLT_MESSAGE_MAX_SIZE
        maxLineCapacity = DLT_MESSAGE_MAX_SIZE;
#elif DLT_USER_BUF_MAX_SIZE
        maxLineCapacity = DLT_USER_BUF_MAX_SIZE;
#endif
        // 21 bytes are needed for meta-data: 4 (standard header) + 10 (extended header) + 4 (argument type) + 2 (text length) + 1 (0-terminated string)
        maxLineCapacity -= 30u; //30 subtracted to have some buffer

        const char* msgData = msg.getStream().c_str();
        uint32_t msgLength = msg.getStream().length();
        const char* msgDataEnd = msgData + msgLength;

        // check if shortcut is possible: short enough line and no linebreaks
        if (msgLength <= maxLineCapacity &&
            std::find(msgData, msgDataEnd, '\n') == msgDataEnd)
        {
            DLT_LOG1((*dltContext), ll, DLT_STRING(msgData));
        }
        else
        {
            // create modifyable copy of msg
            String s(msgData, 0, msgLength);
            InplaceStringTokenizer::TokenizeToCStrings(s, maxLineCapacity, '\n',
                [&](const char* tok) {
                    if (tok && *tok != 0) {
                        DLT_LOG1((*dltContext), ll, DLT_STRING(tok));
                    }
                });
        }

        WARNINGS_POP
    }

    bool DltAdapterImpl::registerApplication(const String& id,const String& description)
    {
        if (m_dltInitialized || id.getLength() < 1 || id.getLength() > 4)
        {
            return false;
        }

        m_appName = id;
        m_appDesc = description;

        DLT_REGISTER_APP(m_appName.c_str(), m_appDesc.c_str());

        m_dltInitialized = true;
        return true;
    }

    void DltAdapterImpl::unregisterApplication()
    {
        if(!m_dltInitialized)
        {
            return;
        }

        for(uint32_t i = 0; i < m_dltContextList.size(); i++ )
        {
            ContextPair& context = m_dltContextList[i];
            LogContext* logContext = context.second;
            logContext->setUserData(nullptr);

            DltContext* dltContext = context.first;
            DLT_UNREGISTER_CONTEXT((*dltContext));
            delete dltContext;
        }

        m_dltContextList.clear();

        DLT_UNREGISTER_APP();

        m_dltInitialized = false;

        m_appName.truncate(0);
        m_appDesc.truncate(0);
    }

    void* DltAdapterImpl::registerContext(LogContext* ctx, bool pushLogLevel, ELogLevel logLevel)
    {
        if(!m_dltInitialized)
        {
            return nullptr;
        }

        DltContext* dltContext = static_cast<DltContext*>(ctx->getUserData());
        if(dltContext)
        {
            // if pointer is set context is already initialized
            return static_cast<void*>(dltContext);
        }

        dltContext = new DltContext;
        m_dltContextList.push_back(ContextPair(dltContext, ctx));

#ifdef DLT_EMBEDDED
        UNUSED(pushLogLevel)
        UNUSED(logLevel)
        DLT_REGISTER_CONTEXT_APP((*dltContext), ctx->getContextId(), m_appName.c_str(), ctx->getContextName());
#else
        if (pushLogLevel)
        {
            dlt_register_context_ll_ts(dltContext,
                ctx->getContextId(),
                ctx->getContextName(),
                static_cast<int>(logLevel),
                DLT_TRACE_STATUS_OFF);
        }
        else
        {
            DLT_REGISTER_CONTEXT_APP((*dltContext), ctx->getContextId(), m_appName.c_str(), ctx->getContextName());
        }

        if (dlt_register_log_level_changed_callback(dltContext, &DltAdapterImpl::DltLogLevelChangedCallback) < 0)
        {
            m_dltError = EDltError_LOGLEVEL_CHANGED_CALLBACK_FAILURE;
        }
#endif
        ctx->setUserData(dltContext);
        return dltContext;
    }

    void DltAdapterImpl::registerInjectionCallback(LogContext* ctx, uint32_t sid, int (*dlt_injection_callback)(uint32_t service_id, void *data, uint32_t length))
    {
        if(!m_dltInitialized)
        {
            return;
        }

#ifdef DLT_EMBEDDED
        UNUSED(ctx);
        UNUSED(sid);
        UNUSED(dlt_injection_callback);
#else
        DltContext* dltContext = static_cast<DltContext*>(ctx->getUserData());
        if (dltContext)
        {
            if (dlt_register_injection_callback(dltContext, sid, dlt_injection_callback) < 0)
            {
                m_dltError = EDltError_INJECTION_CALLBACK_FAILURE;
            }
        }
#endif
    }

    Bool DltAdapterImpl::transmitFile(LogContext& ctx, const String& uri, Bool deleteFile)
    {
        if(!m_dltInitialized || uri.getLength() == 0 || ctx.getUserData() == nullptr)
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
            m_dltError = EDltError_FILETRANSFER_FAILURE;
            return false;
        }
        return true;
#endif
    }

    void DltAdapterImpl::registerLogLevelChangeCallback(const std::function<void(const String&, int)>& callback)
    {
        m_logLevelChangeCallback = callback;
    }
}
