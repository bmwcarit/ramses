//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENECONFIGIMPL_H
#define RAMSES_SCENECONFIGIMPL_H

#include "StatusObjectImpl.h"
#include "ramses-client-api/EScenePublicationMode.h"
#include <chrono>

namespace ramses
{
    class IBinaryShaderCache;
    class SceneConfigImpl : public StatusObjectImpl
    {
    public:
        status_t setPublicationMode(EScenePublicationMode publicationMode);
        EScenePublicationMode getPublicationMode() const;

    private:
        EScenePublicationMode m_publicationMode = EScenePublicationMode_LocalAndRemote;
    };
}

#endif
