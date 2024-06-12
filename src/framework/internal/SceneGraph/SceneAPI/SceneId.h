//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "internal/Core/Common/StronglyTypedValue.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include "EVulkanVersion.h"

#include "ramses/framework/ERenderBackendCompatibility.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace ramses::internal
{
    struct SceneIdTag {};
    using SceneId = StronglyTypedValue<uint64_t, 0, SceneIdTag>;

    using SceneIdVector = std::vector<SceneId>;

    struct SceneInfo
    {
        SceneId                     sceneID;
        std::string                 friendlyName                {};
        EScenePublicationMode       publicationMode             = EScenePublicationMode::LocalAndRemote;
        ERenderBackendCompatibility renderBackendCompatibility  = ERenderBackendCompatibility::OpenGL;
        EVulkanAPIVersion           vulkanAPIVersion            = EVulkanAPIVersion::Invalid;
        ESPIRVVersion               spirvVersion                = ESPIRVVersion::Invalid;
    };
    using SceneInfoVector = std::vector<SceneInfo>;
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses::internal::SceneId)
