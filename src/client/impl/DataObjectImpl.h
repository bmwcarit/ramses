//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

//internal
#include "SceneObjectImpl.h"
#include "ramses/framework/EDataType.h"
#include "impl/DataTypesImpl.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include <string_view>

namespace ramses::internal
{
    class IScene;

    class DataObjectImpl final : public SceneObjectImpl
    {
    public:
        DataObjectImpl(SceneImpl& scene, ERamsesObjectType ramsesType, ramses::EDataType dataType, std::string_view name);
        ~DataObjectImpl() override;

        void     initializeFrameworkData();
        void     deinitializeFrameworkData() override;

        bool serialize(IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(IInputStream& inStream, DeserializationContext& serializationContext) override;

        [[nodiscard]] ramses::EDataType getDataType() const;

        template <typename T>
        bool setValue(const T& value);
        template <typename T>
        bool getValue(T& value) const;

        [[nodiscard]] DataInstanceHandle getDataReference() const;

    private:
        ramses::EDataType m_dataType;

        DataLayoutHandle   m_layoutHandle;
        DataInstanceHandle m_dataReference;
    };
}
