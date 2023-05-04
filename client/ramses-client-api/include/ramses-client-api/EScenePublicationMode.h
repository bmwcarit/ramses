//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ESCENEPUBLICATIONMODE_H
#define RAMSES_ESCENEPUBLICATIONMODE_H

namespace ramses
{
    /**
     * @ingroup CoreAPI
     * Specifies the mode of scene publication.
     *
     * When using localOnly publication it is possible to render the scene locally without calling
     * #ramses::RamsesFramework::connect.
    */
    enum EScenePublicationMode
    {
        EScenePublicationMode_LocalAndRemote = 0,
        EScenePublicationMode_LocalOnly
    };

}

#endif
