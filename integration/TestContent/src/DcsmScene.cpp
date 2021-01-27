//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/DcsmScene.h"
#include "ramses-client-api/Scene.h"
#include "ramses-framework-api/DcsmProvider.h"

namespace ramses_internal
{
    DcsmScene::DcsmScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, ramses::DcsmProvider& dcsmProvider, uint32_t vpWidth, uint32_t vpHeight)
        : MultipleTrianglesScene(scene, state, cameraPosition, vpWidth, vpHeight)
        , m_dcsmProvider(dcsmProvider)
    {
    }

    void DcsmScene::dispatchHandler()
    {
        m_dcsmProvider.dispatchEvents(*this);
    }

    void DcsmScene::contentReadyRequested(ramses::ContentID contentId)
    {
        m_dcsmProvider.markContentReady(contentId);
    }
}
