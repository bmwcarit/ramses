//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesCameraBindingImpl.h"

#include "ramses-utils.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/PerspectiveCamera.h"

#include "ramses-logic/EPropertyType.h"
#include "ramses-logic/Property.h"

#include "impl/PropertyImpl.h"
#include "impl/LoggerImpl.h"

#include "internals/RamsesHelper.h"
#include "internals/ErrorReporting.h"
#include "internals/RamsesObjectResolver.h"

#include "generated/RamsesCameraBindingGen.h"

namespace rlogic::internal
{
    RamsesCameraBindingImpl::RamsesCameraBindingImpl(ramses::Camera& ramsesCamera, bool withFrustumPlanes, std::string_view name, uint64_t id)
        : RamsesBindingImpl(name, id)
        , m_ramsesCamera(ramsesCamera)
        , m_hasFrustumPlanesProperties{ ramsesCamera.isOfType(ramses::ERamsesObjectType_OrthographicCamera) || withFrustumPlanes }
    {
    }

    void RamsesCameraBindingImpl::createRootProperties()
    {
        std::vector<TypeData> frustumPlanes = {
            TypeData{ "nearPlane", EPropertyType::Float },
            TypeData{ "farPlane", EPropertyType::Float },
        };

        if (m_hasFrustumPlanesProperties)
        {
            // Attention! This order is important - it has to match the indices in ECameraFrustumPlanesPropertyStaticIndex
            frustumPlanes.emplace_back("leftPlane", EPropertyType::Float);
            frustumPlanes.emplace_back("rightPlane", EPropertyType::Float);
            frustumPlanes.emplace_back("bottomPlane", EPropertyType::Float);
            frustumPlanes.emplace_back("topPlane", EPropertyType::Float);
        }
        else
        {
            // Attention! This order is important - it has to match the indices in EPerspectiveCameraFrustumPropertyStaticIndex
            frustumPlanes.emplace_back("fieldOfView", EPropertyType::Float);
            frustumPlanes.emplace_back("aspectRatio", EPropertyType::Float);
        }

        HierarchicalTypeData cameraBindingInputs(
            TypeData{"", EPropertyType::Struct},
            {
                MakeStruct("viewport",
                    {
                        // Attention! This order is important - it has to match the indices in ECameraViewportPropertyStaticIndex
                        TypeData{"offsetX", EPropertyType::Int32},
                        TypeData{"offsetY", EPropertyType::Int32},
                        TypeData{"width", EPropertyType::Int32},
                        TypeData{"height", EPropertyType::Int32}
                    }
                ),
                MakeStruct("frustum", frustumPlanes),
            }
        );

        setRootInputs(std::make_unique<Property>(std::make_unique<PropertyImpl>(cameraBindingInputs, EPropertySemantics::BindingInput)));

        ApplyRamsesValuesToInputProperties(*this);
    }

    flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding> RamsesCameraBindingImpl::Serialize(
        const RamsesCameraBindingImpl& cameraBinding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap,
        EFeatureLevel /*featureLevel*/)
    {
        auto ramsesReference = RamsesBindingImpl::SerializeRamsesReference(cameraBinding.m_ramsesCamera, builder);

        const auto logicObject = LogicObjectImpl::Serialize(cameraBinding, builder);
        const auto propertyObject = PropertyImpl::Serialize(*cameraBinding.getInputs()->m_impl, builder, serializationMap);
        auto ramsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            ramsesReference,
            propertyObject);
        builder.Finish(ramsesBinding);

        auto ramsesCameraBinding = rlogic_serialization::CreateRamsesCameraBinding(builder, ramsesBinding);
        builder.Finish(ramsesCameraBinding);

        return ramsesCameraBinding;
    }

    std::unique_ptr<RamsesCameraBindingImpl> RamsesCameraBindingImpl::Deserialize(
        const rlogic_serialization::RamsesCameraBinding& cameraBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!cameraBinding.base())
        {
            errorReporting.add("Fatal error during loading of RamsesCameraBinding from serialized data: missing base class info!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(cameraBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of RamsesCameraBinding from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!cameraBinding.base()->rootInput())
        {
            errorReporting.add("Fatal error during loading of RamsesCameraBinding from serialized data: missing root input!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*cameraBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);
        if (!deserializedRootInput)
            return nullptr;

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.add("Fatal error during loading of RamsesCameraBinding from serialized data: root input has unexpected type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const auto frustumInputProp = deserializedRootInput->getChild("frustum");
        if (!frustumInputProp || !(frustumInputProp->getChildCount() == 4u || frustumInputProp->getChildCount() == 6u))
        {
            errorReporting.add("Fatal error during loading of RamsesCameraBinding from serialized data: missing or invalid input properties!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }
        const bool hasFrustumPlanesProperties = (frustumInputProp->getChildCount() == 6u);

        const auto* boundObject = cameraBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.add("Fatal error during loading of RamsesCameraBinding from serialized data: no reference to ramses camera!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId(boundObject->objectId());

        ramses::Camera* resolvedCamera = ramsesResolver.findRamsesCameraInScene(name, objectId);
        if (!resolvedCamera)
            return nullptr;

        if (resolvedCamera->getType() != static_cast<int>(boundObject->objectType()))
        {
            errorReporting.add("Fatal error during loading of RamsesCameraBinding from serialized data: loaded type does not match referenced camera type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto binding = std::make_unique<RamsesCameraBindingImpl>(*resolvedCamera, hasFrustumPlanesProperties, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::make_unique<Property>(std::move(deserializedRootInput)));

        ApplyRamsesValuesToInputProperties(*binding);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> RamsesCameraBindingImpl::update()
    {
        ramses::status_t status = ramses::StatusOK;
        PropertyImpl& vpProperties = *getInputs()->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Viewport))->m_impl;

        PropertyImpl& vpOffsetX = *vpProperties.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetX))->m_impl;
        PropertyImpl& vpOffsetY = *vpProperties.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetY))->m_impl;
        PropertyImpl& vpWidth = *vpProperties.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortWidth))->m_impl;
        PropertyImpl& vpHeight = *vpProperties.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortHeight))->m_impl;
        if (vpOffsetX.checkForBindingInputNewValueAndReset()
            || vpOffsetY.checkForBindingInputNewValueAndReset()
            || vpWidth.checkForBindingInputNewValueAndReset()
            || vpHeight.checkForBindingInputNewValueAndReset())
        {
            const int32_t vpX = vpOffsetX.getValueAs<int32_t>();
            const int32_t vpY = vpOffsetY.getValueAs<int32_t>();
            const int32_t vpW = vpWidth.getValueAs<int32_t>();
            const int32_t vpH = vpHeight.getValueAs<int32_t>();

            if (vpW <= 0 || vpH <= 0)
            {
                return LogicNodeRuntimeError{ fmt::format("Camera viewport size must be positive! (width: {}; height: {})", vpW, vpH) };
            }

            status = m_ramsesCamera.get().setViewport(vpX, vpY, vpW, vpH);

            if (status != ramses::StatusOK)
            {
                return LogicNodeRuntimeError{m_ramsesCamera.get().getStatusMessage(status)};
            }
        }

        PropertyImpl& frustum = *getInputs()->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Frustum))->m_impl;

        // Index of Perspective Frustum Properties is used, but wouldn't matter as Ortho Camera indeces are the same for these two properties
        PropertyImpl& nearPlane = *frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::NearPlane))->m_impl;
        PropertyImpl& farPlane = *frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FarPlane))->m_impl;

        if (m_hasFrustumPlanesProperties)
        {
            PropertyImpl& leftPlane = *frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::LeftPlane))->m_impl;
            PropertyImpl& rightPlane = *frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::RightPlane))->m_impl;
            PropertyImpl& bottomPlane = *frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::BottomPlane))->m_impl;
            PropertyImpl& topPlane = *frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::TopPlane))->m_impl;

            if (nearPlane.checkForBindingInputNewValueAndReset()
                || farPlane.checkForBindingInputNewValueAndReset()
                || leftPlane.checkForBindingInputNewValueAndReset()
                || rightPlane.checkForBindingInputNewValueAndReset()
                || bottomPlane.checkForBindingInputNewValueAndReset()
                || topPlane.checkForBindingInputNewValueAndReset())
            {
                status = m_ramsesCamera.get().setFrustum(
                    leftPlane.getValueAs<float>(),
                    rightPlane.getValueAs<float>(),
                    bottomPlane.getValueAs<float>(),
                    topPlane.getValueAs<float>(),
                    nearPlane.getValueAs<float>(),
                    farPlane.getValueAs<float>());

                if (status != ramses::StatusOK)
                    return LogicNodeRuntimeError{ m_ramsesCamera.get().getStatusMessage(status) };
            }
        }
        else
        {
            PropertyImpl& fov = *frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FieldOfView))->m_impl;
            PropertyImpl& aR = *frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::AspectRatio))->m_impl;

            if (nearPlane.checkForBindingInputNewValueAndReset()
                || farPlane.checkForBindingInputNewValueAndReset()
                || fov.checkForBindingInputNewValueAndReset()
                || aR.checkForBindingInputNewValueAndReset())
            {
                assert(m_ramsesCamera.get().isOfType(ramses::ERamsesObjectType_PerspectiveCamera));
                auto* perspectiveCam = ramses::RamsesUtils::TryConvert<ramses::PerspectiveCamera>(m_ramsesCamera.get());
                status = perspectiveCam->setFrustum(fov.getValueAs<float>(), aR.getValueAs<float>(), nearPlane.getValueAs<float>(), farPlane.getValueAs<float>());

                if (status != ramses::StatusOK)
                    return LogicNodeRuntimeError{ m_ramsesCamera.get().getStatusMessage(status) };
            }
        }

        return std::nullopt;
    }

    bool RamsesCameraBindingImpl::hasFrustumPlanesProperties() const
    {
        return m_hasFrustumPlanesProperties;
    }

    ramses::Camera& RamsesCameraBindingImpl::getRamsesCamera() const
    {
        return m_ramsesCamera;
    }

    void RamsesCameraBindingImpl::ApplyRamsesValuesToInputProperties(RamsesCameraBindingImpl& binding)
    {
        const auto& ramsesCamera = binding.getRamsesCamera();

        // Initializes input values with values from ramses camera silently (no dirty mechanism triggered)
        PropertyImpl& viewport = *binding.getInputs()->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Viewport))->m_impl;
        viewport.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetX))->m_impl->initializeBindingInputValue(PropertyValue{ ramsesCamera.getViewportX() });
        viewport.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetY))->m_impl->initializeBindingInputValue(PropertyValue{ ramsesCamera.getViewportY() });
        viewport.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortWidth))->m_impl->initializeBindingInputValue(PropertyValue{ static_cast<int32_t>(ramsesCamera.getViewportWidth()) });
        viewport.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortHeight))->m_impl->initializeBindingInputValue(PropertyValue{ static_cast<int32_t>(ramsesCamera.getViewportHeight()) });

        PropertyImpl& frustum = *binding.getInputs()->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Frustum))->m_impl;
        frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::NearPlane))->m_impl->initializeBindingInputValue(PropertyValue{ ramsesCamera.getNearPlane() });
        frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::FarPlane))->m_impl->initializeBindingInputValue(PropertyValue{ ramsesCamera.getFarPlane() });
        if (binding.hasFrustumPlanesProperties())
        {
            frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::LeftPlane))->m_impl->initializeBindingInputValue(PropertyValue{ ramsesCamera.getLeftPlane() });
            frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::RightPlane))->m_impl->initializeBindingInputValue(PropertyValue{ ramsesCamera.getRightPlane() });
            frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::BottomPlane))->m_impl->initializeBindingInputValue(PropertyValue{ ramsesCamera.getBottomPlane() });
            frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::TopPlane))->m_impl->initializeBindingInputValue(PropertyValue{ ramsesCamera.getTopPlane() });
        }
        else
        {
            assert(ramsesCamera.isOfType(ramses::ERamsesObjectType_PerspectiveCamera));
            const auto* perspectiveCam = ramses::RamsesUtils::TryConvert<ramses::PerspectiveCamera>(ramsesCamera);
            frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FieldOfView))->m_impl->initializeBindingInputValue(PropertyValue{ perspectiveCam->getVerticalFieldOfView() });
            frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::AspectRatio))->m_impl->initializeBindingInputValue(PropertyValue{ perspectiveCam->getAspectRatio() });
        }
    }
}
