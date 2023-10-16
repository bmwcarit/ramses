//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/DltLogAppender/IDltAdapter.h"

namespace ramses::internal
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

        static bool IsDummyAdapter()
        {
            return true;
        }

        bool logMessage(const LogMessage& /*msg*/) override
        {
            return true;
        }

        bool initialize(const std::string& /*id*/, const std::string& /*description*/, bool /*registerApplication*/,
                        const std::function<void(const std::string&, int)>& /*logLevelChangeCallback*/,
                        const std::vector<LogContext*>& /*contexts*/, bool /*pushLogLevelsToDaemon*/) override
        {
            return true;
        }

        void uninitialize() override {}

        bool registerInjectionCallback(LogContext* /*ctx*/, uint32_t /*sid*/, int(* /*dltInjectionCallback*/)(uint32_t, void*, uint32_t)) override
        {
            return true;
        }

        bool transmitFile(LogContext& /*ctx*/, const std::string& /*uri*/, bool /*deleteFile*/) override
        {
            return true;
        }

        bool transmit(LogContext& /*ctx*/, std::vector<std::byte>&& /*data*/, const std::string& /*filename*/) override
        {
            return true;
        }

        bool isInitialized() override
        {
            return true;
        }

    private:
        DltAdapterDummy() = default;;
        ~DltAdapterDummy() override = default;;
    };
}
