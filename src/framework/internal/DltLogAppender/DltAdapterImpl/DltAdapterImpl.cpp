//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/DltLogAppender/DltAdapterImpl/DltAdapterImpl.h"
#include "internal/Core/Utils/Warnings.h"
#include "internal/Core/Utils/LogLevel.h"
#include "internal/Core/Utils/LogMessage.h"
#include "internal/Core/Utils/InplaceStringTokenizer.h"
#include "internal/Core/Utils/File.h"
#include <cassert>
#include <string>

namespace ramses::internal
{
    DltAdapterImpl::DltAdapterImpl()
        : m_logLevelChangeCallback([](const std::string& /*unused*/, int /*unused*/) {})
    {
    }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    void DltAdapterImpl::DltLogLevelChangedCallback(char context_id[DLT_ID_SIZE], uint8_t log_level, uint8_t /*trace_status*/)
    {
        const std::string contextIDString(context_id, 0, DLT_ID_SIZE - 1); //set end value as passed char-array is not null-terminated
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

        auto* dltContext = static_cast<DltContext*>(msg.m_context.getUserData());
        if (dltContext == nullptr)
        {
            fmt::print(stderr, "DltAdapterImpl::logMessage: missing dlt context\n");
            return false;
        }

        //DLT_LOG_OFF indicates that message is not logged
        //DLT_LOG_DEFAULT is unused here

        auto ll = DLT_LOG_OFF;

        switch(msg.m_logLevel)
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

        size_t maxLineCapacity = 130u;
#ifdef DLT_MESSAGE_MAX_SIZE
        maxLineCapacity = DLT_MESSAGE_MAX_SIZE;
#elif DLT_USER_BUF_MAX_SIZE
        maxLineCapacity = DLT_USER_BUF_MAX_SIZE;
#endif
        // 21 bytes are needed for meta-data: 4 (standard header) + 10 (extended header) + 4 (argument type) + 2 (text length) + 1 (0-terminated string)
        maxLineCapacity -= 30u; //30 subtracted to have some buffer

        const char* msgData = msg.m_message.c_str();
        uint32_t msgLength = msg.m_message.size();
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
            std::string s(msgData, 0, msgLength);
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

    bool DltAdapterImpl::initialize(const std::string& appId, const std::string& appDescription, bool registerApplication,
                                    const std::function<void(const std::string&, int)>& logLevelChangeCallback,
                                    const std::vector<LogContext*>& contexts, bool pushLogLevelsToDaemon)
    {
        if (m_initialized)
        {
            fmt::print(stderr, "DltAdapterImpl::initialize: already initialized\n");
            return false;
        }

        if (registerApplication && (appId.empty() || appId.size() > 4))
        {
            fmt::print(stderr, "DltAdapterImpl::initialize: dlt app id must be set\n");
            return false;
        }

        if (contexts.empty())
        {
            fmt::print(stderr, "DltAdapterImpl::initialize: contexts may not be empty\n");
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
            auto* dltContext = new DltContext;
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
                fmt::print(stderr, "DltAdapterImpl::initialize: set loglevel changed callback failure\n");
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
            auto* dltContext = static_cast<DltContext*>(ctx->getUserData());
            ctx->setUserData(nullptr);

            DLT_UNREGISTER_CONTEXT((*dltContext));
            delete dltContext;
        }
        m_contexts.clear();

        if (m_appRegistered)
        {
            DLT_UNREGISTER_APP();
        }

        m_logLevelChangeCallback = [](const std::string& /*unused*/, int /*unused*/) {};
        m_appRegistered = false;
        m_initialized = false;
    }

    bool DltAdapterImpl::registerInjectionCallback(LogContext* ctx, uint32_t sid, int (*dltInjectionCallback)(uint32_t service_id, void *data, uint32_t length))
    {
        if(!m_initialized)
        {
            return false;
        }
        auto* dltContext = static_cast<DltContext*>(ctx->getUserData());
        if (!dltContext)
        {
            fmt::print(stderr, "DltAdapterImpl::registerInjectionCallback: no dlt context\n");
            return false;
        }
        if (dlt_register_injection_callback(dltContext, sid, dltInjectionCallback) < 0)
        {
            fmt::print(stderr, "DltAdapterImpl::registerInjectionCallback: failed\n");
            return false;
        }
        return true;
    }

    bool DltAdapterImpl::transmitFile(LogContext& ctx, const std::string& uri, bool deleteFile)
    {
        if (!m_initialized)
        {
            return false;
        }
        File f(uri);
        size_t size = 0u;
        if (!f.getSizeInBytes(size))
        {
            return false;
        }
        if (!f.open(File::Mode::ReadOnlyBinary))
        {
            return false;
        }
        std::vector<std::byte> data;
        data.resize(size);
        if (f.read(data.data(), data.size(), size) != EStatus::Ok)
        {
            return false;
        }
        f.close();
        if (deleteFile)
        {
            f.remove();
        }

        return m_fileTransfer.transmit(ctx, std::move(data), uri);
    }

    bool DltAdapterImpl::transmit(LogContext& ctx, std::vector<std::byte>&& data, const std::string& filename)
    {
        if (!m_initialized)
        {
            return false;
        }
        return m_fileTransfer.transmit(ctx, std::move(data), filename);
    }

    bool DltAdapterImpl::isInitialized()
    {
        return m_initialized;
    }

    DltAdapterImpl::FileTransferWorker::FileTransferWorker()
        : m_thread("R_FileTransfer")
    {
    }

    DltAdapterImpl::FileTransferWorker::~FileTransferWorker()
    {
        m_thread.cancel();
        m_thread.join();
    }

    bool DltAdapterImpl::FileTransferWorker::transmit(LogContext& ctx, std::vector<std::byte>&& data, const std::string& filename)
    {
        if (filename.empty() || ctx.getUserData() == nullptr)
        {
            return false;
        }

        FileTransfer ft;
        ft.filename = filename;
        ft.ctx = static_cast<DltContext*>(ctx.getUserData());
        ft.data = data;
        ft.serial = ++m_fileTransferSerial;
        bool startThread = false;
        {
            PlatformGuard guard(m_lock);
            startThread = m_files.empty();
            m_files.push_back(ft);
        }

        if (startThread)
        {
            m_thread.join();
            resetCancel();
            m_thread.start(*this);
        }
        return true;
    }

    void DltAdapterImpl::FileTransferWorker::get(DltAdapterImpl::FileTransferWorker::FileTransfer& ft)
    {
        PlatformGuard guard(m_lock);
        assert(!m_files.empty());
        ft = m_files.front();
    }

    bool DltAdapterImpl::FileTransferWorker::pop(DltAdapterImpl::FileTransferWorker::FileTransfer& ft)
    {
        PlatformGuard guard(m_lock);
        assert(!m_files.empty());
        m_files.pop_front();
        if (m_files.empty())
        {
            return false;
        }
        ft = m_files.front();
        return true;
    }

    void DltAdapterImpl::FileTransferWorker::run()
    {
        // Sending files per DLT is done by chunking the binary file data in very small chunks (~1024 bytes) and send these per regular DLT_LOG(..)
        // After each chunk the file transfer should wait for some time, so the DLT-FIFO will not be flooded with too many messages.
        // The DLT implementation recommends a timeout of 20 ms, but this would mean long transfer durations for typical scene dumps.
        // To prevent these long delays we simply use a smaller timeout and check for dlt buffer overflows (dlt_user_check_buffer())
#ifdef DLT_HAS_FILETRANSFER
        const auto uptime = std::chrono::seconds(dlt_uptime() / 10000);
#else
        // no dlt_uptime() available - do not wait
        const auto uptime = std::chrono::seconds(60);
#endif
        const auto startupDelay = std::chrono::seconds(60);
        if (uptime < startupDelay)
        {
            // do not transfer files at system startup (reduces risk of losing messages)
            std::this_thread::sleep_for(startupDelay - uptime);
        }
        FileTransfer ft;
        get(ft);
        do
        {
            send(ft);
        } while(pop(ft) && !isCancelRequested());
    }

    void DltAdapterImpl::FileTransferWorker::send(const FileTransfer &ft)
    {
        const int timeoutInMS = 1;
        const size_t bufferSize = 1024;
        const auto filename = ft.filename.c_str();
        const auto packages = (ft.data.size() % bufferSize == 0) ? (ft.data.size() / bufferSize) : (ft.data.size() / bufferSize + 1);

#ifdef DLT_HAS_FILETRANSFER
        auto canWriteToDLT = [](){
            int dltTotal = 0;
            int dltUsed = 0;
            // file transfer should not use more than 50% of the dlt buffer
            dlt_user_check_buffer(&dltTotal, &dltUsed);
            const auto dltFree = dltTotal - dltUsed;
            return (dltFree > (dltTotal / 2) && dltFree > static_cast<int>(5 * bufferSize));
        };
#else
        auto canWriteToDLT = [](){
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            return true;
        };
#endif

        while (!canWriteToDLT())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }

        DLT_LOG8(*ft.ctx, DLT_LOG_INFO,
                DLT_STRING("FLST"),
                DLT_UINT(ft.serial),
                DLT_STRING(filename),
                DLT_UINT(ft.data.size()),
                DLT_STRING(""),
                DLT_UINT(packages),
                DLT_UINT(bufferSize),
                DLT_STRING("FLST")
                );

        for (uint32_t i = 0; ((i < packages) && !isCancelRequested()); ++i)
        {
            while (!canWriteToDLT())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(timeoutInMS));
            }
            const auto it = ft.data.begin() + static_cast<ptrdiff_t>(i * bufferSize);
            const auto chunkSize = std::min(static_cast<size_t>(ft.data.end() - it), bufferSize);
            DLT_LOG5(*ft.ctx, DLT_LOG_INFO,
                    DLT_STRING("FLDA"),
                    DLT_UINT(ft.serial),
                    DLT_UINT(i + 1),
                    DLT_RAW(const_cast<std::byte*>(&(*it)), static_cast<uint16_t>(chunkSize)),
                    DLT_STRING("FLDA")
                    );
        }

        DLT_LOG3(*ft.ctx, DLT_LOG_INFO,
                DLT_STRING("FLFI"),
                DLT_UINT(ft.serial),
                DLT_STRING("FLFI")
                );
    }
}
