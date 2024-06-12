//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/SceneSizeInformation.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"

#include <string>
#include <string_view>

namespace ramses::internal
{
    struct SceneCreationInformation
    {
        explicit SceneCreationInformation(
            SceneInfo sceneInfo = SceneInfo{},
            SceneSizeInformation sizeInfo = SceneSizeInformation())
            : m_sceneInfo(std::move(sceneInfo))
            , m_sizeInfo(std::move(sizeInfo))
        {
        }

        SceneInfo               m_sceneInfo;
        SceneSizeInformation    m_sizeInfo;
    };
}
