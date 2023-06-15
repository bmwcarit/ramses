//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAINSTANCEHELPER_H
#define RAMSES_DATAINSTANCEHELPER_H

#include "SceneAPI/Handles.h"
#include "DataTypesImpl.h"

#include "PlatformAbstraction/VariantWrapper.h"

namespace ramses_internal
{
    class IScene;

    using DataInstanceValueVariant = std::variant<
        std::monostate,
        float,
        int32_t,
        glm::vec2,
        glm::vec3,
        glm::vec4,
        glm::ivec2,
        glm::ivec3,
        glm::ivec4,
        glm::mat2,
        glm::mat3,
        glm::mat4>;

    class DataInstanceHelper
    {
    public:
        static void GetInstanceFieldData(const IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, DataInstanceValueVariant& value);
        static void SetInstanceFieldData(IScene& scene, DataInstanceHandle dataInstance, DataFieldHandle dataField, const DataInstanceValueVariant& value);
    };
}

#endif
