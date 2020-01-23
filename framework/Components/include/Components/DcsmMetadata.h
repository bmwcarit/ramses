//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENTS_DCSMMETADATA_H
#define RAMSES_COMPONENTS_DCSMMETADATA_H

#include "Collections/StringOutputStream.h"
#include <vector>
#include <string>
#include <cstdint>
#include "ramses-framework-api/CarModelViewMetadata.h"

namespace ramses_internal
{
    class DcsmMetadata
    {
    public:
        DcsmMetadata(const unsigned char* data, uint64_t len);
        DcsmMetadata() = default;

        DcsmMetadata(DcsmMetadata&&) RNOEXCEPT = default;
        DcsmMetadata& operator=(DcsmMetadata&&) RNOEXCEPT = default;

        // TODO(tobias): delete + explicit copy?
        DcsmMetadata(const DcsmMetadata&) = default;
        DcsmMetadata& operator=(const DcsmMetadata&) = default;

        bool empty() const;
        std::vector<char> toBinary() const;
        void updateFromOther(const DcsmMetadata& other);

        bool setPreviewImagePng(const void* data, size_t dataLength);
        bool setPreviewDescription(std::u32string previewDescription);
        bool setWidgetOrder(int32_t widgetOrder);
        bool setWidgetBackgroundID(int32_t widgetBackgroundID);
        bool setWidgetHUDLineID(int32_t widgetHUDlineID);
        bool setCarModel(int32_t carModel);
        bool setCarModelView(const ramses::CarModelViewMetadata&);
        bool setCarModelVisibility(bool visibility);

        bool hasPreviewImagePng() const;
        bool hasPreviewDescription() const;
        bool hasWidgetOrder() const;
        bool hasWidgetBackgroundID() const;
        bool hasWidgetHUDLineID() const;
        bool hasCarModel() const;
        bool hasCarModelView() const;
        bool hasCarModelVisibility() const;

        std::vector<unsigned char> getPreviewImagePng() const;
        std::u32string getPreviewDescription() const;
        int32_t getWidgetOrder() const;
        int32_t getWidgetBackgroundID() const;
        int32_t getWidgetHUDLineID() const;
        int32_t getCarModel() const;
        ramses::CarModelViewMetadata getCarModelView() const;
        bool getCarModelVisibility() const;

        bool operator==(const DcsmMetadata& other) const;
        bool operator!=(const DcsmMetadata& other) const;

        friend StringOutputStream& operator<<(StringOutputStream& os, const DcsmMetadata& dm);

    private:
        void fromBinary(const unsigned char* data, uint64_t len);

        bool m_hasPreviewImagePng = false;
        std::vector<unsigned char> m_previewImagePng;

        bool m_hasPreviewDescription = false;
        std::u32string m_previewDescription;
        int32_t m_widgetOrder = 0;
        bool m_hasWidgetOrder = false;
        int32_t m_widgetBackgroundID = 0;
        bool m_hasWidgetBackgroundID = false;
        int32_t m_widgetHUDLineID = 0;
        bool m_hasWidgetHUDLineID = false;
        bool m_hasCarModel = false;
        int32_t m_carModel = 0;
        bool m_hasCarModelView = false;
        ramses::CarModelViewMetadata m_carModelView = {};
        bool m_hasCarModelVisibility = false;
        bool m_carModelVisibility = false;
    };

    static_assert(std::is_nothrow_move_constructible<DcsmMetadata>::value, "DcsmMetadata must be movable");
    static_assert(std::is_nothrow_move_assignable<DcsmMetadata>::value, "DcsmMetadata must be movable");

    // Make printable in logs
    inline StringOutputStream& operator<<(StringOutputStream& os, const DcsmMetadata& dm)
    {
        os << "[png:";
        if (dm.m_hasPreviewImagePng)
            os << dm.m_previewImagePng.size();
        os << "; desc:";
        if (dm.m_hasPreviewDescription)
            os << dm.m_previewDescription.size();
        os << "; order:";
        if (dm.m_hasWidgetOrder)
            os << dm.m_widgetOrder;
        os << "; bkgr:";
        if (dm.m_hasWidgetBackgroundID)
            os << dm.m_widgetBackgroundID;
        os << "; hudline:";
        if (dm.m_hasWidgetHUDLineID)
            os << dm.m_widgetHUDLineID;
        os << "; car:";
        if (dm.m_hasCarModel)
            os << dm.m_carModel;
        os << "; carView:";
        if (dm.m_hasCarModelView)
        {
            os << dm.m_carModelView.pitch    << ",";
            os << dm.m_carModelView.yaw      << ",";
            os << dm.m_carModelView.distance << ",";
            os << dm.m_carModelView.origin_x << ",";
            os << dm.m_carModelView.origin_y << ",";
            os << dm.m_carModelView.origin_z << ",";
        }
        os << "; carVis:";
        if (dm.m_hasCarModelVisibility)
            os << dm.m_carModelVisibility;
        os << "]";
        return os;
    }
}

#endif
