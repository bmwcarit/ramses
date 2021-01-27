//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/DcsmMetadata.h"
#include "Utils/LogMacros.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "lodepng.h"
#include <cassert>

namespace ramses_internal
{
    constexpr const size_t DcsmMetadata::MaxPreviewImageSize;
    constexpr const size_t DcsmMetadata::MaxPreviewImageWidth;
    constexpr const size_t DcsmMetadata::MaxPreviewImageHeight;

    namespace
    {
        enum class DcsmMetadataType : uint32_t
        {
            PreviewImagePng = 1,
            PreviewDescription = 2,
            WidgetOrder = 3,
            WidgetBackgroundID = 4,
            WidgetWidgetHUDLineID = 5,
            WidgetCarModel = 6,
            WidgetCarModelView = 7,
            WidgetCarModelVisibility = 8,
            WidgetExclusiveBackground = 9,
            WidgetStreamID = 10
        };

        constexpr const uint32_t CurrentMetadataVersion = 1;
    }

    DcsmMetadata::DcsmMetadata(absl::Span<const Byte> data)
    {
        fromBinary(data);
    }

    bool DcsmMetadata::empty() const
    {
        return !m_hasPreviewImagePng &&
            !m_hasPreviewDescription &&
            !m_hasWidgetOrder &&
            !m_hasWidgetBackgroundID &&
            !m_hasWidgetHUDLineID &&
            !m_hasCarModel &&
            !m_hasCarModelView &&
            !m_hasCarModelVisibility &&
            !m_hasExclusiveBackground &&
            !m_hasStreamID;
    }

    std::vector<Byte> DcsmMetadata::toBinary() const
    {
        BinaryOutputStream os;
        const uint32_t numEntries =
            (m_hasPreviewImagePng ? 1 : 0) +
            (m_hasPreviewDescription ? 1 : 0) +
            (m_hasWidgetOrder ? 1 : 0) +
            (m_hasWidgetBackgroundID ? 1: 0) +
            (m_hasWidgetHUDLineID ? 1 : 0) +
            (m_hasCarModel ? 1 : 0) +
            (m_hasCarModelView ? 1 : 0) +
            (m_hasCarModelVisibility ? 1 : 0) +
            (m_hasExclusiveBackground ? 1 : 0) +
            (m_hasStreamID ? 1 : 0);
        os << CurrentMetadataVersion
           << numEntries;

        // write entries
        if (m_hasPreviewImagePng)
        {
            const uint32_t size = static_cast<uint32_t>(m_previewImagePng.size());
            os << DcsmMetadataType::PreviewImagePng
               << size;
            os.write(m_previewImagePng.data(), size);
        }
        if (m_hasPreviewDescription)
        {
            const uint32_t size = static_cast<uint32_t>(m_previewDescription.size() * sizeof(std::u32string::value_type));
            os << DcsmMetadataType::PreviewDescription
               << size;
            os.write(m_previewDescription.data(), size);
        }
        if (m_hasWidgetOrder)
        {
            const uint32_t size = static_cast<uint32_t>(sizeof(m_widgetOrder));
            os << DcsmMetadataType::WidgetOrder
               << size
               << m_widgetOrder;
        }
        if (m_hasWidgetBackgroundID)
        {
            const uint32_t size = static_cast<uint32_t>(sizeof(m_widgetBackgroundID));
            os << DcsmMetadataType::WidgetBackgroundID
               << size
               << m_widgetBackgroundID;
        }
        if (m_hasWidgetHUDLineID)
        {
            const uint32_t size = static_cast<uint32_t>(sizeof(m_widgetHUDLineID));
            os << DcsmMetadataType::WidgetWidgetHUDLineID
               << size
               << m_widgetHUDLineID;
        }
        if (m_hasCarModel)
        {
            const uint32_t size = static_cast<uint32_t>(sizeof(m_carModel));
            os << DcsmMetadataType::WidgetCarModel
               << size
               << m_carModel;
        }
        if (m_hasCarModelView)
        {
            constexpr size_t numberOfViewValues = 9; // pitch, yaw, distance, x,y,z of origin, fov, near, far
            constexpr size_t sizeData = sizeof(float) * numberOfViewValues;
            static_assert(sizeData==sizeof(ramses::CarModelViewMetadata), "size mismatch of CarModelViewMetadata");
            constexpr size_t sizeTiming = sizeof(uint64_t) * 2;
            static_assert(sizeTiming == sizeof(AnimationInformation), "size mismatch of AnimationInformation");
            os << DcsmMetadataType::WidgetCarModelView
               << uint32_t(sizeData + sizeTiming)
               << m_carModelView.pitch
               << m_carModelView.yaw
               << m_carModelView.distance
               << m_carModelView.origin_x
               << m_carModelView.origin_y
               << m_carModelView.origin_z
               << m_carModelView.cameraFOV
               << m_carModelView.nearPlane
               << m_carModelView.farPlane
               << m_carModelViewTiming.startTimeStamp
               << m_carModelViewTiming.finishedTimeStamp;
        }
        if (m_hasCarModelVisibility)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(int32_t));
            const int32_t visibility = (m_carModelVisibility ? 1 : 0);
            os << DcsmMetadataType::WidgetCarModelVisibility
               << size
               << visibility;
        }
        if (m_hasExclusiveBackground)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(int32_t));
            const int32_t state = (m_exclusiveBackground ? 1 : 0);
            os << DcsmMetadataType::WidgetExclusiveBackground
               << size
               << state;
        }
        if (m_hasStreamID)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(int32_t));
            os << DcsmMetadataType::WidgetStreamID
                << size
                << static_cast<int32_t>(m_streamID);
        }
        return os.release();
    }

    void DcsmMetadata::fromBinary(absl::Span<const Byte> data)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::fromBinary: length " << data.size());
        BinaryInputStream is(data.data());

        uint32_t readMetadataVersion = 0;
        is >> readMetadataVersion;
        if (readMetadataVersion != CurrentMetadataVersion)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmMetadata::fromBinary: ingore because unexpected metadata version " << readMetadataVersion << ", expected " << CurrentMetadataVersion);
            return;
        }

        uint32_t numEntries = 0;
        is >> numEntries;

        for (uint32_t i = 0; i < numEntries; ++i)
        {
            uint32_t size = 0;
            DcsmMetadataType type;
            is >> type
               >> size;
            switch(type)
            {
            case DcsmMetadataType::PreviewImagePng:
                m_hasPreviewImagePng = true;
                m_previewImagePng.resize(size);    // TODO(tobias) avoid zero init
                is.read(m_previewImagePng.data(), size);
                break;

            case DcsmMetadataType::PreviewDescription:
                assert(size % 4 == 0);
                m_hasPreviewDescription = true;
                m_previewDescription.clear();
                m_previewDescription.resize(size / 4);   // TODO(tobias) avoid zero init
                is.read(reinterpret_cast<char*>(&m_previewDescription[0]), size);
                break;

            case DcsmMetadataType::WidgetOrder:
                m_hasWidgetOrder = true;
                is >> m_widgetOrder;
                break;

            case DcsmMetadataType::WidgetBackgroundID:
                m_hasWidgetBackgroundID = true;
                is >> m_widgetBackgroundID;
                break;

            case DcsmMetadataType::WidgetWidgetHUDLineID:
                m_hasWidgetHUDLineID = true;
                is >> m_widgetHUDLineID;
                break;

            case DcsmMetadataType::WidgetCarModel:
                m_hasCarModel = true;
                is >> m_carModel;
                break;

            case DcsmMetadataType::WidgetCarModelView:
                m_hasCarModelView = true;
                is >> m_carModelView.pitch;
                is >> m_carModelView.yaw;
                is >> m_carModelView.distance;
                is >> m_carModelView.origin_x;
                is >> m_carModelView.origin_y;
                is >> m_carModelView.origin_z;
                is >> m_carModelView.cameraFOV;
                is >> m_carModelView.nearPlane;
                is >> m_carModelView.farPlane;
                is >> m_carModelViewTiming.startTimeStamp;
                is >> m_carModelViewTiming.finishedTimeStamp;
                break;

            case DcsmMetadataType::WidgetCarModelVisibility:
            {
                m_hasCarModelVisibility = true;
                int32_t visibility = 0;
                is >> visibility;
                m_carModelVisibility = (visibility == 0) ? false : true;
                break;
            }
            case DcsmMetadataType::WidgetExclusiveBackground:
            {
                m_hasExclusiveBackground = true;
                int32_t state = 0;
                is >> state;
                m_exclusiveBackground = (state == 0) ? false : true;
                break;
            }
            case DcsmMetadataType::WidgetStreamID:
            {
                m_hasStreamID = true;
                is >> m_streamID;
                break;
            }
            default:
                LOG_WARN(CONTEXT_DCSM, "DcsmMetadata::fromBinary: skip unknown type " << static_cast<uint32_t>(type) << ", size " << size);
                is.skip(size);
            }
        }

        assert(is.readPosition() == &*data.end());
    }

    void DcsmMetadata::updateFromOther(const DcsmMetadata& other)
    {
        // TODO(tobias): move in?
        if (other.m_hasPreviewImagePng)
        {
            m_hasPreviewImagePng = true;
            m_previewImagePng = other.m_previewImagePng;
        }
        if (other.m_hasPreviewDescription)
        {
            m_hasPreviewDescription = true;
            m_previewDescription = other.m_previewDescription;
        }
        if (other.m_hasWidgetOrder)
        {
            m_hasWidgetOrder = true;
            m_widgetOrder = other.m_widgetOrder;
        }
        if (other.m_hasWidgetBackgroundID)
        {
            m_hasWidgetBackgroundID = true;
            m_widgetBackgroundID = other.m_widgetBackgroundID;
        }
        if (other.m_hasWidgetHUDLineID)
        {
            m_hasWidgetHUDLineID = true;
            m_widgetHUDLineID = other.m_widgetHUDLineID;
        }
        if (other.m_hasCarModel)
        {
            m_hasCarModel = true;
            m_carModel = other.m_carModel;
        }
        if (other.m_hasCarModelView)
        {
            m_hasCarModelView = true;
            m_carModelView = other.m_carModelView;
            m_carModelViewTiming = other.m_carModelViewTiming;
        }
        if (other.m_hasCarModelVisibility)
        {
            m_hasCarModelVisibility = true;
            m_carModelVisibility = other.m_carModelVisibility;
        }
        if (other.m_hasExclusiveBackground)
        {
            m_hasExclusiveBackground = true;
            m_exclusiveBackground = other.m_exclusiveBackground;
        }
        if (other.m_hasStreamID)
        {
            m_hasStreamID = true;
            m_streamID = other.m_streamID;
        }
    }

    bool DcsmMetadata::setPreviewImagePng(const void* data, size_t dataLength)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setPreviewImagePng: Length " << dataLength);

        if (!data || !dataLength)
        {
            LOG_ERROR(CONTEXT_DCSM, "DcsmMetadata::setPreviewImagePng: Data empty");
            return false;
        }
        if (dataLength > MaxPreviewImageSize)
        {
            LOG_ERROR_P(CONTEXT_DCSM, "DcsmMetadata::setPreviewImagePng: Data length {} is larger than allowed maximum of {} bytes", dataLength, MaxPreviewImageSize);
            return false;
        }

        const unsigned char* dataC = static_cast<const unsigned char*>(data);
        lodepng::State state;
        unsigned int width = 0;
        unsigned int height = 0;
        const auto ret = lodepng_inspect(&width, &height, &state, dataC, dataLength);
        if (ret != 0)
        {
            LOG_ERROR(CONTEXT_DCSM, "DcsmMetadata::setPreviewImagePng: Invalid PNG, error " << ret << ": " << lodepng_error_text(ret));
            return false;
        }
        if (width > MaxPreviewImageWidth ||
            height > MaxPreviewImageHeight)
        {
            LOG_ERROR_P(CONTEXT_DCSM, "DcsmMetadata::setPreviewImagePng: Dimensions of {}x{} are larger than allowed maximum of {}x{}",
                        width, height, MaxPreviewImageWidth, MaxPreviewImageHeight);
            return false;
        }

        m_previewImagePng.clear();
        m_previewImagePng.insert(m_previewImagePng.end(), dataC, dataC + dataLength);
        m_hasPreviewImagePng = true;

        return true;
    }

    bool DcsmMetadata::setPreviewDescription(std::u32string previewDescription)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setPreviewDescription: Length " << previewDescription.size());

        m_previewDescription = std::move(previewDescription);
        m_hasPreviewDescription = true;
        return true;
    }

    bool DcsmMetadata::setWidgetOrder(int32_t widgetOrder)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setWidgetOrder: order: " << widgetOrder);

        m_widgetOrder = widgetOrder;
        m_hasWidgetOrder = true;
        return true;
    }

    bool DcsmMetadata::setWidgetBackgroundID(int32_t widgetBackgroundID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setWidgetBackgroundID: " << widgetBackgroundID);

        m_widgetBackgroundID = widgetBackgroundID;
        m_hasWidgetBackgroundID = true;
        return true;
    }

    bool DcsmMetadata::setWidgetHUDLineID(int32_t widgetHUDlineID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setWidgetHUDLineID: " << widgetHUDlineID);

        m_widgetHUDLineID = widgetHUDlineID;
        m_hasWidgetHUDLineID = true;
        return true;
    }

    bool DcsmMetadata::setCarModel(int32_t carModel)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setCarModel: " << carModel);

        m_carModel = carModel;
        m_hasCarModel = true;
        return true;
    }

    bool DcsmMetadata::setCarModelView(const ramses::CarModelViewMetadata& values, const AnimationInformation& timingInfo)
    {
        LOG_INFO_P(CONTEXT_DCSM, "DcsmMetadata::setCarModelview: {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}", values.pitch, values.yaw, values.distance, values.origin_x, values.origin_y, values.origin_z, values.cameraFOV, values.nearPlane, values.farPlane, timingInfo.startTimeStamp, timingInfo.finishedTimeStamp);

        m_carModelView = values;
        m_carModelViewTiming = timingInfo;
        m_hasCarModelView = true;
        return true;
    }

    bool DcsmMetadata::setCarModelVisibility(bool visibility)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setCarModelVisibility: " << visibility);

        m_carModelVisibility = visibility;
        m_hasCarModelVisibility = true;
        return true;
    }

    bool DcsmMetadata::setExclusiveBackground(bool state)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setExclusiveBackground: " << state);

        m_exclusiveBackground = state;
        m_hasExclusiveBackground = true;
        return true;
    }

    bool DcsmMetadata::setStreamID(int32_t streamID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setStreamID: " << streamID);

        m_streamID = streamID;
        m_hasStreamID = true;
        return true;
    }

    bool DcsmMetadata::hasPreviewImagePng() const
    {
        return m_hasPreviewImagePng;
    }

    bool DcsmMetadata::hasPreviewDescription() const
    {
        return m_hasPreviewDescription;
    }

    bool DcsmMetadata::hasWidgetOrder() const
    {
        return m_hasWidgetOrder;
    }

    bool DcsmMetadata::hasWidgetBackgroundID() const
    {
        return m_hasWidgetBackgroundID;
    }

    bool DcsmMetadata::hasWidgetHUDLineID() const
    {
        return m_hasWidgetHUDLineID;
    }

    bool DcsmMetadata::hasCarModel() const
    {
        return m_hasCarModel;
    }

    bool DcsmMetadata::hasCarModelView() const
    {
        return m_hasCarModelView;
    }

    bool DcsmMetadata::hasCarModelVisibility() const
    {
        return m_hasCarModelVisibility;
    }

    bool DcsmMetadata::hasExclusiveBackground() const
    {
        return m_hasExclusiveBackground;
    }

    bool DcsmMetadata::hasStreamID() const
    {
        return m_hasStreamID;
    }

    std::vector<unsigned char> DcsmMetadata::getPreviewImagePng() const
    {
        return m_previewImagePng;
    }

    std::u32string DcsmMetadata::getPreviewDescription() const
    {
        return m_previewDescription;
    }

    int32_t DcsmMetadata::getWidgetOrder() const
    {
        return m_widgetOrder;
    }

    int32_t DcsmMetadata::getWidgetBackgroundID() const
    {
        return m_widgetBackgroundID;
    }

    int32_t DcsmMetadata::getWidgetHUDLineID() const
    {
        return m_widgetHUDLineID;
    }

    int32_t DcsmMetadata::getCarModel() const
    {
        return m_carModel;
    }

    ramses::CarModelViewMetadata DcsmMetadata::getCarModelView() const
    {
        return m_carModelView;
    }

    AnimationInformation DcsmMetadata::getCarModelViewAnimationInfo() const
    {
        return m_carModelViewTiming;
    }

    bool DcsmMetadata::getCarModelVisibility() const
    {
        return m_carModelVisibility;
    }

    bool DcsmMetadata::getExclusiveBackground() const
    {
        return m_exclusiveBackground;
    }

    int32_t DcsmMetadata::getStreamID() const
    {
        return m_streamID;
    }

    bool DcsmMetadata::operator==(const DcsmMetadata& other) const
    {
        return m_hasPreviewImagePng == other.m_hasPreviewImagePng &&
            m_hasPreviewDescription == other.m_hasPreviewDescription &&
            m_previewImagePng == other.m_previewImagePng &&
            m_previewDescription == other.m_previewDescription &&
            m_hasWidgetOrder == other.m_hasWidgetOrder &&
            m_widgetOrder == other.m_widgetOrder &&
            m_hasWidgetBackgroundID == other.m_hasWidgetBackgroundID &&
            m_widgetBackgroundID == other.m_widgetBackgroundID &&
            m_hasWidgetHUDLineID == other.m_hasWidgetHUDLineID &&
            m_widgetHUDLineID == other.m_widgetHUDLineID &&
            m_hasCarModel == other.m_hasCarModel &&
            m_carModel == other.m_carModel &&
            m_hasCarModelView == other.m_hasCarModelView &&
            m_carModelView == other.m_carModelView &&
            m_carModelViewTiming == other.m_carModelViewTiming &&
            m_hasCarModelVisibility == other.m_hasCarModelVisibility &&
            m_carModelVisibility == other.m_carModelVisibility &&
            m_hasExclusiveBackground == other.m_hasExclusiveBackground &&
            m_exclusiveBackground == other.m_exclusiveBackground &&
            m_hasStreamID == other.m_hasStreamID &&
            m_streamID == other.m_streamID;
    }

    bool DcsmMetadata::operator!=(const DcsmMetadata& other) const
    {
        return !(*this == other);
    }
}
