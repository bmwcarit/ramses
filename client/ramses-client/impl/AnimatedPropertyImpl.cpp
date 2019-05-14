//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimatedPropertyImpl.h"
#include "AnimationSystemImpl.h"
#include "SerializationContext.h"
#include "AnimationAPI/IAnimationSystem.h"
#include "Animation/AnimationDataBind.h"
#include "Animation/AnimationInstance.h"
#include "Collections/IOutputStream.h"
#include "Collections/IInputStream.h"

namespace ramses
{
    AnimatedPropertyImpl::AnimatedPropertyImpl(AnimationSystemImpl& animationSystem, const char* name)
        : AnimationObjectImpl(animationSystem, ERamsesObjectType_AnimatedProperty, name)
        , m_vectorComponent(ramses_internal::EVectorComponent_All)
        , m_dataTypeID(ramses_internal::EDataTypeID_Invalid)
    {
    }

    void AnimatedPropertyImpl::initializeFrameworkData(
        ramses_internal::IScene&                   scene,
        ramses_internal::MemoryHandle              handle1,
        ramses_internal::MemoryHandle              handle2,
        ramses_internal::TDataBindID               dataBindID,
        ramses_internal::EVectorComponent          vectorComponent,
        ramses_internal::EDataTypeID               dataTypeID)
    {
        assert(!m_dataBindHandle.isValid());
        m_dataBindHandle = getIAnimationSystem().allocateDataBinding(scene, dataBindID, handle1, handle2);

        m_vectorComponent = vectorComponent;
        m_dataTypeID = dataTypeID;
    }

    void AnimatedPropertyImpl::deinitializeFrameworkData()
    {
        getIAnimationSystem().removeDataBinding(m_dataBindHandle);
        m_dataBindHandle = ramses_internal::DataBindHandle::Invalid();
    }

    status_t AnimatedPropertyImpl::serialize(ramses_internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::serialize(outStream, serializationContext));

        outStream << m_dataBindHandle;
        outStream << static_cast<uint32_t>(m_vectorComponent);
        outStream << static_cast<uint32_t>(m_dataTypeID);

        return StatusOK;
    }

    status_t AnimatedPropertyImpl::deserialize(ramses_internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        CHECK_RETURN_ERR(AnimationObjectImpl::deserialize(inStream, serializationContext));

        inStream >> m_dataBindHandle;
        uint32_t enumInt = 0u;
        inStream >> enumInt;
        m_vectorComponent = static_cast<ramses_internal::EVectorComponent>(enumInt);
        inStream >> enumInt;
        m_dataTypeID = static_cast<ramses_internal::EDataTypeID>(enumInt);

        return StatusOK;
    }

    status_t AnimatedPropertyImpl::validate(uint32_t indent) const
    {
        status_t status = AnimationObjectImpl::validate(indent);
        indent += IndentationStep;

        if (!getIAnimationSystem().getDataBinding(m_dataBindHandle)->isPropertyValid())
        {
            addValidationMessage(EValidationSeverity_Error, indent, "property to animate does not exist in scene");
            status = getValidationErrorStatus();
        }

        // find all animations using this property
        uint32_t propertyUsageCount = 0u;
        const uint32_t animInstanceCount = getIAnimationSystem().getTotalAnimationInstanceCount();
        for (ramses_internal::AnimationInstanceHandle animInstHandle(0u); animInstHandle < animInstanceCount; ++animInstHandle)
        {
            if (getIAnimationSystem().containsAnimationInstance(animInstHandle))
            {
                const ramses_internal::AnimationInstance& animInst = getIAnimationSystem().getAnimationInstance(animInstHandle);
                if (contains_c(animInst.getDataBindings(), m_dataBindHandle))
                {
                    ++propertyUsageCount;
                }
            }
        }

        if (propertyUsageCount > 1u)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "this AnimatedProperty seems to be used by multiple animations, this is fine only if those animations do not overlap");
            status = getValidationErrorStatus();
        }

        return status;
    }

    ramses_internal::DataBindHandle AnimatedPropertyImpl::getDataBindHandle() const
    {
        return m_dataBindHandle;
    }

    ramses_internal::EVectorComponent AnimatedPropertyImpl::getVectorComponent() const
    {
        return m_vectorComponent;
    }

    ramses_internal::EDataTypeID AnimatedPropertyImpl::getDataTypeID() const
    {
        return m_dataTypeID;
    }
}
