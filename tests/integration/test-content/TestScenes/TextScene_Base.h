//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#if defined(RAMSES_TEXT_ENABLED)

#include "IntegrationScene.h"
#include "ramses/client/text/FontInstanceId.h"
#include "ramses/client/text/TextLine.h"
#include "ramses/client/text/FontRegistry.h"
#include "ramses/client/text/TextCache.h"

namespace ramses
{
    class OrthographicCamera;
}

namespace ramses::internal
{
    class TextScene_Base : public IntegrationScene
    {
    public:
        TextScene_Base(ramses::Scene& scene, const glm::vec3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

    protected:
        ramses::FontRegistry        m_fontRegistry;
        ramses::TextCache           m_textCache;
        ramses::OrthographicCamera* m_textOrthoCamera;
    };
}

#endif
