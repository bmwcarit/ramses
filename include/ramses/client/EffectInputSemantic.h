//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief Effect uniform semantic type
    */
    enum class EEffectUniformSemantic
    {
        Invalid = 0,                ///< Invalid semantic

        ProjectionMatrix,           ///< Projection matrix 4x4
        ModelMatrix,                ///< Mesh model matrix 4x4
        CameraWorldPosition,        ///< Camera position vector 3 (camera world position of the camera used in current render pass)
        ViewMatrix,                 ///< View matrix 4x4
        ModelViewMatrix,            ///< Model-view matrix 4x4
        ModelViewMatrix33,          ///< Model-view matrix 3x3
        ModelViewProjectionMatrix,  ///< Model-view-projection matrix 4x4
        NormalMatrix,               ///< Transposed and inversed MVP matrix for vertex normals
        DisplayBufferResolution,    ///< Resolution of currently set destination display buffer (either display framebuffer or offscreen buffer, does not give RenderTarget resolution)
        TextTexture,                ///< Text specific
        TimeMs,                     ///< synchronized clock in milliseconds, resets to 0 every ~24 days, i.e. value range is: 0 .. std::numeric_limits<int32_t>::max().
                                    ///< In order to avoid handling the wrap in the shader code or potential overflow issues the value should be reset using #ramses::Scene::resetUniformTimeMs().
                                    ///< The value is not reset automatically at startup, but contains the time elapsed since clock epoch.

        ModelBlock,                 ///< Uniform buffer containing: ModelMatrix (mat44)
                                    ///< - exists per MeshNode and is updated whenever MeshNode transformation changes
                                    ///< Declaration example in shader:
                                    ///<   layout(std140, binding = 1) uniform uniformBlock_t
                                    ///<   {
                                    ///<       mat4 modelMat;
                                    ///<   } myModelUBO;

        CameraBlock,                ///< Uniform buffer containing (in this order): ProjectionMatrix(mat44), ViewMatrix(mat44), CameraWorldPosition(vec3)
                                    ///< - exists per Camera and is updated whenever Camera transformation or view/projection parameters change
                                    ///< Declaration example in shader:
                                    ///<   layout(std140, binding = 1) uniform uniformBlock_t
                                    ///<   {
                                    ///<       mat4 projMat;
                                    ///<       mat4 viewMat;
                                    ///<       vec3 camPos;
                                    ///<   } myCameraUBO;

        ModelCameraBlock,           ///< Uniform buffer containing (in this order): ModelViewProjectionMatrix(mat44), ModelViewMatrix(mat44), NormalMatrix(mat44)
                                    ///< - exists per every Camera/MeshNode combination in render passes and is updated whenever Camera or MeshNode transformation changes or Camera view/projection parameters change
                                    ///< Declaration example in shader:
                                    ///<   layout(std140, binding = 1) uniform uniformBlock_t
                                    ///<   {
                                    ///<       mat4 mvpMat;
                                    ///<       mat4 mvMat;
                                    ///<       mat4 normalMat;
                                    ///<   } myModelCameraUBO;

        FramebufferBlock,           ///< Uniform buffer containing: FramebufferResolution(vec2)
                                    ///< - exists per RenderTarget and is never updated
                                    ///< Declaration example in shader:
                                    ///<   layout(std140, binding = 1) uniform uniformBlock_t
                                    ///<   {
                                    ///<       vec2 resolution;
                                    ///<   } myFramebufferInfoUBO;

        SceneBlock,                 ///< Uniform buffer containing: TimeMs(int)
                                    ///< - exists per Scene and is updated once per frame
                                    ///< Declaration example in shader:
                                    ///<   layout(std140, binding = 1) uniform uniformBlock_t
                                    ///<   {
                                    ///<       int time;
                                    ///<   } mySceneInfoUBO;
    };

    /**
    * @ingroup CoreAPI
    * @brief Effect attribute semantic type
    */
    enum class EEffectAttributeSemantic
    {
        Invalid = 0,                 ///< Invalid semantic
        TextPositions,               ///< Text specific - vertex positions input. MUST be of type vec2
        TextTextureCoordinates       ///< Text specific - texture coordinates input. MUST be of type vec2
    };
}
