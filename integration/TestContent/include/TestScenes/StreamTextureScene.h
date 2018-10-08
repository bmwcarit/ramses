//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STREAMTEXTURESCENE_H
#define RAMSES_STREAMTEXTURESCENE_H

#include "IntegrationScene.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class Node;
    class RotateNode;
}

namespace ramses_internal
{
    /// Scene, that creates a cube with six stream textures.
    class StreamTextureScene : public IntegrationScene
    {
    public:
        StreamTextureScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            INITIAL_STATE = 0,
            MULTI_SOURCE_SAME_FALLBACK_TEXTURE,
            SAME_SOURCE_MULTI_FALLBACK,
            MULTI_SOURCE_MULTI_FALLBACK,
            FORCE_FALLBACK_ON_SOME_TEXTURES
        };

    private:
        void addPngQuad(const char* pngFilePath, const float* vertexPositionsArray, ramses::streamSource_t surfaceId, ramses::Node* parentNode = nullptr, const char* streamTextureName = "streamTexture", bool forcefallback = false);

        const ramses::Effect* m_effect;
        ramses::RotateNode* m_root;
    };
}

#endif
