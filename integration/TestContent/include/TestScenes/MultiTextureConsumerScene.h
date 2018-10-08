//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTITEXTURECONSUMERSCENE_H
#define RAMSES_MULTITEXTURECONSUMERSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class TriangleStripQuad;
    class Vector2fArray;
    class Texture2D;
    class Effect;
}

namespace ramses_internal
{
    class MultiTextureConsumerScene : public IntegrationScene
    {
    public:
        MultiTextureConsumerScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            THREE_CONSUMERS = 0,
        };

    private:
        void createQuadWithTextureConsumer(ramses::TriangleStripQuad& quad, uint32_t quadId, ramses::dataConsumerId_t textureConsumerId, const ramses::Texture2D& texture, const ramses::Vector2fArray& textureCoords);

        ramses::Effect* m_effectTex;
    };
}

#endif
