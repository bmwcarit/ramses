//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRESOURCESTORAGECHANGELISTENER_H
#define RAMSES_IRESOURCESTORAGECHANGELISTENER_H

namespace ramses_internal
{
    class IResourceStorageChangeListener
    {
    public:
        virtual ~IResourceStorageChangeListener() {};
        virtual void onBytesNeededByStorageDecreased(uint64_t bytesNowUsed) = 0;
    };
}

#endif
