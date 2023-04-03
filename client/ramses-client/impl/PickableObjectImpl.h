//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PICKABLEOBJECTIMPL_H
#define RAMSES_PICKABLEOBJECTIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"
#include "NodeImpl.h"

namespace ramses
{
    class ArrayBufferImpl;
    class ArrayBuffer;
    class CameraNodeImpl;
    class Camera;

    class PickableObjectImpl final : public NodeImpl
    {
    public:
        PickableObjectImpl(SceneImpl& scene, const char* pickableObjectName);
        ~PickableObjectImpl() override = default;

        void         initializeFrameworkData(const ArrayBufferImpl& geometryBuffer, pickableObjectId_t id);
        void deinitializeFrameworkData() override;
        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        status_t validate() const override;

        const ArrayBuffer& getGeometryBuffer() const;

        status_t setCamera(const CameraNodeImpl& cameraImpl);
        const Camera* getCamera() const;

        status_t setPickableObjectId(pickableObjectId_t id);
        pickableObjectId_t getPickableObjectId() const;

        status_t setEnabled(bool enable);
        bool     isEnabled() const;

        ramses_internal::PickableObjectHandle getPickableObjectHandle() const;

    private:
        ramses_internal::PickableObjectHandle m_pickableObjectHandle;
        const ArrayBufferImpl*                m_geometryBufferImpl = nullptr;
        const CameraNodeImpl*                 m_cameraImpl = nullptr;
    };
}

#endif
