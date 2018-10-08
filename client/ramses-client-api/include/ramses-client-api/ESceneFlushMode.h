//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ESCENEFLUSHMODE_H
#define RAMSES_ESCENEFLUSHMODE_H

namespace ramses
{
    /**
     * Specifies the mode of scene flushing.
     */
    enum ESceneFlushMode
    {
        ESceneFlushMode_SynchronizedWithResources = 1       /// Scene actions from flush will be applied
                                                            /// only after all resources are available
                                                            /// and ready to render on renderer side
    };

}

#endif
