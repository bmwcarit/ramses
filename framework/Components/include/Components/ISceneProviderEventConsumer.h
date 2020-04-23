//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISCENEPROVIDEREVENTCONSUMER_H
#define RAMSES_ISCENEPROVIDEREVENTCONSUMER_H

namespace ramses_internal
{
    struct SceneReferenceEvent;
    class Guid;

    class ISceneProviderEventConsumer
    {
    public:
        virtual ~ISceneProviderEventConsumer() = default;

        virtual void handleSceneReferenceEvent(SceneReferenceEvent const& event, const Guid& rendererId) = 0;
    };
}

#endif
