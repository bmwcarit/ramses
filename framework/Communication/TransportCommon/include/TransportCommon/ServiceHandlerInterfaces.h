//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SERVICEHANDLERINTERFACES_H
#define RAMSES_SERVICEHANDLERINTERFACES_H

#include "SceneAPI/SceneId.h"
#include "SceneAPI/SceneTypes.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Components/DcsmTypes.h"
#include "Components/DcsmMetadata.h"
#include "absl/types/span.h"

namespace ramses_internal
{
    class Guid;
    class SceneActionCollection;
    struct SceneReferenceEvent;
    class CategoryInfo;

    class ISceneProviderServiceHandler
    {
    public:
        virtual ~ISceneProviderServiceHandler() {}

        virtual void handleSubscribeScene(const SceneId& sceneId, const Guid& consumerID) = 0;
        virtual void handleUnsubscribeScene(const SceneId& sceneId, const Guid& consumerID) = 0;
        virtual void handleRendererEvent(const SceneId& sceneId, const std::vector<Byte>& data, const Guid& rendererId) = 0;
    };

    class ISceneRendererServiceHandler
    {
    public:
        virtual ~ISceneRendererServiceHandler() {}

        virtual void handleNewScenesAvailable(const SceneInfoVector& newScenes, const Guid& providerID) = 0;
        virtual void handleScenesBecameUnavailable(const SceneInfoVector& unavailableScenes, const Guid& providerID) = 0;

        virtual void handleSceneNotAvailable(const SceneId& sceneId, const Guid& providerID) = 0;

        virtual void handleInitializeScene(const SceneId& sceneId, const Guid& providerID) = 0;
        virtual void handleSceneUpdate(const SceneId& sceneId, absl::Span<const Byte> actionData, const Guid& providerID) = 0;
    };

    class IDcsmProviderServiceHandler
    {
    public:
        virtual ~IDcsmProviderServiceHandler() {};
        virtual void handleCanvasSizeChange(ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation, const Guid& consumerID) = 0;
        virtual void handleContentStateChange(ContentID contentID, EDcsmState status, const CategoryInfo& categoryInfo, AnimationInformation, const Guid& consumerID) = 0;
    };

    class IDcsmConsumerServiceHandler
    {
    public:
        virtual ~IDcsmConsumerServiceHandler() {};
        virtual void handleOfferContent(ContentID contentID, Category, ETechnicalContentType technicalContentType, const std::string& friendlyName, const Guid& providerID) = 0;
        virtual void handleContentDescription(ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID) = 0;
        virtual void handleContentReady(ContentID contentID, const Guid& providerID) = 0;
        virtual void handleContentEnableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID) = 0;
        virtual void handleContentDisableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID) = 0;
        virtual void handleRequestStopOfferContent(ContentID contentID, const Guid& providerID) = 0;
        virtual void handleForceStopOfferContent(ContentID contentID, const Guid& providerID) = 0;
        virtual void handleUpdateContentMetadata(ContentID contentID, DcsmMetadata metadata, const Guid& providerID) = 0;
    };
}

#endif
