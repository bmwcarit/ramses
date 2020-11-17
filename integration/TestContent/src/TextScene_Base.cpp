//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/TextScene_Base.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/OrthographicCamera.h"

namespace ramses_internal
{
    TextScene_Base::TextScene_Base(ramses::Scene& scene, const Vector3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene(scene, cameraPosition, vpWidth, vpHeight)
        , m_textCache(scene, m_fontRegistry, 1024u, 1024u)
        , m_textOrthoCamera(m_scene.createOrthographicCamera("text camera"))
    {
        m_textOrthoCamera->setFrustum(0.0f, static_cast<Float>(IntegrationScene::DefaultViewportWidth), 0.0f, static_cast<Float>(IntegrationScene::DefaultViewportHeight), 0.1f, 10.f);
        m_textOrthoCamera->setViewport(0, 0, IntegrationScene::DefaultViewportWidth, IntegrationScene::DefaultViewportHeight);
        setCameraToDefaultRenderPass(m_textOrthoCamera);
    }
}
