//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cassert>
#include "ramses/framework/EScenePublicationMode.h"

namespace ramses::internal
{
    using ramses::EScenePublicationMode;

    inline const char* EnumToString(EScenePublicationMode publicationMode)
    {
        switch (publicationMode)
        {
        case EScenePublicationMode::LocalAndRemote:
            return "EScenePublicationMode::LocalAndRemote";
        case EScenePublicationMode::LocalOnly:
            return "EScenePublicationMode::LocalOnly";
        }
        assert(false);
        return "";
    }
}
