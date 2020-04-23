//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BLITPASSIMPL_H
#define RAMSES_BLITPASSIMPL_H

#include "SceneObjectImpl.h"
#include "SceneAPI/Handles.h"

namespace ramses
{
    class RenderBufferImpl;
    class RenderBuffer;

    class BlitPassImpl final : public SceneObjectImpl
    {
    public:
        BlitPassImpl(SceneImpl& scene, const char* blitpassName);
        virtual ~BlitPassImpl();

        void             initializeFrameworkData(const RenderBufferImpl& sourceRenderBuffer, const RenderBufferImpl& destinationRenderBuffer);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        const RenderBuffer& getSourceRenderBuffer() const;
        const RenderBuffer& getDestinationRenderBuffer() const;

        status_t         setBlittingRegion(uint32_t sourceX, uint32_t sourceY, uint32_t destinationX, uint32_t destinationY, uint32_t width, uint32_t height);
        void             getBlittingRegion(uint32_t& sourceX, uint32_t& sourceY, uint32_t& destinationX, uint32_t& destinationY, uint32_t& width, uint32_t& height) const;

        status_t         setRenderOrder(int32_t renderOrder);
        int32_t          getRenderOrder() const;

        status_t         setEnabled(bool isEnabeld);
        bool             isEnabled() const;

        ramses_internal::BlitPassHandle getBlitPassHandle() const;

    private:
        ramses_internal::BlitPassHandle m_blitPassHandle;
        const RenderBufferImpl* m_sourceRenderBufferImpl = nullptr;
        const RenderBufferImpl* m_destinationRenderBufferImpl = nullptr;
    };
}

#endif
