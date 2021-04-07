//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_INPUTSTREAMCONTAINER_H
#define RAMSES_FRAMEWORK_INPUTSTREAMCONTAINER_H

#include "Collections/IInputStream.h"

namespace ramses_internal
{
    class IInputStreamContainer
    {
    public:
        IInputStreamContainer() = default;
        virtual ~IInputStreamContainer() = default;

        IInputStreamContainer(const IInputStreamContainer&) = delete;
        IInputStreamContainer& operator=(const IInputStreamContainer&) = delete;


        virtual IInputStream& getStream() = 0;
    };

    using InputStreamContainerSPtr = std::shared_ptr<IInputStreamContainer>;
}

#endif
