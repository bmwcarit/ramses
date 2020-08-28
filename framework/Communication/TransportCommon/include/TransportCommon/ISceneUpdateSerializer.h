//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISCENEUPDATESERIALIZER_H
#define RAMSES_ISCENEUPDATESERIALIZER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "absl/types/span.h"

namespace ramses_internal
{
    class ISceneUpdateSerializer
    {
    public:
        virtual ~ISceneUpdateSerializer() = default;
        virtual bool writeToPackets(absl::Span<Byte> packetMem, const std::function<bool(size_t)>& writeDoneFunc) const = 0;
    };
}

#endif
