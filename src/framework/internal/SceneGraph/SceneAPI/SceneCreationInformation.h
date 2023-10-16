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
            SceneId id = SceneId(),
            std::string_view name = {},
            const SceneSizeInformation& sizeInfo = SceneSizeInformation())
            : m_id(id)
            , m_name(name)
            , m_sizeInfo(sizeInfo)
        {
        }

        SceneId              m_id;
        std::string          m_name;
        SceneSizeInformation m_sizeInfo;
    };
}
