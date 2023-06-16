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
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformLock.h"

#include "Utils/LogContext.h"
#include "Collections/Vector.h"
#include "Collections/Pair.h"
#include "Utils/Warnings.h"

#include <string>

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Wold-style-cast)

#include <dlt_common_api.h>
#include <list>

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

        bool initialize(const std::string& id, const std::string& description, bool registerApplication,
                        const std::function<void(const std::string&, int)>& logLevelChangeCallback,
                        const std::vector<LogContext*>& contexts, bool pushLogLevelsToDaemon) override;
        void uninitialize() override;

        bool logMessage(const LogMessage& msg) override;
        bool registerInjectionCallback(LogContext* ctx, uint32_t sid, int (*dltInjectionCallback)(uint32_t service_id, void *data, uint32_t length)) override;
        bool transmitFile(LogContext& ctx, const std::string& uri, bool deleteFile) override;

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

            bool transmitFile(LogContext& ctx, const std::string& uri, bool deleteFile);

        private:
            void run() override;

            struct FileTransfer
            {
                std::string filename;
                DltContext* ctx = nullptr;
                int deleteFlag = 0;
            };

            void get(FileTransfer& ft);
            bool pop(FileTransfer& ft);

            PlatformThread m_thread;
            PlatformLock m_lock;
            std::list<FileTransfer> m_files;
        };

        FileTransferWorker m_fileTransfer;
    };
}
#endif // RAMSES_DLTADAPTERIMPL_H
