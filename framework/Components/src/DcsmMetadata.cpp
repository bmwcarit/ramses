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

namespace ramses_internal
{
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
        };

        constexpr static const uint32_t CurrentMetadataVersion = 1;
    }

    DcsmMetadata::DcsmMetadata(const unsigned char* data, uint64_t len)
    {
        fromBinary(data, len);
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
            !m_hasCarModelVisibility;
    }

    std::vector<char> DcsmMetadata::toBinary() const
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
            (m_hasCarModelVisibility ? 1 : 0);
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
            constexpr uint32_t numberOfViewValues = 6; // pitch, yaw, distance, x,y,z of origin
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(float) * numberOfViewValues);
            static_assert(size==sizeof(ramses::CarModelViewMetadata), "size mismatch of CarModelViewMetadata");
            os << DcsmMetadataType::WidgetCarModelView
               << size
               << m_carModelView.pitch
               << m_carModelView.yaw
               << m_carModelView.distance
               << m_carModelView.origin_x
               << m_carModelView.origin_y
               << m_carModelView.origin_z;
        }
        if (m_hasCarModelVisibility)
        {
            constexpr uint32_t size = static_cast<uint32_t>(sizeof(int32_t));
            const int32_t visibility = (m_carModelVisibility ? 1 : 0);
            os << DcsmMetadataType::WidgetCarModelVisibility
               << size
               << visibility;
        }
        return os.release();
    }

    void DcsmMetadata::fromBinary(const unsigned char* data, uint64_t len)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::fromBinary: length " << len);
        BinaryInputStream is(data);

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
                break;

            case DcsmMetadataType::WidgetCarModelVisibility:
            {
                m_hasCarModelVisibility = true;
                int32_t visibility = 0;
                is >> visibility;
                m_carModelVisibility = (visibility == 0) ? false : true;
                break;
            }

            default:
                LOG_WARN(CONTEXT_DCSM, "DcsmMetadata::fromBinary: skip unknown type " << static_cast<uint32_t>(type) << ", size " << size);
                is.skip(size);
            }
        }

        assert(reinterpret_cast<const unsigned char*>(is.readPosition()) == data + len);
        UNUSED(len);
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
        }
        if (other.m_hasCarModelVisibility)
        {
            m_hasCarModelVisibility = true;
            m_carModelVisibility = other.m_carModelVisibility;
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
        m_hasWidgetBackgroundID = widgetBackgroundID;
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

    bool DcsmMetadata::setCarModelView(const ramses::CarModelViewMetadata& values)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmMetadata::setCarModelview: " << values.pitch << "," << values.yaw << "," << values.distance << "," << values.origin_x << "," << values.origin_y << "," << values.origin_z);

        m_carModelView = values;
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

    bool DcsmMetadata::getCarModelVisibility() const
    {
        return m_hasCarModelVisibility;
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
            m_hasCarModelVisibility == other.m_hasCarModelVisibility &&
            m_carModelVisibility == other.m_carModelVisibility;
    }

    bool DcsmMetadata::operator!=(const DcsmMetadata& other) const
    {
        return !(*this == other);
    }
}
