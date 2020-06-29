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
    class VertexDataBufferImpl;
    class VertexDataBuffer;
    class CameraNodeImpl;
    class LocalCamera;

    class PickableObjectImpl final : public NodeImpl
    {
    public:
        PickableObjectImpl(SceneImpl& scene, const char* pickableObjectName);
        virtual ~PickableObjectImpl() = default;

        void         initializeFrameworkData(const VertexDataBufferImpl& geometryBuffer, pickableObjectId_t id);
        virtual void deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        const VertexDataBuffer& getGeometryBuffer() const;

        status_t setCamera(const CameraNodeImpl& cameraImpl);
        const LocalCamera* getCamera() const;

        status_t setPickableObjectId(pickableObjectId_t id);
        pickableObjectId_t getPickableObjectId() const;

        status_t setEnabled(bool enable);
        bool     isEnabled() const;

        ramses_internal::PickableObjectHandle getPickableObjectHandle() const;

    private:
        ramses_internal::PickableObjectHandle m_pickableObjectHandle;
        const VertexDataBufferImpl*           m_geometryBufferImpl = nullptr;
        const CameraNodeImpl*                 m_cameraImpl = nullptr;
    };
}

#endif
