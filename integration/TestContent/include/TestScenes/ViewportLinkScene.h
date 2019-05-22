//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VIEWPORTLINKSCENE_H
#define RAMSES_VIEWPORTLINKSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses_internal
{
    class ViewportLinkScene : public IntegrationScene
    {
    public:
        ViewportLinkScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            VIEWPORT_PROVIDER = 0,
            VIEWPORT_CONSUMER,
        };

        static const ramses::dataProviderId_t ViewportOffsetProviderId = 151u;
        static const ramses::dataProviderId_t ViewportSizeProviderId = 152u;
        static const ramses::dataConsumerId_t ViewportOffsetConsumerId = 153u;
        static const ramses::dataConsumerId_t ViewportSizeConsumerId = 154u;

    private:
        void setUpConsumerScene();
        void setUpProviderScene();
    };
}

#endif
