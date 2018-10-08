//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMUNICATIONCHANNEL_H
#define RAMSES_RAMSHCOMMUNICATIONCHANNEL_H

namespace ramses_internal
{
    class Ramsh;

    class RamshCommunicationChannel
    {
    public:
        RamshCommunicationChannel();
        virtual ~RamshCommunicationChannel();
        virtual void registerRamsh(Ramsh& ramsh);

    protected:
        Ramsh* m_ramsh;
    };

}// namespace ramses_internal

#endif
