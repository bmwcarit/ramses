//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

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
    class Geometry;
    class RenderGroup;
    class RenderPass;
    class RenderBuffer;
    class RenderTarget;
    class DataObject;
    class BlitPass;
    class TextureSampler;
    class TextureSamplerMS;
    class TextureSamplerExternal;
    class Texture2D;
    class Texture3D;
    class TextureCube;
    class PickableObject;
    class SceneReference;
    class ArrayResource;
    class LogicEngine;

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
        LogicEngine,
        Node,
        MeshNode,
        PerspectiveCamera,
        OrthographicCamera,
        Appearance,
        Geometry,
        RenderGroup,
        RenderPass,
        BlitPass,
        TextureSampler,
        TextureSamplerMS,
        TextureSamplerExternal,
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
        LogicEngine,
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
        TextureSamplerExternal,
        RenderBuffer,
        RenderTarget,
        ArrayBuffer,
        Texture2DBuffer,
        Geometry,
        DataObject,
        PickableObject,
        SceneReference>;
}
