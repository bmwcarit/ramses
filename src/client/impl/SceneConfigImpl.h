//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/EScenePublicationMode.h"

namespace ramses::internal
{
    class SceneConfigImpl
    {
    public:
        void setPublicationMode(EScenePublicationMode publicationMode);
        void setMemoryVerificationEnabled(bool enabled);
        void setSceneId(sceneId_t sceneId);

        [[nodiscard]] EScenePublicationMode getPublicationMode() const;
        [[nodiscard]] bool getMemoryVerificationEnabled() const;
        [[nodiscard]] sceneId_t getSceneId() const;

    private:
        EScenePublicationMode m_publicationMode = EScenePublicationMode::LocalOnly;
        sceneId_t m_sceneId;
        bool m_memoryVerificationEnabled = true;
    };
}
