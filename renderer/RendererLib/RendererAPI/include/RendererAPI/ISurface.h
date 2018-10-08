//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISURFACE_H
#define RAMSES_ISURFACE_H

#include "Types.h"

namespace ramses_internal
{
    class IWindow;
    class IContext;

    class ISurface
    {
    public:
        virtual ~ISurface(){}

        virtual Bool enable() = 0;
        virtual Bool disable() = 0;
        virtual Bool swapBuffers() = 0;
        virtual Bool canRenderNewFrame() const = 0;
        virtual void frameRendered() = 0;

        virtual const IWindow& getWindow() const = 0;
        virtual IWindow& getWindow() = 0;
        virtual const IContext& getContext() const = 0;
        virtual IContext& getContext() = 0;

    };
}

#endif
