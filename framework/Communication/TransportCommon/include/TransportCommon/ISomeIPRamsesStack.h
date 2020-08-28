//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISOMEIPRAMSESSTACK_H
#define RAMSES_ISOMEIPRAMSESSTACK_H

#include "TransportCommon/SomeIPStackCommon.h"
#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/SceneId.h"
#include "absl/types/span.h"
#include <cstdint>

namespace ramses_internal
{
    using RamsesInstanceId = StronglyTypedValue<uint16_t, 0xFFFF, struct RamsesInstanceIdTag>;

    struct SceneAvailabilityUpdate
    {
        SceneId sceneId;
        String friendlyName;
        bool available;

        bool operator==(const SceneAvailabilityUpdate& other) const
        {
            return sceneId == other.sceneId && friendlyName == other.friendlyName && available == other.available;
        }

        bool operator!=(const SceneAvailabilityUpdate& other) const
        {
            return !(*this == other);
        }
    };

    struct SceneSubscriptionUpdate
    {
        SceneId sceneId;
        bool subscribed;

        bool operator==(const SceneSubscriptionUpdate& other) const
        {
            return sceneId == other.sceneId && subscribed == other.subscribed;
        }

        bool operator!=(const SceneSubscriptionUpdate& other) const
        {
            return !(*this == other);
        }
    };

    class ISomeIPRamsesStackCallbacks : public ISomeIPStackCallbacksCommon<RamsesInstanceId>
    {
    public:
        virtual void handleRendererEvent(const SomeIPMsgHeader& header, SceneId sceneId, const std::vector<Byte>& data) = 0;
        virtual void handleRequestResources(const SomeIPMsgHeader& header, const ResourceContentHashVector& resources) = 0;
        virtual void handleResourcesNotAvailable(const SomeIPMsgHeader& header, const ResourceContentHashVector& resources) = 0;
        virtual void handleResourceTransfer(const SomeIPMsgHeader& header, absl::Span<const Byte> resourceData) = 0;
        virtual void handleSceneUpdate(const SomeIPMsgHeader& header, SceneId sceneId, absl::Span<const Byte> sceneUpdate) = 0;
        virtual void handleSceneAvailabilityChange(const SomeIPMsgHeader& header, const std::vector<SceneAvailabilityUpdate>& update) = 0;
        virtual void handleSceneSubscriptionChange(const SomeIPMsgHeader& header, const std::vector<SceneSubscriptionUpdate>& update) = 0;
        virtual void handleInitializeScene(const SomeIPMsgHeader& header, SceneId sceneId) = 0;
    };

    struct RamsesStackSendDataSizes
    {
        uint32_t sceneActionData;
        uint32_t resourceData;
        uint32_t renderEventData;
        uint32_t sceneAvailabilityList;
        uint32_t sceneSubscriptionList;
        uint32_t resourceHashList;
    };

    class ISomeIPRamsesStack : public ISomeIPStackCommon<RamsesInstanceId>
    {
    public:
        virtual void setCallbacks(ISomeIPRamsesStackCallbacks* handlers) = 0;
        virtual RamsesStackSendDataSizes getSendDataSizes() const = 0;

        virtual bool sendRendererEvent(RamsesInstanceId to, const SomeIPMsgHeader& header, SceneId sceneId, const std::vector<Byte>& data) = 0;
        virtual bool sendRequestResources(RamsesInstanceId to, const SomeIPMsgHeader& header, absl::Span<const ResourceContentHash> resources) = 0;
        virtual bool sendResourcesNotAvailable(RamsesInstanceId to, const SomeIPMsgHeader& header, absl::Span<const ResourceContentHash> resources) = 0;
        virtual bool sendResourceTransfer(RamsesInstanceId to, const SomeIPMsgHeader& header, const std::vector<Byte>& resourceData) = 0;
        virtual bool sendSceneUpdate(RamsesInstanceId to, const SomeIPMsgHeader& header, SceneId sceneId, const std::vector<Byte>& sceneUpdate) = 0;
        virtual bool sendSceneAvailabilityChange(RamsesInstanceId to, const SomeIPMsgHeader& header, const std::vector<SceneAvailabilityUpdate>& update) = 0;
        virtual bool sendSceneSubscriptionChange(RamsesInstanceId to, const SomeIPMsgHeader& header, const std::vector<SceneSubscriptionUpdate>& update) = 0;
        virtual bool sendInitializeScene(RamsesInstanceId to, const SomeIPMsgHeader& header, SceneId sceneId) = 0;  // TODO(tobias) try to remove
    };
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::RamsesInstanceId)

#endif
