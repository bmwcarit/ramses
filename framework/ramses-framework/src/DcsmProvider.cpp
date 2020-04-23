//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-framework-api/DcsmProvider.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "DcsmProviderImpl.h"
#include "RamsesFrameworkImpl.h"
#include "RamsesFrameworkTypesImpl.h"

#include "APILoggingMacros.h"

namespace ramses
{
    DcsmProvider::~DcsmProvider()
    {
        LOG_HL_CLIENT_API_NOARG(LOG_API_VOID);
    }

    DcsmProvider::DcsmProvider(DcsmProviderImpl& impl_)
        : StatusObject(impl_)
        , impl(impl_)
    {
    }

    status_t DcsmProvider::offerContent(ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode)
    {
        auto status = impl.offerContent(contentID, category, scene, mode);
        LOG_HL_CLIENT_API4(status, contentID, category, scene, static_cast<int>(mode));
        return status;
    }

    status_t DcsmProvider::offerContentWithMetadata(ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode, const DcsmMetadataCreator& metadata)
    {
        const auto status = impl.offerContentWithMetadata(contentID, category, scene, mode, metadata);
        LOG_HL_CLIENT_API5(status, contentID, category, scene, static_cast<int>(mode), LOG_API_GENERIC_OBJECT_STRING(metadata));
        return status;
    }

    status_t DcsmProvider::updateContentMetadata(ContentID contentID, const DcsmMetadataCreator& metadata)
    {
        const auto status = impl.updateContentMetadata(contentID, metadata);
        LOG_HL_CLIENT_API1(status, contentID);
        return status;
    }

    status_t DcsmProvider::requestStopOfferContent(ContentID contentID)
    {
        auto status = impl.requestStopOfferContent(contentID);
        LOG_HL_CLIENT_API1(status, contentID);
        return status;
    }

    status_t DcsmProvider::markContentReady(ContentID contentID)
    {
        auto status = impl.markContentReady(contentID);
        LOG_HL_CLIENT_API1(status, contentID);
        return status;
    }

    status_t DcsmProvider::requestContentFocus(ContentID contentID)
    {
        auto status = impl.requestContentFocus(contentID);
        LOG_HL_CLIENT_API1(status, contentID);
        return status;
    }

    status_t DcsmProvider::dispatchEvents(IDcsmProviderEventHandler& handler)
    {
        auto status = impl.dispatchEvents(handler);
        LOG_HL_CLIENT_API1(status, LOG_API_GENERIC_OBJECT_STRING(handler));
        return status;
    }

}
