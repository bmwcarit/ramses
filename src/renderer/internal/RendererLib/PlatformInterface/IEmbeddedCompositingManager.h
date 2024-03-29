//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include <vector>

namespace ramses::internal
{
    using StreamSourceUpdates = std::vector< std::pair<WaylandIviSurfaceId, uint32_t> >;

    class IEmbeddedCompositingManager
    {
    public:
        virtual ~IEmbeddedCompositingManager() = default;

        virtual void refStream(WaylandIviSurfaceId source) = 0;
        virtual void unrefStream(WaylandIviSurfaceId source) = 0;

        virtual void dispatchStateChangesOfSources(WaylandIviSurfaceIdVector& streamsWithAvailabilityChanged, WaylandIviSurfaceIdVector& newStreams, WaylandIviSurfaceIdVector& obsoleteStreams) = 0;
        virtual void processClientRequests() = 0;
        [[nodiscard]] virtual bool hasUpdatedContentFromStreamSourcesToUpload() const = 0;
        virtual void uploadResourcesAndGetUpdates(StreamSourceUpdates& updatedStreams) = 0;
        virtual void notifyClients() = 0;
        [[nodiscard]] virtual DeviceResourceHandle getCompositedTextureDeviceHandleForStreamTexture(WaylandIviSurfaceId source) const = 0;

        [[nodiscard]] virtual bool hasRealCompositor() const = 0; //TODO Mohamed: remove this as soon as EC dummy is removed
    };
}
