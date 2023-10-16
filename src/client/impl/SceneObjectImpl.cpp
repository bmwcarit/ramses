//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneObjectImpl.h"
#include "ramses/client/Scene.h"
#include "impl/SceneImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/SerializationContext.h"
#include "impl/RamsesFrameworkTypesImpl.h"

namespace ramses::internal
{
    SceneObjectImpl::SceneObjectImpl(SceneImpl& scene, ERamsesObjectType type, std::string_view name)
        : ClientObjectImpl{ scene.getClientImpl(), type, name }
        , m_sceneObjectId{ scene.getNextSceneObjectId() }
        , m_scene{ scene }
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

    const ramses::internal::ClientScene& SceneObjectImpl::getIScene() const
    {
        return m_scene.getIScene();
    }

    ramses::internal::ClientScene& SceneObjectImpl::getIScene()
    {
        return m_scene.getIScene();
    }

    bool SceneObjectImpl::isFromTheSameSceneAs(const SceneObjectImpl& otherObject) const
    {
        return &getIScene() == &(otherObject.getIScene());
    }

    bool SceneObjectImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!RamsesObjectImpl::serialize(outStream, serializationContext))
            return false;

        assert(m_sceneObjectId.isValid());
        outStream << (serializationContext.getSerializeSceneObjectIds() ? m_sceneObjectId.getValue() : sceneObjectId_t::Invalid().getValue());

        return true;
    }

    bool SceneObjectImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!RamsesObjectImpl::deserialize(inStream, serializationContext))
            return true;

        sceneObjectId_t id;
        inStream >> id.getReference();

        if (id.isValid())
            m_sceneObjectId = std::move(id);

        return true;
    }

    sceneObjectId_t SceneObjectImpl::getSceneObjectId() const
    {
        return m_sceneObjectId;
    }

    const Scene& SceneObjectImpl::getScene() const
    {
        return RamsesObjectTypeUtils::ConvertTo<Scene>(m_scene.getRamsesObject());
    }

    Scene& SceneObjectImpl::getScene()
    {
        return RamsesObjectTypeUtils::ConvertTo<Scene>(m_scene.getRamsesObject());
    }

    std::string SceneObjectImpl::getIdentificationString() const
    {
        auto idString = RamsesObjectImpl::getIdentificationString();
        idString.insert(idString.size() - 1, fmt::format(" ScnObjId={}", m_sceneObjectId));

        return idString;
    }

    void SceneObjectImpl::setObjectRegistryHandle(SceneObjectRegistryHandle handle)
    {
        m_objectRegistryHandle = handle;
    }

    SceneObjectRegistryHandle SceneObjectImpl::getObjectRegistryHandle() const
    {
        return m_objectRegistryHandle;
    }
}
