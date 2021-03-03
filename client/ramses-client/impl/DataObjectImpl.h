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
#include "SceneAPI/EDataType.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "Math3d/Matrix44f.h"
#include "Math3d/Vector2i.h"
#include "Math3d/Vector3i.h"
#include "Math3d/Vector4i.h"

namespace ramses_internal
{
    class IScene;
}

namespace ramses
{
    class DataObjectImpl final : public SceneObjectImpl
    {
    public:
        DataObjectImpl(SceneImpl& scene, ERamsesObjectType type, const char* name);
        virtual ~DataObjectImpl() override;

        void             initializeFrameworkData();
        virtual void     deinitializeFrameworkData() override;

        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;

        // Setters for the stored value
        template <typename T>
        status_t setValue(const T& value);

        // Getters for the stored value
        template <typename T>
        status_t getValue(T& value) const;

        ramses_internal::EDataType getDataType() const;
        ramses_internal::DataInstanceHandle getDataReference() const;

    private:
        static ramses_internal::EDataType GetDataTypeForDataObjectType(ERamsesObjectType type);

        ramses_internal::EDataType m_dataType;

        ramses_internal::DataLayoutHandle   m_layoutHandle;
        ramses_internal::DataInstanceHandle m_dataReference;
    };
}

#endif
