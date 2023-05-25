//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURELINKSCENE_H
#define RAMSES_TEXTURELINKSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class TextureSampler;
    class Appearance;
    class Texture2D;
}

namespace ramses_internal
{
    class TextureLinkScene : public IntegrationScene
    {
    public:
        TextureLinkScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            DATA_PROVIDER = 0,
            DATA_PROVIDER_LARGE,
            DATA_CONSUMER,
            DATA_CONSUMER_AND_PROVIDER_LARGE,
            DATA_CONSUMER_AND_PROVIDER,
            DATA_CONSUMER_MS
        };

        static constexpr const ramses::dataProviderId_t DataProviderId{151u};
        static constexpr const ramses::dataConsumerId_t DataConsumerId{152u};

    private:
        const ramses::TextureSampler& createSampler(const ramses::Texture2D& texture);
        void                          setSampler(ramses::Appearance& appearance, const ramses::TextureSampler& sampler);
    };
}

#endif
