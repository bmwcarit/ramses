//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MULTITYPELINKSCENE_H
#define RAMSES_MULTITYPELINKSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class DataObject;
    class TextureSampler;
    class Appearance;
    class Texture2D;
}

namespace ramses_internal
{
    class MultiTypeLinkScene : public IntegrationScene
    {
    public:
        MultiTypeLinkScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            TRANSFORMATION_CONSUMER_DATA_AND_TEXTURE_PROVIDER = 0,
            TRANSFORMATION_PROVIDER_DATA_AND_TEXTURE_CONSUMER
        };

        static const ramses::dataProviderId_t TransformationProviderId = 121u;
        static const ramses::dataConsumerId_t TransformationConsumerId = 122u;
        static const ramses::dataProviderId_t DataProviderId = 1251u;
        static const ramses::dataConsumerId_t DataConsumerId = 1252u;
        static const ramses::dataProviderId_t TextureProviderId = 31251u;
        static const ramses::dataConsumerId_t TextureConsumerId = 31252u;

    private:
        const ramses::TextureSampler& createSampler(const ramses::Texture2D& texture);
        void                          setSampler(ramses::Appearance& appearance, const ramses::TextureSampler& sampler);
    };
}

#endif
