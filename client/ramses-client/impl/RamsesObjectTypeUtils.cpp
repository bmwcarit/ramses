//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesObjectTypeUtils.h"
#include "Utils/LoggingUtils.h"

namespace ramses
{
    const std::array RamsesObjectTypeNames =
    {
        "ERamsesObjectType_Invalid",
        "ERamsesObjectType_RamsesObject",
        "ERamsesObjectType_ClientObject",
        "ERamsesObjectType_SceneObject",
        "ERamsesObjectType_Client",
        "ERamsesObjectType_Scene",
        "ERamsesObjectType_Node",
        "ERamsesObjectType_MeshNode",
        "ERamsesObjectType_Camera",
        "ERamsesObjectType_PerspectiveCamera",
        "ERamsesObjectType_OrthographicCamera",
        "ERamsesObjectType_Effect",
        "ERamsesObjectType_Appearance",
        "ERamsesObjectType_Geometry",
        "ERamsesObjectType_PickableObject",
        "ERamsesObjectType_Resource",
        "ERamsesObjectType_Texture2D",
        "ERamsesObjectType_Texture3D",
        "ERamsesObjectType_TextureCube",
        "ERamsesObjectType_ArrayResource",
        "ERamsesObjectType_RenderGroup",
        "ERamsesObjectType_RenderPass",
        "ERamsesObjectType_BlitPass",
        "ERamsesObjectType_TextureSampler",
        "ERamsesObjectType_TextureSamplerMS",
        "ERamsesObjectType_RenderBuffer",
        "ERamsesObjectType_RenderTarget",
        "ERamsesObjectType_ArrayBufferObject",
        "ERamsesObjectType_Texture2DBuffer",
        "ERamsesObjectType_DataObject",
        "ERamsesObjectType_SceneReference",
        "ERamsesObjectType_TextureSamplerExternal"
    };

    ENUM_TO_STRING(ERamsesObjectType, RamsesObjectTypeNames, ERamsesObjectType::NUMBER_OF_TYPES);

    static_assert(static_cast<size_t>(ERamsesObjectType::NUMBER_OF_TYPES) == RamsesObjectTraits.size(), "Every RamsesObject type must register its traits!");

    const char* RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType type)
    {
        return EnumToString(type);
    }

    bool RamsesObjectTypeUtils::IsTypeMatchingBaseType(ERamsesObjectType type, ERamsesObjectType baseType)
    {
        while (type != ERamsesObjectType::Invalid)
        {
            if (type == baseType)
            {
                return true;
            }
            const auto index = static_cast<int>(type);
            assert(RamsesObjectTraits[index].typeID == type && "Wrong order of RamsesObject traits!");
            type = RamsesObjectTraits[index].baseClassTypeID;
        }

        return false;
    }

    bool RamsesObjectTypeUtils::IsConcreteType(ERamsesObjectType type)
    {
        const auto index = static_cast<int>(type);
        assert(RamsesObjectTraits[index].typeID == type && "Wrong order of RamsesObject traits!");
        return RamsesObjectTraits[index].isConcreteType;
    }
}
