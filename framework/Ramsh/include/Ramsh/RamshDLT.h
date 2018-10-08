//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHDLT_H
#define RAMSES_RAMSHDLT_H

#include "Ramsh/Ramsh.h"
#include "Ramsh/RamshCommunicationChannelDLT.h"

namespace ramses_internal
{
    class RamshDLT : public Ramsh
    {
    public:
        RamshDLT(String prompt = "noname");

        virtual bool start();
        virtual bool stop();

        ~RamshDLT();

    protected:
        Bool m_started;

    private:
        bool internalStop();
        RamshCommunicationChannelDLT m_dltChannel;
    };

}

#endif
