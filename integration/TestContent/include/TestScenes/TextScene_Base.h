//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTSCENE_BASE_H
#define RAMSES_TEXTSCENE_BASE_H

#include "IntegrationScene.h"
#include "ramses-text-api/FontInstanceId.h"
#include "ramses-text-api/TextLine.h"
#include "ramses-text-api/FontRegistry.h"
#include "ramses-text-api/TextCache.h"

namespace ramses
{
    class OrthographicCamera;
}

namespace ramses_internal
{
    class TextScene_Base : public IntegrationScene
    {
    public:
        TextScene_Base(ramses::Scene& scene, const Vector3& cameraPosition, uint32_t vpWidth = IntegrationScene::DefaultViewportWidth, uint32_t vpHeight = IntegrationScene::DefaultViewportHeight);

    protected:
        ramses::FontRegistry        m_fontRegistry;
        ramses::TextCache           m_textCache;
        ramses::OrthographicCamera* m_textOrthoCamera;
    };
}

#endif
