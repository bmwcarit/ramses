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
    /// RamsesObject type ID
    enum ERamsesObjectType
    {
        ERamsesObjectType_Invalid = 0,
        ERamsesObjectType_ClientObject, // base type
        ERamsesObjectType_RamsesObject, // base type
        ERamsesObjectType_SceneObject, // base type
        ERamsesObjectType_Client,
        ERamsesObjectType_Scene,
        ERamsesObjectType_Node, // base type
        ERamsesObjectType_MeshNode,
        ERamsesObjectType_Camera,
        ERamsesObjectType_PerspectiveCamera,
        ERamsesObjectType_OrthographicCamera,
        ERamsesObjectType_Effect,
        ERamsesObjectType_Appearance,
        ERamsesObjectType_GeometryBinding,
        ERamsesObjectType_PickableObject,
        ERamsesObjectType_Resource, // base type
        ERamsesObjectType_Texture2D,
        ERamsesObjectType_Texture3D,
        ERamsesObjectType_TextureCube,
        ERamsesObjectType_ArrayResource,
        ERamsesObjectType_RenderGroup,
        ERamsesObjectType_RenderPass,
        ERamsesObjectType_BlitPass,
        ERamsesObjectType_TextureSampler,
        ERamsesObjectType_TextureSamplerMS,
        ERamsesObjectType_RenderBuffer,
        ERamsesObjectType_RenderTarget,
        ERamsesObjectType_ArrayBufferObject,
        ERamsesObjectType_Texture2DBuffer,
        ERamsesObjectType_DataObject, // base type
        ERamsesObjectType_DataFloat,
        ERamsesObjectType_DataVector2f,
        ERamsesObjectType_DataVector3f,
        ERamsesObjectType_DataVector4f,
        ERamsesObjectType_DataMatrix22f,
        ERamsesObjectType_DataMatrix33f,
        ERamsesObjectType_DataMatrix44f,
        ERamsesObjectType_DataInt32,
        ERamsesObjectType_DataVector2i,
        ERamsesObjectType_DataVector3i,
        ERamsesObjectType_DataVector4i,
        ERamsesObjectType_SceneReference,

        ERamsesObjectType_TextureSamplerExternal,
        // Whenever new type of object is added
        // its traits must be registered in RamsesObjectTypeTraits.h using helper macros
        // and added to appropriate test type list(s) in RamsesObjectTestTypes.h
        // and added a conversion template instantiation in RamsesObjectTypeUtils.cpp
        ERamsesObjectType_NUMBER_OF_TYPES
    };
}

#endif
