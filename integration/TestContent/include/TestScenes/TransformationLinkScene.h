//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TRANSFORMATIONLINKSCENE_H
#define RAMSES_TRANSFORMATIONLINKSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "TestScenes/Triangle.h"

namespace ramses_internal
{
    class TransformationLinkScene : public IntegrationScene
    {
    public:
        TransformationLinkScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            TRANSFORMATION_PROVIDER = 0,
            TRANSFORMATION_PROVIDER_WITHOUT_CONTENT,
            TRANSFORMATION_CONSUMER,
            TRANSFORMATION_CONSUMER_AND_PROVIDER,
            TRANSFORMATION_CONSUMER_OVERRIDEN,
        };

        static constexpr ramses::dataProviderId_t transformProviderDataId{51u};
        static constexpr ramses::dataProviderId_t transformProviderDataId_Left{52u};
        static constexpr ramses::dataProviderId_t transformProviderDataId_Right{53u};
        static constexpr ramses::dataConsumerId_t transformConsumerDataId{54u};

    private:
        ramses::MeshNode* createTriangleMesh(ramses::TriangleAppearance::EColor color);
    };
}

#endif
