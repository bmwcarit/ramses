//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEFILEDESCRIPTIONSETIMPL_H
#define RAMSES_RESOURCEFILEDESCRIPTIONSETIMPL_H

#include "ramses-client-api/Resource.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "PlatformAbstraction/PlatformSharedPointer.h"
#include "Collections/Vector.h"
#include "Collections/String.h"

namespace ramses
{
    typedef std::vector<ResourceFileDescription> ResourceFileDescriptionVector;

    class ResourceFileDescriptionSetImpl
    {
    public:
        ResourceFileDescriptionVector descriptions;

        std::vector<ramses_internal::String> getFilenames() const
        {
            std::vector<ramses_internal::String> result;
            result.reserve(descriptions.size());
            for (const auto& desc : descriptions)
            {
                result.push_back(desc.getFilename());
            }
            return result;
        }
    };
}

#endif //RAMSES_RESOURCEFILEDESCRIPTIONSETIMPL_H
