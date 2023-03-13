//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CAMERADATALINKSCENE_H
#define RAMSES_CAMERADATALINKSCENE_H

#include "IntegrationScene.h"
#include "SceneAPI/Handles.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses_internal
{
    class CameraDataLinkScene : public IntegrationScene
    {
    public:
        CameraDataLinkScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition);

        enum
        {
            CAMERADATA_PROVIDER = 0,
            CAMERADATA_CONSUMER,
        };

        static constexpr ramses::dataProviderId_t ViewportOffsetProviderId{151u};
        static constexpr ramses::dataProviderId_t ViewportSizeProviderId{152u};
        static constexpr ramses::dataProviderId_t FrustumPlanesProviderId{153u};
        static constexpr ramses::dataProviderId_t FrustumPlanesNearFarProviderId{154u};
        static constexpr ramses::dataConsumerId_t ViewportOffsetConsumerId{1051u};
        static constexpr ramses::dataConsumerId_t ViewportSizeConsumerId{1052u};
        static constexpr ramses::dataConsumerId_t FrustumPlanesConsumerId{1053u};
        static constexpr ramses::dataConsumerId_t FrustumPlanesNearFarConsumerId{1054u};

    private:
        void setUpConsumerScene();
        void setUpProviderScene();
    };
}

#endif
