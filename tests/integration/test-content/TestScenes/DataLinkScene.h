//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "IntegrationScene.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

namespace ramses::internal
{
    class DataLinkScene : public IntegrationScene
    {
    public:
        DataLinkScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition);

        enum
        {
            DATA_PROVIDER = 0,
            DATA_CONSUMER,
            DATA_CONSUMER_AND_PROVIDER
        };

        static constexpr const ramses::dataProviderId_t DataProviderId{151u};
        static constexpr const ramses::dataConsumerId_t DataConsumerId{152u};
    };
}
