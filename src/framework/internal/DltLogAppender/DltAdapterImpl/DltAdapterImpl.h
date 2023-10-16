//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/DltLogAppender/IDltAdapter.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/PlatformAbstraction/PlatformLock.h"

#include "internal/Core/Utils/LogContext.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"
#include "internal/Core/Utils/Warnings.h"

#include <string>

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Wold-style-cast)

#include <dlt_common_api.h>
#include <list>

WARNINGS_POP


namespace ramses::internal
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

        bool initialize(const std::string& id, const std::string& description, bool registerApplication,
                        const std::function<void(const std::string&, int)>& logLevelChangeCallback,
                        const std::vector<LogContext*>& contexts, bool pushLogLevelsToDaemon) override;
        void uninitialize() override;

        bool logMessage(const LogMessage& msg) override;
        bool registerInjectionCallback(LogContext* ctx, uint32_t sid, int (*dltInjectionCallback)(uint32_t service_id, void *data, uint32_t length)) override;
        bool transmitFile(LogContext& ctx, const std::string& uri, bool deleteFile) override;
        bool transmit(LogContext& ctx, std::vector<std::byte>&& data, const std::string& filename) override;

        bool isInitialized() override;

        static bool IsDummyAdapter()
        {
            return false;
        }

    private:
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        static void DltLogLevelChangedCallback(char context_id[DLT_ID_SIZE], uint8_t log_level, uint8_t trace_status);

        DltAdapterImpl();
        ~DltAdapterImpl() override;

        bool m_initialized = false;
        bool m_appRegistered = false;

        std::vector<LogContext*> m_contexts;
        std::function<void(const std::string&, int)> m_logLevelChangeCallback;

        class FileTransferWorker : private Runnable
        {
        public:
            FileTransferWorker();
            ~FileTransferWorker() override;

            bool transmit(LogContext& ctx, std::vector<std::byte>&& data, const std::string& filename);

        private:
            void run() override;

            struct FileTransfer
            {
                std::string filename;
                std::vector<std::byte> data;
                DltContext* ctx = nullptr;
                uint32_t serial = 0;
            };

            void get(FileTransfer& ft);
            bool pop(FileTransfer& ft);

            void send(const FileTransfer& ft);

            PlatformThread m_thread;
            PlatformLock m_lock;
            std::list<FileTransfer> m_files;
            uint32_t m_fileTransferSerial = 0;
        };

        FileTransferWorker m_fileTransfer;
    };
}
