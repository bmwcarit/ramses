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
#include "DataTypesImpl.h"
#include <array>

namespace ramses
{
    class Node;
}

namespace ramses_internal
{
    /// Scene, that creates a cube with six stream textures.
    class StreamTextureScene : public IntegrationScene
    {
    public:
        StreamTextureScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            INITIAL_STATE = 0,
            MULTI_SOURCE_SAME_FALLBACK_TEXTURE,
            SAME_SOURCE_MULTI_FALLBACK,
            MULTI_SOURCE_MULTI_FALLBACK
        };

        static constexpr std::array<ramses::dataConsumerId_t, 6> TextureConsumers{ ramses::dataConsumerId_t{ 1u },
                                                                                    ramses::dataConsumerId_t{ 2u },
                                                                                    ramses::dataConsumerId_t{ 3u },
                                                                                    ramses::dataConsumerId_t{ 4u },
                                                                                    ramses::dataConsumerId_t{ 5u },
                                                                                    ramses::dataConsumerId_t{ 6u },
                                                                                 };

        static constexpr std::array<uint32_t, 6> getConsumersLinkingIndices(UInt32 state)
        {
            switch(state)
            {
            case INITIAL_STATE:
            case MULTI_SOURCE_SAME_FALLBACK_TEXTURE:
                return { 0u, 1u, 2u, 3u, 4u, 5u };
            case SAME_SOURCE_MULTI_FALLBACK:
                return { 1u, 1u, 1u, 1u, 1u, 1u };
            case MULTI_SOURCE_MULTI_FALLBACK:
                return { 1u, 1u, 3u, 3u, 1u, 5u };
            }

            return {};
        }

    private:
        void addPngQuad(const char* pngFilePath, const std::array<ramses::vec3f, 4u>& vertexPositionsArray, ramses::dataConsumerId_t consumerId, ramses::Node* parentNode = nullptr);

        const ramses::Effect* m_effect;
        ramses::Node* m_root;
    };
}

#endif
