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
    enum class EEffectUniformSemantic
    {
        Invalid = 0,                 ///< Invalid semantic
        ProjectionMatrix,            ///< Projection matrix 4x4
        ModelMatrix,                 ///< Mesh model matrix 4x4
        CameraWorldPosition,         ///< Camera position vector 3
                                                            ///< ^ Position, from where the renderer eye looks at the scene in world coordinates
        ViewMatrix,                  ///< View matrix 4x4
        ModelViewMatrix,             ///< Model-view matrix 4x4
        ModelViewMatrix33,           ///< Model-view matrix 3x3
        ModelViewProjectionMatrix,   ///< Model-view-projection matrix 4x4
        NormalMatrix,                ///< Transposed and inversed MVP matrix for vertex normals
        DisplayBufferResolution,     ///< Resolution of currently set destination display buffer (either display framebuffer or offscreen buffer, does not give RenderTarget resolution)

        TextTexture                  ///< Text specific
    };

    /**
    * @brief Effect attribute semantic type
    */
    enum class EEffectAttributeSemantic
    {
        Invalid = 0,                 ///< Invalid semantic
        TextPositions,               ///< Text specific - vertex positions input. MUST be of type vec2
        TextTextureCoordinates       ///< Text specific - texture coordinates input. MUST be of type vec2
    };
}

#endif
