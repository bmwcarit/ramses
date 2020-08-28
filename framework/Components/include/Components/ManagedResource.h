//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MANAGEDRESOURCE_H
#define RAMSES_MANAGEDRESOURCE_H

#include "Resource/IResource.h"
#include "Collections/Vector.h"
#include <memory>

namespace ramses_internal
{
    using ManagedResource = std::shared_ptr<const IResource>;
    using ManagedResourceVector = std::vector<ManagedResource>;
}

#endif
