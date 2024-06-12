//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/SceneConfigImpl.h"

namespace ramses::internal
{
    void SceneConfigImpl::setPublicationMode(EScenePublicationMode publicationMode)
    {
        m_publicationMode = publicationMode;
    }

    EScenePublicationMode SceneConfigImpl::getPublicationMode() const
    {
        return m_publicationMode;
    }

    void SceneConfigImpl::setSceneId(sceneId_t sceneId)
    {
        m_sceneId = sceneId;
    }

    sceneId_t SceneConfigImpl::getSceneId() const
    {
        return m_sceneId;
    }

    void SceneConfigImpl::setMemoryVerificationEnabled(bool enabled)
    {
        m_memoryVerificationEnabled = enabled;
    }

    bool SceneConfigImpl::getMemoryVerificationEnabled() const
    {
        return m_memoryVerificationEnabled;
    }

    void SceneConfigImpl::setRenderBackendCompatibility(ERenderBackendCompatibility renderBackendCompatibility)
    {
        m_renderBackendCompatibility = renderBackendCompatibility;
    }

    ERenderBackendCompatibility SceneConfigImpl::getRenderBackendCompatibility() const
    {
        return m_renderBackendCompatibility;
    }
}
