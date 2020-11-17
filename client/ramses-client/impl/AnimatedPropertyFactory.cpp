//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimatedPropertyFactory.h"
#include "AnimatedPropertyUtils.h"
#include "AnimationSystemImpl.h"
#include "NodeImpl.h"
#include "AppearanceImpl.h"
#include "EffectInputImpl.h"
#include "DataObjectImpl.h"
#include "SceneImpl.h"
#include "Scene/ClientScene.h"
#include "Scene/SceneDataBinding.h"
#include "Utils/LogMacros.h"

namespace ramses
{
    AnimatedPropertyFactory::AnimatedPropertyFactory(AnimationSystemImpl& animationSystem)
        : m_animationSystem(animationSystem)
    {
    }

    AnimatedProperty* AnimatedPropertyFactory::createAnimatedProperty(const NodeImpl& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        AnimatedPropertyImpl* pimpl = createAnimatedPropertyImpl(propertyOwner, ePropertyComponent, bindID, name);
        if (pimpl != nullptr)
        {
            AnimatedProperty* animProperty = new AnimatedProperty(*pimpl);
            return animProperty;
        }

        return nullptr;
    }

    AnimatedProperty* AnimatedPropertyFactory::createAnimatedProperty(const EffectInputImpl& propertyOwner, const AppearanceImpl& appearance, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        AnimatedPropertyImpl* pimpl = createAnimatedPropertyImpl(propertyOwner, appearance, ePropertyComponent, bindID, name);
        if (pimpl != nullptr)
        {
            AnimatedProperty* animProperty = new AnimatedProperty(*pimpl);
            return animProperty;
        }

        return nullptr;
    }

    AnimatedProperty* AnimatedPropertyFactory::createAnimatedProperty(const DataObjectImpl& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        AnimatedPropertyImpl* pimpl = createAnimatedPropertyImpl(propertyOwner, ePropertyComponent, bindID, name);
        if (pimpl != nullptr)
        {
            AnimatedProperty* animProperty = new AnimatedProperty(*pimpl);
            return animProperty;
        }

        return nullptr;
    }

    AnimatedPropertyImpl* AnimatedPropertyFactory::createAnimatedPropertyImpl(const NodeImpl& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        if (!AnimatedPropertyUtils::isComponentMatchingTransformNode(ePropertyComponent))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimatedPropertyFactory::createAnimatedProperty:  cannot initialize animated property, requested property and/or component is not compatible");
            return nullptr;
        }

        if (!m_animationSystem.getSceneImpl().containsSceneObject(propertyOwner))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimatedPropertyFactory::createAnimatedProperty:  cannot initialize animated property, requested property and animation system do not belong to the same scene");
            return nullptr;
        }

        using ContainerTraitsClass = ramses_internal::DataBindContainerToTraitsSelector<ramses_internal::IScene>::ContainerTraitsClassType;
        if (bindID == ContainerTraitsClass::TransformNode_Rotation
            && propertyOwner.getIScene().getRotationConvention(propertyOwner.getTransformHandle()) != ramses_internal::ERotationConvention::Legacy_ZYX)
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimatedPropertyFactory::createAnimatedProperty:  cannot initialize animated property, can not animate rotation for node that does not use legacy rotation convention");
            return nullptr;
        }

        const ramses_internal::MemoryHandle handle1 = propertyOwner.getTransformHandle().asMemoryHandle();
        const ramses_internal::MemoryHandle handle2 = ramses_internal::InvalidMemoryHandle;

        return createAnimatedPropertyImpl(handle1, handle2, ePropertyComponent, bindID, name);
    }

    AnimatedPropertyImpl* AnimatedPropertyFactory::createAnimatedPropertyImpl(const EffectInputImpl& propertyOwner, const AppearanceImpl& appearance, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        if (!AnimatedPropertyUtils::isComponentMatchingEffectInput(ePropertyComponent, propertyOwner.getDataType()))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimatedPropertyFactory::createAnimatedProperty:  cannot initialize animated property, requested property and/or component is not compatible");
            return nullptr;
        }

        if (!m_animationSystem.getSceneImpl().containsSceneObject(appearance))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimatedPropertyFactory::createAnimatedProperty:  cannot initialize animated property, requested property and animation system do not belong to the same scene");
            return nullptr;
        }

        ramses_internal::MemoryHandle handle2 = ramses_internal::InvalidMemoryHandle;
        ramses_internal::MemoryHandle handle1 = ramses_internal::InvalidMemoryHandle;

        const ramses_internal::DataLayoutHandle dataLayout = m_animationSystem.getIScene().getLayoutOfDataInstance(appearance.getUniformDataInstance());
        const ramses_internal::DataFieldHandle dataField(propertyOwner.getInputIndex());
        const ramses_internal::EDataType dataType = m_animationSystem.getIScene().getDataLayout(dataLayout).getField(dataField).dataType;
        if (dataType == ramses_internal::EDataType::DataReference)
        {
            const ramses_internal::DataInstanceHandle dataRef = m_animationSystem.getIScene().getDataReference(appearance.getUniformDataInstance(), dataField);
            handle1 = dataRef.asMemoryHandle();
            handle2 = 0u;
        }
        else
        {
            handle1 = appearance.getUniformDataInstance().asMemoryHandle();
            handle2 = propertyOwner.getInputIndex();
        }

        return createAnimatedPropertyImpl(handle1, handle2, ePropertyComponent, bindID, name);
    }

    AnimatedPropertyImpl* AnimatedPropertyFactory::createAnimatedPropertyImpl(const DataObjectImpl& propertyOwner, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        if (!AnimatedPropertyUtils::isComponentMatchingEffectInput(ePropertyComponent, propertyOwner.getDataType()))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimatedPropertyFactory::createAnimatedProperty:  cannot initialize animated property, requested property and/or component is not compatible");
            return nullptr;
        }

        if (!m_animationSystem.getSceneImpl().containsSceneObject(propertyOwner))
        {
            LOG_ERROR(ramses_internal::CONTEXT_CLIENT, "AnimatedPropertyFactory::createAnimatedProperty:  cannot initialize animated property, requested property and animation system do not belong to the same scene");
            return nullptr;
        }

        const ramses_internal::MemoryHandle handle1 = propertyOwner.getDataReference().asMemoryHandle();
        const ramses_internal::MemoryHandle handle2 = 0u;

        return createAnimatedPropertyImpl(handle1, handle2, ePropertyComponent, bindID, name);
    }

    AnimatedPropertyImpl* AnimatedPropertyFactory::createAnimatedPropertyImpl(ramses_internal::MemoryHandle handle1, ramses_internal::MemoryHandle handle2, EAnimatedPropertyComponent ePropertyComponent, ramses_internal::TDataBindID bindID, const char* name)
    {
        typedef ramses_internal::DataBindContainerToTraitsSelector<ramses_internal::IScene>::ContainerTraitsClassType ContainerTraitsClass;
        const ramses_internal::DataBindContainerTraits<ramses_internal::IScene>& containerTraits = ContainerTraitsClass::m_dataBindTraits[bindID];
        const ramses_internal::TDataBindID dataBindID = bindID;
        const ramses_internal::EVectorComponent vectorComponent = AnimatedPropertyUtils::getVectorComponentFromProperty(ePropertyComponent);
        const ramses_internal::EDataTypeID dataTypeID = containerTraits.m_eDataTypeID;

        AnimatedPropertyImpl* propertyData = new AnimatedPropertyImpl(m_animationSystem, name);
        propertyData->initializeFrameworkData(m_animationSystem.getIScene(), handle1, handle2, dataBindID, vectorComponent, dataTypeID);

        return propertyData;
    }
}
