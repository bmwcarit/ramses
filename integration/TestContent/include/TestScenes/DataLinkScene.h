//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATALINKSCENE_H
#define RAMSES_DATALINKSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses_internal
{
    class DataLinkScene : public IntegrationScene
    {
    public:
        DataLinkScene(ramses::Scene& scene, UInt32 state, const glm::vec3& cameraPosition);

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

#endif
