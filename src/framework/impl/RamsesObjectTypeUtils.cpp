//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesObjectTypeUtils.h"
#include "internal/Core/Utils/LoggingUtils.h"

namespace ramses::internal
{
    const std::array RamsesObjectTypeNames =
    {
        "Invalid",
        "ClientObject",
        "RamsesObject",
        "SceneObject",
        "Client",
        "Scene",
        "LogicEngine",
        "LogicObject",
        "Node",
        "MeshNode",
        "Camera",
        "PerspectiveCamera",
        "OrthographicCamera",
        "Effect",
        "Appearance",
        "Geometry",
        "PickableObject",
        "Resource",
        "Texture2D",
        "Texture3D",
        "TextureCube",
        "ArrayResource",
        "RenderGroup",
        "RenderPass",
        "BlitPass",
        "TextureSampler",
        "TextureSamplerMS",
        "RenderBuffer",
        "RenderTarget",
        "ArrayBuffer",
        "Texture2DBuffer",
        "DataObject",
        "SceneReference",
        "TextureSamplerExternal"
    };

    ENUM_TO_STRING(ERamsesObjectType, RamsesObjectTypeNames, ERamsesObjectType::TextureSamplerExternal);

    const char* RamsesObjectTypeUtils::GetRamsesObjectTypeName(ERamsesObjectType type)
    {
        return EnumToString(type);
    }
}
