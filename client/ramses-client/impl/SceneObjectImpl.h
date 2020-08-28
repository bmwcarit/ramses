//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEOBJECTIMPL_H
#define RAMSES_SCENEOBJECTIMPL_H

#include "ClientObjectImpl.h"

namespace ramses_internal
{
    class ClientScene;
}

namespace ramses
{
    class SceneImpl;

    class SceneObjectImpl : public ClientObjectImpl
    {
    public:
        explicit SceneObjectImpl(SceneImpl& scene, ERamsesObjectType type, const char* name);
        virtual ~SceneObjectImpl();



        // impl methods
        const SceneImpl& getSceneImpl() const;
        SceneImpl&       getSceneImpl();
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        const ramses_internal::ClientScene& getIScene() const;
        ramses_internal::ClientScene&       getIScene();
        sceneObjectId_t getSceneObjectId() const;

        bool isFromTheSameSceneAs(const SceneObjectImpl& otherObject) const;

    private:
        SceneImpl& m_scene;
        sceneObjectId_t m_sceneObjectId;
    };
}

#endif
