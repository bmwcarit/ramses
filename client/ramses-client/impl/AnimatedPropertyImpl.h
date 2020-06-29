//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATEDPROPERTYIMPL_H
#define RAMSES_ANIMATEDPROPERTYIMPL_H

#include "AnimationObjectImpl.h"
#include "Utils/DataBindCommon.h"
#include "Animation/AnimationCommon.h"

namespace ramses_internal
{
    class IScene;
    class IAnimationSystem;
}

namespace ramses
{
    class AnimatedPropertyImpl final : public AnimationObjectImpl
    {
    public:
        AnimatedPropertyImpl(AnimationSystemImpl& animationSystem, const char* name);

        void             initializeFrameworkData(
            ramses_internal::IScene&                   scene,
            ramses_internal::MemoryHandle              handle1,
            ramses_internal::MemoryHandle              handle2,
            ramses_internal::TDataBindID               dataBindID,
            ramses_internal::EVectorComponent          vectorComponent,
            ramses_internal::EDataTypeID               dataTypeID);
        virtual void     deinitializeFrameworkData() override;
        virtual status_t serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        virtual status_t deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        virtual status_t validate(uint32_t indent, StatusObjectSet& visitedObjects) const override;

        ramses_internal::DataBindHandle            getDataBindHandle() const;
        ramses_internal::EVectorComponent          getVectorComponent() const;
        ramses_internal::EDataTypeID               getDataTypeID() const;

    private:
        ramses_internal::DataBindHandle            m_dataBindHandle;
        ramses_internal::EVectorComponent          m_vectorComponent;
        ramses_internal::EDataTypeID               m_dataTypeID;
    };
}

#endif
