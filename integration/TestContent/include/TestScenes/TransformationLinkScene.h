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
        TransformationLinkScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

        enum
        {
            TRANSFORMATION_PROVIDER = 0,
            TRANSFORMATION_PROVIDER_WITHOUT_CONTENT,
            TRANSFORMATION_CONSUMER,
            TRANSFORMATION_CONSUMER_AND_PROVIDER,
            TRANSFORMATION_CONSUMER_OVERRIDEN,
        };

        static constexpr const ramses::dataProviderId_t transformProviderDataId{51u};
        static constexpr const ramses::dataProviderId_t transformProviderDataId_Left{52u};
        static constexpr const ramses::dataProviderId_t transformProviderDataId_Right{53u};
        static constexpr const ramses::dataConsumerId_t transformConsumerDataId{54u};

    private:
        ramses::MeshNode* createTriangleMesh(ramses::TriangleAppearance::EColor color);
    };
}

#endif
