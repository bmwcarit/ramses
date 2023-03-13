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
    static const char* const RamsesObjectTypeNames[] =
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
        "ERamsesObjectType_DataBufferObject",
        "ERamsesObjectType_Texture2DBuffer",
        "ERamsesObjectType_DataObject",
        "ERamsesObjectType_DataFloat",
        "ERamsesObjectType_DataVector2f",
        "ERamsesObjectType_DataVector3f",
        "ERamsesObjectType_DataVector4f",
        "ERamsesObjectType_DataMatrix22f",
        "ERamsesObjectType_DataMatrix33f",
        "ERamsesObjectType_DataMatrix44f",
        "ERamsesObjectType_DataInt32",
        "ERamsesObjectType_DataVector2i",
        "ERamsesObjectType_DataVector3i",
        "ERamsesObjectType_DataVector4i",
        "ERamsesObjectType_SceneReference",
        "ERamsesObjectType_TextureSamplerExternal"
    };

    ENUM_TO_STRING(ERamsesObjectType, RamsesObjectTypeNames, ERamsesObjectType_NUMBER_OF_TYPES);

    static_assert(ERamsesObjectType_NUMBER_OF_TYPES == (sizeof(RamsesObjectTraits) / sizeof(RamsesObjectTraits[0])), "Every RamsesObject type must register its traits!");

    const char* RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType type)
    {
        return EnumToString(type);
    }

    bool RamsesObjectTypeUtils::IsTypeMatchingBaseType(ERamsesObjectType type, ERamsesObjectType baseType)
    {
        while (type != ERamsesObjectType_Invalid)
        {
            if (type == baseType)
            {
                return true;
            }
            assert(RamsesObjectTraits[type].typeID == type && "Wrong order of RamsesObject traits!");
            type = RamsesObjectTraits[type].baseClassTypeID;
        }

        return false;
    }

    bool RamsesObjectTypeUtils::IsConcreteType(ERamsesObjectType type)
    {
        assert(RamsesObjectTraits[type].typeID == type && "Wrong order of RamsesObject traits!");
        return RamsesObjectTraits[type].isConcreteType;
    }
}
