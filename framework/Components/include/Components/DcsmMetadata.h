//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENTS_DCSMMETADATA_H
#define RAMSES_COMPONENTS_DCSMMETADATA_H

#include "ramses-framework-api/CarModelViewMetadata.h"
#include "ramses-framework-api/CarCameraPlaneMetadata.h"
#include "Collections/StringOutputStream.h"
#include "Components/DcsmTypes.h"
#include "absl/types/span.h"
#include <vector>
#include <string>
#include <cstdint>

namespace ramses_internal
{
    class DcsmMetadata
    {
    public:
        explicit DcsmMetadata(absl::Span<const Byte> data);
        DcsmMetadata() = default;

        DcsmMetadata(DcsmMetadata&&) noexcept = default;
        DcsmMetadata& operator=(DcsmMetadata&&) noexcept = default;

        // TODO(tobias): delete + explicit copy?
        DcsmMetadata(const DcsmMetadata&) = default;
        DcsmMetadata& operator=(const DcsmMetadata&) = default;

        bool empty() const;
        std::vector<Byte> toBinary() const;
        void updateFromOther(const DcsmMetadata& other);

        bool setPreviewImagePng(const void* data, size_t dataLength);
        bool setPreviewDescription(std::u32string previewDescription);
        bool setWidgetOrder(int32_t widgetOrder);
        bool setWidgetBackgroundID(int32_t widgetBackgroundID);
        bool setWidgetHUDLineID(int32_t widgetHUDlineID);
        bool setCarModel(int32_t carModel);
        bool setCarModelView(const ramses::CarModelViewMetadata& values, const AnimationInformation& timingInfo);
        bool setCarCameraPlanes(const ramses::CarCameraPlaneMetadata& values);
        bool setCarModelVisibility(bool visibility);
        bool setExclusiveBackground(bool state);
        bool setStreamID(int32_t streamID);

        bool hasPreviewImagePng() const;
        bool hasPreviewDescription() const;
        bool hasWidgetOrder() const;
        bool hasWidgetBackgroundID() const;
        bool hasWidgetHUDLineID() const;
        bool hasCarModel() const;
        bool hasCarModelView() const;
        bool hasCarCameraPlanes() const;
        bool hasCarModelVisibility() const;
        bool hasExclusiveBackground() const;
        bool hasStreamID() const;

        std::vector<unsigned char> getPreviewImagePng() const;
        std::u32string getPreviewDescription() const;
        int32_t getWidgetOrder() const;
        int32_t getWidgetBackgroundID() const;
        int32_t getWidgetHUDLineID() const;
        int32_t getCarModel() const;
        ramses::CarModelViewMetadata getCarModelView() const;
        AnimationInformation getCarModelViewAnimationInfo() const;
        ramses::CarCameraPlaneMetadata getCarCameraPlanes() const;
        bool getCarModelVisibility() const;
        bool getExclusiveBackground() const;
        int32_t getStreamID() const;

        bool operator==(const DcsmMetadata& other) const;
        bool operator!=(const DcsmMetadata& other) const;

        static constexpr const size_t MaxPreviewImageSize = 500000;
        static constexpr const size_t MaxPreviewImageWidth = 1000;
        static constexpr const size_t MaxPreviewImageHeight = 1000;

    private:
        friend struct fmt::formatter<ramses_internal::DcsmMetadata>;

        void fromBinary(absl::Span<const Byte> data);

        std::vector<unsigned char> m_previewImagePng;
        std::u32string m_previewDescription;
        int32_t m_widgetOrder = 0;
        int32_t m_widgetBackgroundID = 0;
        int32_t m_widgetHUDLineID = 0;
        int32_t m_carModel = 0;
        int32_t m_streamID = 0;
        ramses::CarModelViewMetadata m_carModelView = {};
        AnimationInformation m_carModelViewTiming = {};
        ramses::CarCameraPlaneMetadata m_carCameraPlanes = {};
        bool m_exclusiveBackground = false;
        bool m_carModelVisibility = false;

        bool m_hasCarModelView = false;
        bool m_hasCarCameraPlanes = false;
        bool m_hasPreviewImagePng = false;
        bool m_hasPreviewDescription = false;
        bool m_hasWidgetOrder = false;
        bool m_hasWidgetBackgroundID = false;
        bool m_hasWidgetHUDLineID = false;
        bool m_hasCarModel = false;
        bool m_hasCarModelVisibility = false;
        bool m_hasExclusiveBackground = false;
        bool m_hasStreamID = false;
    };

    static_assert(std::is_nothrow_move_constructible<DcsmMetadata>::value, "DcsmMetadata must be movable");
    static_assert(std::is_nothrow_move_assignable<DcsmMetadata>::value, "DcsmMetadata must be movable");
}

template <>
struct fmt::formatter<ramses_internal::DcsmMetadata> : public ramses_internal::SimpleFormatterBase
{
    template <typename FormatContext>
    auto format(const ramses_internal::DcsmMetadata& dm, FormatContext& ctx)
    {
        fmt::format_to(ctx.out(), "[png:");
        if (dm.hasPreviewImagePng())
            fmt::format_to(ctx.out(), "{}",  dm.m_previewImagePng.size());
        fmt::format_to(ctx.out(), "; desc:");
        if (dm.hasPreviewDescription())
            fmt::format_to(ctx.out(), "{}", dm.m_previewDescription.size());
        fmt::format_to(ctx.out(), "; order:");
        if (dm.hasWidgetOrder())
            fmt::format_to(ctx.out(), "{}", dm.m_widgetOrder);
        fmt::format_to(ctx.out(), "; bkgr:");
        if (dm.hasWidgetBackgroundID())
            fmt::format_to(ctx.out(), "{}", dm.m_widgetBackgroundID);
        fmt::format_to(ctx.out(), "; hudline:");
        if (dm.hasWidgetHUDLineID())
            fmt::format_to(ctx.out(), "{}", dm.m_widgetHUDLineID);
        fmt::format_to(ctx.out(), "; car:");
        if (dm.hasCarModel())
            fmt::format_to(ctx.out(), "{}", dm.m_carModel);
        fmt::format_to(ctx.out(), "; carView:");
        if (dm.hasCarModelView())
            fmt::format_to(ctx.out(), "{},{},{},{},{},{},{},{}",
                           dm.m_carModelView.pitch,
                           dm.m_carModelView.yaw,
                           dm.m_carModelView.distance,
                           dm.m_carModelView.origin_x,
                           dm.m_carModelView.origin_y,
                           dm.m_carModelView.origin_z,
                           dm.m_carModelViewTiming.startTimeStamp,
                           dm.m_carModelViewTiming.finishedTimeStamp);
        fmt::format_to(ctx.out(), "; carVis:");
        if (dm.hasCarModelVisibility())
            fmt::format_to(ctx.out(), "{}", dm.m_carModelVisibility);
        fmt::format_to(ctx.out(), "; exclBG:");
        if (dm.hasExclusiveBackground())
            fmt::format_to(ctx.out(), "{}", dm.m_exclusiveBackground);
        fmt::format_to(ctx.out(), "; focReq:");
        return fmt::format_to(ctx.out(), "]");
    }
};

#endif
