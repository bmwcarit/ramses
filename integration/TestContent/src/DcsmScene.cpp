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
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    DcsmScene::DcsmScene(ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition, ramses::DcsmProvider& dcsmProvider, uint32_t vpWidth, uint32_t vpHeight)
        : MultipleTrianglesScene(scene, state, cameraPosition, vpWidth, vpHeight)
        , m_dcsmProvider(dcsmProvider)
    {
    }

    void DcsmScene::contentStatus(ramses::ContentID id, ramses::DcsmStatusMessage const& msg)
    {
        if (auto statusMsg = msg.getAsStreamStatus())
            LOG_INFO_P(CONTEXT_FRAMEWORK, "DcsmScene::contentStatus: received DcsmStatusMessage of type StreamStatus with value {} for content {}", statusMsg->getStreamStatus(), id.getValue());
        else if (auto layoutMsg = msg.getAsActiveLayout())
            LOG_INFO_P(CONTEXT_FRAMEWORK, "DcsmScene::contentStatus: received DcsmStatusMessage of type ActiveLayout with value {} for content {}", layoutMsg->getLayout(), id.getValue());
        else if (auto focusMsg = msg.getAsWidgetFocusStatus())
            LOG_INFO_P(CONTEXT_FRAMEWORK, "DcsmScene::contentStatus: received DcsmStatusMessage of type WidgetFocusStatus with value {} for content {}", focusMsg->getWidgetFocusStatus(), id.getValue());
        else
            LOG_INFO_P(CONTEXT_FRAMEWORK, "DcsmScene::contentStatus: received unknown DcsmStatusMessage for content {}", id.getValue());
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
