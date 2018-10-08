//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DYNAMICQUAD_SCENERESOURCES_H
#define RAMSES_DYNAMICQUAD_SCENERESOURCES_H

#include "DynamicQuad_Base.h"

namespace ramses
{
    class Texture2DBuffer;
    class TextureSampler;
    class IndexDataBuffer;
    class VertexDataBuffer;
}

namespace ramses_internal
{
    class DynamicQuad_SceneResources : public DynamicQuad_Base
    {
    public:
        DynamicQuad_SceneResources(ramses::RamsesClient& client, ramses::Scene& scene, const ScreenspaceQuad& screenspaceQuad);
        ~DynamicQuad_SceneResources();

        virtual void recreate() override final;

        // Needed for proper clean-up upon destruction (destroy scene -> mark objects destroyed -> destroy client resources)
        void markSceneObjectsDestroyed();

    private:

        ramses::Texture2DBuffer* m_textureBuffer = nullptr;
        ramses::TextureSampler* m_textureSampler = nullptr;
        ramses::IndexDataBuffer* m_indices = nullptr;
        ramses::VertexDataBuffer* m_texCoords = nullptr;
        ramses::VertexDataBuffer* m_vertexPos = nullptr;

        static const uint32_t DynamicTextureWidth = 64u;
        static const uint32_t DynamicTextureHeight = 32u;
    };
}

#endif
