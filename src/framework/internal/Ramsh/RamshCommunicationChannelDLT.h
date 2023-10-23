//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <mutex>

namespace ramses::internal
{
    class Ramsh;

    /**
     * RamshInput from DLT
     */
    class RamshCommunicationChannelDLT final
    {
    public:
        static RamshCommunicationChannelDLT& GetInstance()
        {
            static RamshCommunicationChannelDLT instance;
            return instance;
        }

        void registerRamsh(Ramsh& ramsh);
        void unregisterRamsh(Ramsh& ramsh);

    private:
        RamshCommunicationChannelDLT();

        static int dltInjectionCallbackF(uint32_t sid, void* data, uint32_t length);
        void processInput(const std::string& s);

        std::mutex m_ramshLock;
        Ramsh* m_ramsh = nullptr;
    };

}
