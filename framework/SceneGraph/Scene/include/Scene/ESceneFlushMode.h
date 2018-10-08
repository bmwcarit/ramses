//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENE_ESCENEFLUSHMODE_H
#define RAMSES_SCENE_ESCENEFLUSHMODE_H

namespace ramses_internal
{
    /**
     * Specifies the mode of scene flushing
     */
    enum ESceneFlushMode
    {
        ESceneFlushMode_Asynchronous = 0,
        ESceneFlushMode_Synchronous
    };

    inline const Char* EnumToString(ESceneFlushMode flushMode)
    {
        switch (flushMode)
        {
            case ESceneFlushMode_Asynchronous:
                return "ESceneFlushMode_Asynchronous";
            case ESceneFlushMode_Synchronous:
                return "ESceneFlushMode_Synchronous";
            default:
                assert(false);
                break;
        }

        return "ESceneFlushMode UNKNOWN";
    }
}

#endif
