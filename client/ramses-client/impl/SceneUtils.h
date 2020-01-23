//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEUTILS_H
#define RAMSES_SCENEUTILS_H

#include <assert.h>

#include "Scene/EScenePublicationMode.h"
#include "ramses-client-api/EScenePublicationMode.h"

namespace ramses
{
    class SceneUtils
    {
    public:
        static ramses_internal::EScenePublicationMode GetScenePublicationModeInternal(ramses::EScenePublicationMode publicationMode)
        {
            switch (publicationMode)
            {
            case ramses::EScenePublicationMode_LocalAndRemote:
                return ramses_internal::EScenePublicationMode_LocalAndRemote;
            case ramses::EScenePublicationMode_LocalOnly:
                return ramses_internal::EScenePublicationMode_LocalOnly;
            default:
                assert(false);
                return ramses_internal::EScenePublicationMode_LocalAndRemote;
            }
        }
    };
}

#endif
