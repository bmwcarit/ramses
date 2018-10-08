//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMUNICATIONCHANNELDLT_H
#define RAMSES_RAMSHCOMMUNICATIONCHANNELDLT_H

#include "Ramsh/RamshCommunicationChannel.h"
#include "Collections/String.h"

namespace ramses_internal
{
    /**
     * RamshInput from DLT
     */
    class RamshCommunicationChannelDLT : public RamshCommunicationChannel
    {
    public:
        /**
         * Construtctor
         */
        RamshCommunicationChannelDLT();
        virtual ~RamshCommunicationChannelDLT();

    private:
        static int dltInjectionCallbackF(uint32_t sid, void* data, uint32_t length);
        void processInput(String s);

        /**
         * Static communication Channel instance for DLT
         */
        static RamshCommunicationChannelDLT* m_instance;
    };

}// namespace ramses
#endif
