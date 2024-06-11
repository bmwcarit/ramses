//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/logic/LogicNodeImpl.h"

namespace ramses
{
    class SceneObject;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    template<bool> class FlatBufferBuilderImpl;
    using FlatBufferBuilder = FlatBufferBuilderImpl<false>;
}

namespace rlogic_serialization
{
    struct RamsesReference;
}

namespace ramses::internal
{
    class RamsesBindingImpl : public LogicNodeImpl
    {
    public:
        explicit RamsesBindingImpl(SceneImpl& scene, std::string_view name, sceneObjectId_t id, SceneObject& boundObject) noexcept;

        [[nodiscard]] const SceneObject& getBoundObject() const;

    protected:
        // Used by subclasses to handle serialization
        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RamsesReference> SerializeRamsesReference(const ramses::SceneObject& object, flatbuffers::FlatBufferBuilder& builder);

        void setRootInputs(std::unique_ptr<PropertyImpl> rootInputs);

        std::reference_wrapper<SceneObject> m_boundObject;

    private:
        using LogicNodeImpl::setRootProperties;
    };
}
