//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESOBJECTTESTTYPES_H
#define RAMSES_RAMSESOBJECTTESTTYPES_H

#include <gtest/gtest.h>

namespace ramses
{
    class RamsesClient;
    class Scene;
    class Appearance;
    class Camera;
    class PerspectiveCamera;
    class OrthographicCamera;
    class Appearance;
    class Node;
    class Effect;
    class MeshNode;
    class ArrayBuffer;
    class Texture2DBuffer;
    class GeometryBinding;
    class RenderGroup;
    class RenderPass;
    class RenderBuffer;
    class RenderTarget;
    class DataObject;
    class BlitPass;
    class TextureSampler;
    class TextureSamplerMS;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class PickableObject;
    class SceneReference;
    class ArrayResource;

    // Objects derived from Node class
    using NodeTypes = ::testing::Types<
        Node,
        MeshNode,
        PerspectiveCamera,
        OrthographicCamera,
        PickableObject>;

    // Objects derived from Resource class
    using ResourceTypes = ::testing::Types<
        ArrayResource,
        Texture2D,
        Texture3D,
        TextureCube,
        Effect>;

    // Objects owned by Scene
    using SceneObjectTypes = ::testing::Types<
        Node,
        MeshNode,
        PerspectiveCamera,
        OrthographicCamera,
        Appearance,
        GeometryBinding,
        RenderGroup,
        RenderPass,
        BlitPass,
        TextureSampler,
        TextureSamplerMS,
        RenderBuffer,
        RenderTarget,
        DataObject,
        ArrayBuffer,
        Texture2DBuffer,
        PickableObject,
        SceneReference,
        Texture2D,
        Texture3D,
        TextureCube,
        ArrayResource,
        Effect>;

    // Objects owned by client
    using ClientObjectTypes = ::testing::Types<Scene>;

    // All Ramses objects
    using RamsesObjectTypes = ::testing::Types<
        RamsesClient,
        Scene,
        Node,
        MeshNode,
        PerspectiveCamera,
        OrthographicCamera,
        Effect,
        Appearance,
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
        GeometryBinding,
        DataObject,
        PickableObject,
        SceneReference>;
}

#endif
