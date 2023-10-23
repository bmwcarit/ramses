//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/IOutputStream.h"
#include "internal/PlatformAbstraction/Collections/IInputStream.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include <type_traits>

namespace ramses::internal
{
    enum class EMessageId : uint32_t
    {
        Invalid,
        PublishScene,
        UnpublishScene,
        SubscribeScene,
        UnsubscribeScene,

        ConnectionDescriptionMessage,
        ConnectorAddressExchange,
        InputEvent,
        SendSceneUpdate,

        RendererEvent,

        Alive,

        // scene
        CreateScene,
    };

    const std::array EMessageIdNames = {
        "Invalid",
        "PublishScene",
        "UnpublishScene",
        "SubscribeScene",
        "UnsubscribeScene",
        "ConnectionDescriptionMessage",
        "ConnectorAddressExchange",
        "InputEvent",
        "SendSceneUpdate",
        "RendererEvent",
        "Alive",
        "CreateScene",
    };

    inline IOutputStream& operator<<(IOutputStream& outputStream, EMessageId messageId)
    {
        return outputStream << static_cast<std::underlying_type_t<EMessageId>>(messageId);
    }

    inline IInputStream& operator>>(IInputStream& inputStream, EMessageId& messageId)
    {
        std::underlying_type_t<EMessageId> val = 0;
        inputStream >> val;
        messageId = static_cast<EMessageId>(val);
        return inputStream;
    }
}

MAKE_ENUM_CLASS_PRINTABLE(ramses::internal::EMessageId,
                          "EMessageId",
                          ramses::internal::EMessageIdNames,
                          ramses::internal::EMessageId::CreateScene);
