//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEFILEDESCRIPTIONIMPL_H
#define RAMSES_RESOURCEFILEDESCRIPTIONIMPL_H

#include "ramses-client-api/Resource.h"
#include "Collections/String.h"
#include "ResourceObjects.h"

namespace ramses
{
    class ResourceFileDescriptionImpl
    {
    public:
        ResourceFileDescriptionImpl()
            : m_resources()
        {
        }
        ramses_internal::String m_filename;
        ResourceObjects m_resources;
    };
}

#endif //RAMSES_RESOURCEFILEDESCRIPTIONIMPL_H
