//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/RamsesBindingImpl.h"
#include "ramses/client/logic/Property.h"

#include "ramses/client/SceneObject.h"

#include "internal/logic/flatbuffers/generated/RamsesReferenceGen.h"

namespace ramses::internal
{
    RamsesBindingImpl::RamsesBindingImpl(SceneImpl& scene, std::string_view name, sceneObjectId_t id, SceneObject& boundObject) noexcept
        : LogicNodeImpl{ scene, name, id }
        , m_boundObject{ boundObject }
    {
        // Bindings are not supposed to do anything unless user set an actual value to them
        // Thus, they are not dirty by default!
        setDirty(false);
    }

    flatbuffers::Offset<rlogic_serialization::RamsesReference> RamsesBindingImpl::SerializeRamsesReference(const ramses::SceneObject& object, flatbuffers::FlatBufferBuilder& builder)
    {
        const ramses::sceneObjectId_t ramsesObjectId = object.getSceneObjectId();
        const ramses::ERamsesObjectType ramsesObjectType = object.getType();

        auto ramsesRef = rlogic_serialization::CreateRamsesReference(builder,
            ramsesObjectId.getValue(),
            static_cast<uint32_t>(ramsesObjectType)
        );
        builder.Finish(ramsesRef);

        return ramsesRef;
    }

    const SceneObject& RamsesBindingImpl::getBoundObject() const
    {
        return m_boundObject;
    }

    void RamsesBindingImpl::setRootInputs(std::unique_ptr<PropertyImpl> rootInputs)
    {
        setRootProperties(std::move(rootInputs), {});
    }
}
