//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief RamsesObject type ID
    */
    enum class ERamsesObjectType
    {
        Invalid,
        ClientObject, // base type
        RamsesObject, // base type
        SceneObject, // base type
        Client,
        Scene,
        LogicEngine,
        LogicObject, // base type
        Node, // base type and concrete type
        MeshNode,
        Camera, // base type
        PerspectiveCamera,
        OrthographicCamera,
        Effect,
        Appearance,
        Geometry,
        PickableObject,
        Resource, // base type
        Texture2D,
        Texture3D,
        TextureCube,
        ArrayResource,
        RenderGroup,
        RenderPass,
        BlitPass,
        TextureSampler,
        TextureSamplerMS,
        RenderBuffer,
        RenderTarget,
        ArrayBuffer,
        Texture2DBuffer,
        DataObject,
        SceneReference,

        TextureSamplerExternal,
    };
}
