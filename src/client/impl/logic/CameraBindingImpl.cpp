//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/CameraBindingImpl.h"

#include "ramses/client/ramses-utils.h"
#include "ramses/client/Camera.h"
#include "ramses/client/PerspectiveCamera.h"

#include "ramses/client/logic/EPropertyType.h"
#include "ramses/client/logic/Property.h"

#include "impl/logic/PropertyImpl.h"
#include "fmt/format.h"

#include "internal/logic/RamsesHelper.h"
#include "impl/ErrorReporting.h"
#include "internal/logic/RamsesObjectResolver.h"

#include "internal/logic/flatbuffers/generated/CameraBindingGen.h"

namespace ramses::internal
{
    CameraBindingImpl::CameraBindingImpl(SceneImpl& scene, ramses::Camera& ramsesCamera, bool withFrustumPlanes, std::string_view name, sceneObjectId_t id)
        : RamsesBindingImpl{ scene, name, id, ramsesCamera }
        , m_ramsesCamera(ramsesCamera)
        , m_hasFrustumPlanesProperties{ ramsesCamera.isOfType(ramses::ERamsesObjectType::OrthographicCamera) || withFrustumPlanes }
    {
    }

    void CameraBindingImpl::createRootProperties()
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

        setRootInputs(std::make_unique<PropertyImpl>(cameraBindingInputs, EPropertySemantics::BindingInput));

        ApplyRamsesValuesToInputProperties(*this);
    }

    flatbuffers::Offset<rlogic_serialization::CameraBinding> CameraBindingImpl::Serialize(
        const CameraBindingImpl& binding,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        auto ramsesReference = RamsesBindingImpl::SerializeRamsesReference(binding.m_ramsesCamera, builder);

        const auto logicObject = LogicObjectImpl::Serialize(binding, builder);
        const auto propertyObject = PropertyImpl::Serialize(binding.getInputs()->impl(), builder, serializationMap);
        auto ramsesBinding = rlogic_serialization::CreateRamsesBinding(builder,
            logicObject,
            ramsesReference,
            propertyObject);
        builder.Finish(ramsesBinding);

        auto cameraBinding = rlogic_serialization::CreateCameraBinding(builder, ramsesBinding);
        builder.Finish(cameraBinding);

        return cameraBinding;
    }

    std::unique_ptr<CameraBindingImpl> CameraBindingImpl::Deserialize(
        const rlogic_serialization::CameraBinding& cameraBinding,
        const IRamsesObjectResolver& ramsesResolver,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        if (!cameraBinding.base())
        {
            errorReporting.set("Fatal error during loading of CameraBinding from serialized data: missing base class info!", nullptr);
            return nullptr;
        }

        std::string name;
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(cameraBinding.base()->base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of CameraBinding from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!cameraBinding.base()->rootInput())
        {
            errorReporting.set("Fatal error during loading of CameraBinding from serialized data: missing root input!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootInput = PropertyImpl::Deserialize(*cameraBinding.base()->rootInput(), EPropertySemantics::BindingInput, errorReporting, deserializationMap);
        if (!deserializedRootInput)
            return nullptr;

        if (deserializedRootInput->getType() != EPropertyType::Struct)
        {
            errorReporting.set("Fatal error during loading of CameraBinding from serialized data: root input has unexpected type!", nullptr);
            return nullptr;
        }

        const auto frustumInputProp = deserializedRootInput->getChild("frustum");
        if (!frustumInputProp || !(frustumInputProp->getChildCount() == 4u || frustumInputProp->getChildCount() == 6u))
        {
            errorReporting.set("Fatal error during loading of CameraBinding from serialized data: missing or invalid input properties!", nullptr);
            return nullptr;
        }
        const bool hasFrustumPlanesProperties = (frustumInputProp->getChildCount() == 6u);

        const auto* boundObject = cameraBinding.base()->boundRamsesObject();
        if (!boundObject)
        {
            errorReporting.set("Fatal error during loading of CameraBinding from serialized data: no reference to ramses camera!", nullptr);
            return nullptr;
        }

        const ramses::sceneObjectId_t objectId(boundObject->objectId());

        ramses::Camera* resolvedCamera = ramsesResolver.findRamsesCameraInScene(name, objectId);
        if (!resolvedCamera)
            return nullptr;

        if (static_cast<uint32_t>(resolvedCamera->getType()) != boundObject->objectType())
        {
            errorReporting.set("Fatal error during loading of CameraBinding from serialized data: loaded type does not match referenced camera type!", nullptr);
            return nullptr;
        }

        auto binding = std::make_unique<CameraBindingImpl>(deserializationMap.getScene(), *resolvedCamera, hasFrustumPlanesProperties, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootInputs(std::move(deserializedRootInput));

        ApplyRamsesValuesToInputProperties(*binding);

        return binding;
    }

    std::optional<LogicNodeRuntimeError> CameraBindingImpl::update()
    {
        PropertyImpl&    vpProperties = getInputs()->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Viewport))->impl();
        PropertyImpl&    vpOffsetX    = vpProperties.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetX))->impl();
        PropertyImpl&    vpOffsetY    = vpProperties.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetY))->impl();
        PropertyImpl&    vpWidth      = vpProperties.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortWidth))->impl();
        PropertyImpl&    vpHeight     = vpProperties.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortHeight))->impl();
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

            if (!m_ramsesCamera.get().setViewport(vpX, vpY, vpW, vpH))
                return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
        }

        PropertyImpl& frustum = getInputs()->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Frustum))->impl();

        // Index of Perspective Frustum Properties is used, but wouldn't matter as Ortho Camera indeces are the same for these two properties
        PropertyImpl& nearPlane = frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::NearPlane))->impl();
        PropertyImpl& farPlane  = frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FarPlane))->impl();

        if (m_hasFrustumPlanesProperties)
        {
            PropertyImpl& leftPlane   = frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::LeftPlane))->impl();
            PropertyImpl& rightPlane  = frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::RightPlane))->impl();
            PropertyImpl& bottomPlane = frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::BottomPlane))->impl();
            PropertyImpl& topPlane    = frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::TopPlane))->impl();

            if (nearPlane.checkForBindingInputNewValueAndReset()
                || farPlane.checkForBindingInputNewValueAndReset()
                || leftPlane.checkForBindingInputNewValueAndReset()
                || rightPlane.checkForBindingInputNewValueAndReset()
                || bottomPlane.checkForBindingInputNewValueAndReset()
                || topPlane.checkForBindingInputNewValueAndReset())
            {
                if (!m_ramsesCamera.get().setFrustum(
                    leftPlane.getValueAs<float>(),
                    rightPlane.getValueAs<float>(),
                    bottomPlane.getValueAs<float>(),
                    topPlane.getValueAs<float>(),
                    nearPlane.getValueAs<float>(),
                    farPlane.getValueAs<float>()))
                {
                    return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
                }
            }
        }
        else
        {
            PropertyImpl& fov = frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FieldOfView))->impl();
            PropertyImpl& aR = frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::AspectRatio))->impl();

            if (nearPlane.checkForBindingInputNewValueAndReset()
                || farPlane.checkForBindingInputNewValueAndReset()
                || fov.checkForBindingInputNewValueAndReset()
                || aR.checkForBindingInputNewValueAndReset())
            {
                assert(m_ramsesCamera.get().isOfType(ramses::ERamsesObjectType::PerspectiveCamera));
                auto* perspectiveCam = m_ramsesCamera.get().as<PerspectiveCamera>();
                if (!perspectiveCam->setFrustum(fov.getValueAs<float>(), aR.getValueAs<float>(), nearPlane.getValueAs<float>(), farPlane.getValueAs<float>()))
                    return LogicNodeRuntimeError{ getErrorReporting().getError()->message };
            }
        }

        return std::nullopt;
    }

    bool CameraBindingImpl::hasFrustumPlanesProperties() const
    {
        return m_hasFrustumPlanesProperties;
    }

    ramses::Camera& CameraBindingImpl::getRamsesCamera() const
    {
        return m_ramsesCamera;
    }

    void CameraBindingImpl::ApplyRamsesValuesToInputProperties(CameraBindingImpl& binding)
    {
        const auto& ramsesCamera = binding.getRamsesCamera();

        // Initializes input values with values from ramses camera silently (no dirty mechanism triggered)
        PropertyImpl& viewport = binding.getInputs()->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Viewport))->impl();
        viewport.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetX))->impl().initializeBindingInputValue(PropertyValue{ ramsesCamera.getViewportX() });
        viewport.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortOffsetY))->impl().initializeBindingInputValue(PropertyValue{ ramsesCamera.getViewportY() });
        viewport.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortWidth))->impl().initializeBindingInputValue(PropertyValue{ static_cast<int32_t>(ramsesCamera.getViewportWidth()) });
        viewport.getChild(static_cast<size_t>(ECameraViewportPropertyStaticIndex::ViewPortHeight))->impl().initializeBindingInputValue(PropertyValue{ static_cast<int32_t>(ramsesCamera.getViewportHeight()) });

        PropertyImpl& frustum = binding.getInputs()->getChild(static_cast<size_t>(ECameraPropertyStructStaticIndex::Frustum))->impl();
        frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::NearPlane))->impl().initializeBindingInputValue(PropertyValue{ ramsesCamera.getNearPlane() });
        frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::FarPlane))->impl().initializeBindingInputValue(PropertyValue{ ramsesCamera.getFarPlane() });
        if (binding.hasFrustumPlanesProperties())
        {
            frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::LeftPlane))->impl().initializeBindingInputValue(PropertyValue{ ramsesCamera.getLeftPlane() });
            frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::RightPlane))->impl().initializeBindingInputValue(PropertyValue{ ramsesCamera.getRightPlane() });
            frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::BottomPlane))->impl().initializeBindingInputValue(PropertyValue{ ramsesCamera.getBottomPlane() });
            frustum.getChild(static_cast<size_t>(ECameraFrustumPlanesPropertyStaticIndex::TopPlane))->impl().initializeBindingInputValue(PropertyValue{ ramsesCamera.getTopPlane() });
        }
        else
        {
            assert(ramsesCamera.isOfType(ramses::ERamsesObjectType::PerspectiveCamera));
            const auto* perspectiveCam = ramsesCamera.as<PerspectiveCamera>();
            frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::FieldOfView))->impl().initializeBindingInputValue(PropertyValue{ perspectiveCam->getVerticalFieldOfView() });
            frustum.getChild(static_cast<size_t>(EPerspectiveCameraFrustumPropertyStaticIndex::AspectRatio))->impl().initializeBindingInputValue(PropertyValue{ perspectiveCam->getAspectRatio() });
        }
    }
}
