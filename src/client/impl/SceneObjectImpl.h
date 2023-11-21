//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/ClientObjectImpl.h"
#include "impl/RamsesClientTypesImpl.h"
#include <string_view>

namespace ramses
{
    class Scene;
}

namespace ramses::internal
{
    class ClientScene;
    class SceneImpl;

    class SceneObjectImpl : public ClientObjectImpl
    {
    public:
        explicit SceneObjectImpl(SceneImpl& scene, ERamsesObjectType type, std::string_view name);
        ~SceneObjectImpl() override;

        [[nodiscard]] sceneObjectId_t getSceneObjectId() const;
        [[nodiscard]] const Scene& getScene() const;
        [[nodiscard]] Scene& getScene();

        // impl methods
        [[nodiscard]] const SceneImpl& getSceneImpl() const;
        [[nodiscard]] SceneImpl&       getSceneImpl();
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        [[nodiscard]] const ramses::internal::ClientScene& getIScene() const;
        [[nodiscard]] ramses::internal::ClientScene&       getIScene();

        [[nodiscard]] bool isFromTheSameSceneAs(const SceneObjectImpl& otherObject) const;

        [[nodiscard]] std::string getIdentificationString() const final;

    protected:
        sceneObjectId_t m_sceneObjectId;

    private:
        SceneImpl& m_scene;
    };
}
