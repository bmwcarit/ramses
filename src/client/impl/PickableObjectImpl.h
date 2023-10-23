//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "SceneObjectImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "impl/NodeImpl.h"

#include <string_view>

namespace ramses
{
    class ArrayBuffer;
    class Camera;
}

namespace ramses::internal
{
    class ArrayBufferImpl;
    class CameraNodeImpl;

    class PickableObjectImpl final : public NodeImpl
    {
    public:
        PickableObjectImpl(SceneImpl& scene, std::string_view pickableObjectName);
        ~PickableObjectImpl() override = default;

        void initializeFrameworkData(const ArrayBufferImpl& geometryBuffer, pickableObjectId_t id);
        void deinitializeFrameworkData() override;
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        void onValidate(ValidationReportImpl& report) const override;

        [[nodiscard]] const ArrayBuffer& getGeometryBuffer() const;

        bool setCamera(const CameraNodeImpl& cameraImpl);
        [[nodiscard]] const ramses::Camera* getCamera() const;

        bool setPickableObjectId(pickableObjectId_t id);
        [[nodiscard]] pickableObjectId_t getPickableObjectId() const;

        bool setEnabled(bool enable);
        [[nodiscard]] bool     isEnabled() const;

        [[nodiscard]] ramses::internal::PickableObjectHandle getPickableObjectHandle() const;

    private:
        ramses::internal::PickableObjectHandle m_pickableObjectHandle;
        const ArrayBufferImpl*                m_geometryBufferImpl = nullptr;
        const CameraNodeImpl*                 m_cameraImpl = nullptr;
    };
}
