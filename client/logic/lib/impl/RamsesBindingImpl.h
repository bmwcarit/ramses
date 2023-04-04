//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/LogicNodeImpl.h"

namespace ramses
{
    class SceneObject;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    class FlatBufferBuilder;
}

namespace rlogic_serialization
{
    struct RamsesReference;
}

namespace rlogic::internal
{
    class RamsesBindingImpl : public LogicNodeImpl
    {
    public:
        explicit RamsesBindingImpl(std::string_view name, uint64_t id) noexcept;

    protected:
        // Used by subclasses to handle serialization
        [[nodiscard]] static flatbuffers::Offset<rlogic_serialization::RamsesReference> SerializeRamsesReference(const ramses::SceneObject& object, flatbuffers::FlatBufferBuilder& builder);

        void setRootInputs(std::unique_ptr<Property> rootInputs);

    private:
        using LogicNodeImpl::setRootProperties;
    };
}
