//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DATAOBJECTIMPL_H
#define RAMSES_DATAOBJECTIMPL_H

//internal
#include "SceneObjectImpl.h"
#include "ramses-framework-api/EDataType.h"
#include "ramses-framework-api/DataTypes.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    class IScene;
}

namespace ramses
{
    class DataObjectImpl final : public SceneObjectImpl
    {
    public:
        DataObjectImpl(SceneImpl& scene, ERamsesObjectType ramsesType, EDataType dataType, const char* name);
        ~DataObjectImpl() override;

        void     initializeFrameworkData();
        void     deinitializeFrameworkData() override;

        status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        EDataType getDataType() const;

        template <typename T>
        status_t setValue(const T& value);
        template <typename T>
        status_t getValue(T& value) const;

        ramses_internal::DataInstanceHandle getDataReference() const;

    private:
        EDataType m_dataType;

        ramses_internal::DataLayoutHandle   m_layoutHandle;
        ramses_internal::DataInstanceHandle m_dataReference;
    };
}

#endif
