//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMESSAGEID_H
#define RAMSES_EMESSAGEID_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"

namespace ramses_internal
{
    typedef UInt32 MessageId;

    enum EMessageId
    {
        EMessageId_PublishScene = 1,
        EMessageId_UnpublishScene,
        EMessageId_SubscribeScene,
        EMessageId_UnsubscribeScene,
        EMessageId_SceneNotAvailable,

        EMessageId_ConnectionDescriptionMessage,
        EMessageId_ConnectorAddressExchange,
        EMessageId_InputEvent,
        EMessageId_SendSceneActionList,

        EMessageId_Alive,

        // resources
        EMessageId_TransferResources,
        EMessageId_RequestResources,
        EMessageId_ResourcesNotAvailable,

        // scene
        EMessageId_CreateScene,

        // dcsm
        EMessageId_DcsmRegisterContent,
        EMessageId_DcsmCanvasSizeChange,
        EMessageId_DcsmContentStatusChange,
        EMessageId_DcsmContentAvailable,
        EMessageId_DcsmCategoryContentSwitchRequest,
        EMessageId_DcsmRequestUnregisterContent,
        EMessageId_DcsmForceUnregisterContent,
    };

#ifndef CreateNameForEnumID
#define CreateNameForEnumID(ENUMVALUE) \
case ENUMVALUE: return #ENUMVALUE
#endif

    inline
    const Char* GetNameForMessageId(MessageId type)
    {
            switch (type)
            {
                CreateNameForEnumID(EMessageId_PublishScene);
                CreateNameForEnumID(EMessageId_UnpublishScene);
                CreateNameForEnumID(EMessageId_SubscribeScene);
                CreateNameForEnumID(EMessageId_UnsubscribeScene);
                CreateNameForEnumID(EMessageId_SceneNotAvailable);

                CreateNameForEnumID(EMessageId_ConnectionDescriptionMessage);
                CreateNameForEnumID(EMessageId_ConnectorAddressExchange);
                CreateNameForEnumID(EMessageId_InputEvent);
                CreateNameForEnumID(EMessageId_SendSceneActionList);

                CreateNameForEnumID(EMessageId_Alive);

                // resources
                CreateNameForEnumID(EMessageId_TransferResources);
                CreateNameForEnumID(EMessageId_RequestResources);
                CreateNameForEnumID(EMessageId_ResourcesNotAvailable);

                // scene
                CreateNameForEnumID(EMessageId_CreateScene);
            }
            return "Unknown Message Type";
    }

    inline
    IOutputStream& operator<<(IOutputStream& outputStream, EMessageId messageId)
    {
        outputStream << static_cast<UInt32>(messageId);
        return outputStream;
    }

    inline
    IInputStream& operator>>(IInputStream& inputStream, EMessageId& messageId)
    {
        inputStream >> reinterpret_cast<UInt32&>(messageId);
        return inputStream;
    }
}

#endif
