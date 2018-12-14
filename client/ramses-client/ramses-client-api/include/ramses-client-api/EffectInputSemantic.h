//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EFFECTINPUTSEMANTIC_H
#define RAMSES_EFFECTINPUTSEMANTIC_H

namespace ramses
{
    /**
    * @brief Effect uniform semantic type
    */
    enum EEffectUniformSemantic
    {
        EEffectUniformSemantic_Invalid = 0,                 /// Invalid semantic
        EEffectUniformSemantic_ProjectionMatrix,            /// Projection matrix 4x4
        EEffectUniformSemantic_ModelMatrix,                 /// Mesh model matrix 4x4
        EEffectUniformSemantic_RendererViewMatrix,          /// Renderer view matrix 4x4
        EEffectUniformSemantic_CameraViewMatrix,            /// Camera view matrix 4x4
        EEffectUniformSemantic_CameraWorldPosition,         /// Camera position vector 3
                                                            /// ^ Position, from where the renderer eye looks at the scene in world coordinates
        EEffectUniformSemantic_ViewMatrix,                  /// View matrix 4x4
        EEffectUniformSemantic_ModelViewMatrix,             /// Model-view matrix 4x4
        EEffectUniformSemantic_ModelViewMatrix33,           /// Model-view matrix 3x3
        EEffectUniformSemantic_ModelViewProjectionMatrix,   /// Model-view-projection matrix 4x4
        EEffectUniformSemantic_NormalMatrix,                /// transposed and inverse of mvp for vertex normals
        EEffectUniformSemantic_RendererScreenResolution,    /// Resolution of renderer display vector 2 (width, height)

        EEffectUniformSemantic_TextTexture                  /// Text specific - texture input for font characters
    };

    /**
    * @brief Effect attribute semantic type
    */
    enum EEffectAttributeSemantic
    {
        EEffectAttributeSemantic_Invalid = 0,                 /// Invalid semantic
        EEffectAttributeSemantic_TextPositions,               /// Text specific - vertex positions input. MUST be of type vec2
        EEffectAttributeSemantic_TextTextureCoordinates       /// Text specific - texture coordinates input. MUST be of type vec2
    };
}

#endif
