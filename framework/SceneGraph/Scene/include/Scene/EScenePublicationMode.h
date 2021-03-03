//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_ESCENEPUBLICATIONMODE_H
#define RAMSES_SCENE_ESCENEPUBLICATIONMODE_H

#include "PlatformAbstraction/PlatformTypes.h"
#include <cassert>

namespace ramses_internal
{
    /**
     * Specifies the mode of scene publication.
    */
    enum EScenePublicationMode
    {
        EScenePublicationMode_Unpublished = 0,
        EScenePublicationMode_LocalAndRemote,
        EScenePublicationMode_LocalOnly
    };

    inline const Char* EnumToString(EScenePublicationMode publicationMode)
    {
        switch (publicationMode)
        {
            case EScenePublicationMode_Unpublished:
                return "EScenePublicationMode_Unpublished";
            case EScenePublicationMode_LocalAndRemote:
                return "EScenePublicationMode_LocalAndRemote";
            case EScenePublicationMode_LocalOnly:
                return "EScenePublicationMode_LocalOnly";
            default:
                assert(false);
                break;
        }

        return "EScenePublicationMode UNKNOWN";
    }
}

#endif
