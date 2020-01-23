//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesClientTypesImpl.h"
#include "ramses-client-api/ResourceFileDescription.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"

namespace ramses
{
    ramses_internal::StringOutputStream& operator<<(ramses_internal::StringOutputStream& lhs, const ResourceFileDescription& rhs)
    {
        const auto numResources = rhs.getNumberOfResources();
        lhs << "Filename: " << rhs.getFilename() << " ; Resource count " << numResources << ": [ ";
        for (uint32_t i = 0; i < numResources; ++i)
            lhs << (i != 0 ? " ; " : "") << rhs.getResource(i).getResourceId() ;
        lhs << " ]";
        return lhs;
    }

    ramses_internal::StringOutputStream& operator<<(ramses_internal::StringOutputStream& lhs, const ResourceFileDescriptionSet& rhs)
    {
        const auto numDescriptions = rhs.getNumberOfDescriptions();
        lhs << numDescriptions << " Resource File Descriptions: [ ";
        for (uint32_t i = 0; i < numDescriptions; ++i)
            lhs << (i != 0 ? " ; " : "") << rhs.getDescription(i);
        lhs << " ]";
        return lhs;
    }
}
