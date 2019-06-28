//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ICONTEXT_H
#define RAMSES_ICONTEXT_H

#include "Types.h"

namespace ramses_internal
{
    class DeviceResourceMapper;

    class IContext
    {
    public:

        enum EType
        {
            EType_EGL,      /// IContext instance is down-castable to ramses_internal::Context_EGL.
            EType_Other,    /// No information about concrete type is available. Do no attempt to cast.
        };

        virtual ~IContext(){}

        virtual DeviceResourceMapper& getResources() = 0;

        // TODO Violin this should be removed - provides access to platform-specific data
        virtual void* getProcAddress(const Char* name) const = 0;

        // Poor-man's RTTI
        virtual EType getType() const
        {
            return EType_Other;
        }
    };
}

#endif
