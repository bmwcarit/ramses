//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneReferencing/SceneReferenceEvent.h"
#include "Components/ERendererToClientEventType.h"

namespace ramses_internal
{
    template<class T>
    inline void write(std::vector<Byte>::iterator& where, T const& value)
    {
        std::memcpy(&(*where), &value, sizeof(T));
        where += sizeof(T);
    }

    template<class T>
    inline void read(std::vector<Byte>::const_iterator& where, T& value)
    {
        std::memcpy(&value, &(*where), sizeof(T));
        where += sizeof(T);
    }

    void SceneReferenceEvent::readFromBlob(std::vector<Byte> const& blob)
    {
        assert(blob.size() == serializedSize);

        auto it = blob.cbegin();
        ERendererToClientEventType eventType;
        read(it, eventType);
        assert(eventType == ERendererToClientEventType::SceneReferencingEvent);

        read(it, type);
        read(it, referencedScene);
        read(it, consumerScene);
        read(it, providerScene);
        read(it, dataConsumer);
        read(it, dataProvider);
        read(it, sceneState);
        read(it, tag);
        read(it, status);

        assert(it == blob.cend());
    }

    void SceneReferenceEvent::writeToBlob(std::vector<Byte>& blob) const
    {
        assert(blob.empty());

        blob.resize(serializedSize);
        auto it = blob.begin();
        write(it, ERendererToClientEventType::SceneReferencingEvent);
        write(it, type);
        write(it, referencedScene);
        write(it, consumerScene);
        write(it, providerScene);
        write(it, dataConsumer);
        write(it, dataProvider);
        write(it, sceneState);
        write(it, tag);
        write(it, status);

        assert(it == blob.end());
    }
}
