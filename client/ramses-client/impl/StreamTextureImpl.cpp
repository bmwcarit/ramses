//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StreamTextureImpl.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"
#include "Scene/ClientScene.h"
#include "RamsesClientImpl.h"
#include "Texture2DImpl.h"
#include "ramses-client-api/Texture2D.h"
#include "RamsesObjectTypeUtils.h"
#include "SceneAPI/WaylandIviSurfaceId.h"

namespace ramses
{
    StreamTextureImpl::StreamTextureImpl(SceneImpl& scene, const char* name)
        : SceneObjectImpl(scene, ERamsesObjectType_StreamTexture, name )
        , m_streamTextureHandle(ramses_internal::StreamTextureHandle::Invalid())
    {
    }

    StreamTextureImpl::~StreamTextureImpl()
    {
    }

    ramses::status_t StreamTextureImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(SceneObjectImpl::serialize(outStream, serializationContext));

        outStream << m_streamTextureHandle;

        return StatusOK;
    }

    status_t StreamTextureImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(SceneObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_streamTextureHandle;

        return StatusOK;
    }

    bool StreamTextureImpl::getForceFallbackImage() const
    {
        return getIScene().getStreamTexture(m_streamTextureHandle).forceFallbackTexture;
    }

    waylandIviSurfaceId_t StreamTextureImpl::getStreamSource() const
    {
        return waylandIviSurfaceId_t{ getIScene().getStreamTexture(m_streamTextureHandle).source.getValue() };
    }

    void StreamTextureImpl::initializeFrameworkData(waylandIviSurfaceId_t source, const Texture2DImpl& fallbackTexture)
    {
        assert(!m_streamTextureHandle.isValid());
        m_streamTextureHandle = getIScene().allocateStreamTexture(ramses_internal::WaylandIviSurfaceId{ source.getValue() }, fallbackTexture.getLowlevelResourceHash());
        assert(m_streamTextureHandle.isValid());
    }

    void StreamTextureImpl::deinitializeFrameworkData()
    {
        assert(m_streamTextureHandle.isValid());
        getIScene().releaseStreamTexture(m_streamTextureHandle);
        m_streamTextureHandle = ramses_internal::StreamTextureHandle::Invalid();
    }

    ramses_internal::StreamTextureHandle StreamTextureImpl::getHandle() const
    {
        return m_streamTextureHandle;
    }

    ramses_internal::ResourceContentHash StreamTextureImpl::getFallbackTextureHash() const
    {
        return getIScene().getStreamTexture(m_streamTextureHandle).fallbackTexture;
    }

    status_t StreamTextureImpl::validate() const
    {
        status_t status = SceneObjectImpl::validate();

        const ramses_internal::ResourceContentHash fallbackTextureHash = getFallbackTextureHash();
        const Resource* resource = getSceneImpl().scanForResourceWithHash(fallbackTextureHash);
        if (!resource)
            return addValidationMessage(EValidationSeverity_Error, "StreamTexture is using a fallback texture which does not exist");

        assert(resource->getType() == ERamsesObjectType_Texture2D);
        const Texture2D& texture = RamsesObjectTypeUtils::ConvertTo<Texture2D>(*resource);
        return std::max(status, addValidationOfDependentObject(texture.impl));
    }

    status_t StreamTextureImpl::forceFallbackImage(bool forceFallbackImage)
    {
        getIScene().setForceFallbackImage(m_streamTextureHandle, forceFallbackImage);
        return StatusOK;
    }

}

