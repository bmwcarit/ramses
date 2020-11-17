//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneObjectImpl.h"
#include "SceneImpl.h"

#include "SerializationContext.h"

namespace ramses
{
    SceneObjectImpl::SceneObjectImpl(SceneImpl& scene, ERamsesObjectType type, const char* name)
        : ClientObjectImpl(scene.getClientImpl(), type, name)
        , m_scene(scene)
        , m_sceneObjectId(scene.getNextSceneObjectId())
    {
        m_scene.getStatisticCollection().statObjectsCreated.incCounter(1);
    }

    SceneObjectImpl::~SceneObjectImpl()
    {
        m_scene.getStatisticCollection().statObjectsDestroyed.incCounter(1);
    }

    const SceneImpl& SceneObjectImpl::getSceneImpl() const
    {
        return m_scene;
    }

    SceneImpl& SceneObjectImpl::getSceneImpl()
    {
        return m_scene;
    }

    const ramses_internal::ClientScene& SceneObjectImpl::getIScene() const
    {
        return m_scene.getIScene();
    }

    ramses_internal::ClientScene& SceneObjectImpl::getIScene()
    {
        return m_scene.getIScene();
    }

    bool SceneObjectImpl::isFromTheSameSceneAs(const SceneObjectImpl& otherObject) const
    {
        return &getIScene() == &(otherObject.getIScene());
    }

    status_t SceneObjectImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(RamsesObjectImpl::serialize(outStream, serializationContext));
        assert(m_sceneObjectId.isValid());
        outStream << (serializationContext.getSerializeSceneObjectIds() ? m_sceneObjectId.getValue() : sceneObjectId_t::Invalid().getValue());

        return StatusOK;
    }

    status_t SceneObjectImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(RamsesObjectImpl::deserialize(inStream, serializationContext));
        sceneObjectId_t id;
        inStream >> id.getReference();

        if (id.isValid())
            m_sceneObjectId = std::move(id);

        return StatusOK;
    }

    sceneObjectId_t SceneObjectImpl::getSceneObjectId() const
    {
        return m_sceneObjectId;
    }

    ramses::sceneId_t SceneObjectImpl::getSceneId() const
    {
        return m_scene.getSceneId();
    }

}
