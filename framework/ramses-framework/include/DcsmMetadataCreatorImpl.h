//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMMETADATACREATORIMPL_H
#define RAMSES_DCSMMETADATACREATORIMPL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Components/DcsmMetadata.h"
#include "StatusObjectImpl.h"

namespace ramses
{
    class DcsmMetadataCreatorImpl : public StatusObjectImpl
    {
    public:
        DcsmMetadataCreatorImpl();
        ~DcsmMetadataCreatorImpl();

        status_t setPreviewImagePng(const void* data, size_t dataLength);
        status_t setPreviewDescription(std::u32string previewDescription);
        status_t setWidgetOrder(int32_t widgetOrder);
        status_t setWidgetBackgroundID(int32_t backgroundID);
        status_t setWidgetHUDLineID(int32_t widgetHudLineID);
        status_t setCarModel(int32_t carModel);
        status_t setCarModelView(const CarModelViewMetadata& data);
        status_t setCarModelVisibility(bool visibility);

        ramses_internal::DcsmMetadata getMetadata() const;
    private:
        ramses_internal::DcsmMetadata m_metadata;
    };
}

#endif
