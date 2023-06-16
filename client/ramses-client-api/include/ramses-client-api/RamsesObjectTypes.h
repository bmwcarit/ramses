//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTTYPES_H
#define RAMSES_RAMSESOBJECTTYPES_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"

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
        Node, // base type
        MeshNode,
        Camera, // base type
        PerspectiveCamera,
        OrthographicCamera,
        Effect,
        Appearance,
        GeometryBinding,
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
        ArrayBufferObject,
        Texture2DBuffer,
        DataObject,
        SceneReference,

        TextureSamplerExternal,
        // Whenever new type of object is added
        // its traits must be registered in RamsesObjectTypeTraits.h using helper macros
        // and added to appropriate test type list(s) in RamsesObjectTestTypes.h
        // and added a conversion template instantiation in RamsesObjectTypeUtils.cpp
        NUMBER_OF_TYPES
    };
}

#endif
