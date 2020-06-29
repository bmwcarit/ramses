//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_OVERLAYCLIENT_H
#define RAMSES_OVERLAYCLIENT_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class RamsesFramework;
    class RamsesClient;
    class Scene;
}

/* This class contains a RamsesClient which provides overlays
 *   Creates and publishes two small text scenes with data consumers which allow to control their
 *   viewport offset.
 */
class OverlayClient
{
public:
    explicit OverlayClient(ramses::RamsesFramework& framework);
    ~OverlayClient();

    void createOverlayScenes();

    constexpr static ramses::sceneId_t Overlay1SceneId{ 10001 };
    constexpr static ramses::sceneId_t Overlay2SceneId{ 10002 };

    constexpr static ramses::dataConsumerId_t Overlay1ViewportOffsetId{ 100011 };
    constexpr static ramses::dataConsumerId_t Overlay2ViewportOffsetId{ 100021 };

private:
    ramses::RamsesFramework& m_framework;
    ramses::RamsesClient& m_client;

    ramses::Scene* m_overlay1Scene = nullptr;
    ramses::Scene* m_overlay2Scene = nullptr;
};

#endif
