//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/IResource.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include <memory>

namespace ramses::internal
{
    using ManagedResource = std::shared_ptr<const IResource>;
    using ManagedResourceVector = std::vector<ManagedResource>;
}
