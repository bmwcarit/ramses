//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENECREATIONINFORMATION_H
#define RAMSES_SCENECREATIONINFORMATION_H

#include "SceneAPI/SceneSizeInformation.h"
#include "SceneAPI/SceneId.h"
#include "Collections/String.h"

namespace ramses_internal
{
    struct SceneCreationInformation
    {
        SceneCreationInformation(
            SceneId id = SceneId(),
            const String& name = String(),
            const SceneSizeInformation& sizeInfo = SceneSizeInformation())
            : m_id(id)
            , m_name(name)
            , m_sizeInfo(sizeInfo)
        {
        }

        SceneId              m_id;
        String               m_name;
        SceneSizeInformation m_sizeInfo;
    };
}

#endif
