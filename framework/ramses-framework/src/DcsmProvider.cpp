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

    status_t DcsmProvider::offerContent(ContentID contentID, Category category, sceneId_t scene)
    {
        auto status = impl.offerContent(contentID, category, scene);
        LOG_HL_CLIENT_API3(status, contentID.getValue(), category.getValue(), scene);
        return status;
    }

    status_t DcsmProvider::requestStopOfferContent(ContentID contentID)
    {
        auto status = impl.requestStopOfferContent(contentID);
        LOG_HL_CLIENT_API1(status, contentID.getValue());
        return status;
    }

    status_t DcsmProvider::markContentReady(ContentID contentID)
    {
        auto status = impl.markContentReady(contentID);
        LOG_HL_CLIENT_API1(status, contentID.getValue());
        return status;
    }

    status_t DcsmProvider::requestContentFocus(ContentID contentID)
    {
        auto status = impl.requestContentFocus(contentID);
        LOG_HL_CLIENT_API1(status, contentID.getValue());
        return status;
    }

    status_t DcsmProvider::dispatchEvents(IDcsmProviderEventHandler& handler)
    {
        auto status = impl.dispatchEvents(handler);
        LOG_HL_CLIENT_API1(status, LOG_API_GENERIC_OBJECT_STRING(handler));
        return status;
    }

}
