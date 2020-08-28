//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SceneUpdateSerializer.h"
#include "TransportCommon/SingleSceneUpdateWriter.h"

namespace ramses_internal
{
    SceneUpdateSerializer::SceneUpdateSerializer(const SceneUpdate& update)
        : m_update(update)
    {
    }

    bool SceneUpdateSerializer::writeToPackets(absl::Span<Byte> packetMem, const std::function<bool(size_t)>& writeDoneFunc) const
    {
        SingleSceneUpdateWriter writer(m_update, packetMem, writeDoneFunc);
        return writer.write();
    }

    const SceneUpdate& SceneUpdateSerializer::getUpdate() const
    {
        return m_update;
    }

}
