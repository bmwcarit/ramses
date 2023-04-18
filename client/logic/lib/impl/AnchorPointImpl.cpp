//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/AnchorPointImpl.h"

#include "ramses-client-api/Node.h"
#include "ramses-client-api/Camera.h"

#include "ramses-logic/Property.h"

#include "impl/PropertyImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "impl/RamsesCameraBindingImpl.h"

#include "internals/ErrorReporting.h"

#include "generated/AnchorPointGen.h"
#include "glm/gtc/type_ptr.hpp"

namespace rlogic::internal
{
    AnchorPointImpl::AnchorPointImpl(RamsesNodeBindingImpl& nodeBinding, RamsesCameraBindingImpl& cameraBinding, std::string_view name, uint64_t id)
        : LogicNodeImpl{ name, id }
        , m_nodeBinding{ nodeBinding }
        , m_cameraBinding{ cameraBinding }
    {
    }

    void AnchorPointImpl::createRootProperties()
    {
        auto outputsType = MakeStruct("", {
                TypeData{"viewportCoords", EPropertyType::Vec2f},
                TypeData{"depth", EPropertyType::Float},
            });
        auto outputs = std::make_unique<Property>(std::make_unique<PropertyImpl>(std::move(outputsType), EPropertySemantics::ScriptOutput));

        setRootProperties({}, std::move(outputs));
    }

    flatbuffers::Offset<rlogic_serialization::AnchorPoint> AnchorPointImpl::Serialize(
        const AnchorPointImpl& anchorPoint,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        const auto fbLogicObject = LogicObjectImpl::Serialize(anchorPoint, builder);
        const auto fbOutputs = PropertyImpl::Serialize(*anchorPoint.getOutputs()->m_impl, builder, serializationMap);
        auto fbAnchorPoint = rlogic_serialization::CreateAnchorPoint(builder,
            fbLogicObject,
            anchorPoint.m_nodeBinding.getId(),
            anchorPoint.m_cameraBinding.getId(),
            0, // no inputs
            fbOutputs);

        builder.Finish(fbAnchorPoint);

        return fbAnchorPoint;
    }

    std::unique_ptr<AnchorPointImpl> AnchorPointImpl::Deserialize(
        const rlogic_serialization::AnchorPoint& anchorPoint,
        ErrorReporting& errorReporting,
        DeserializationMap& deserializationMap)
    {
        std::string name;
        uint64_t id = 0u;
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(anchorPoint.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.add("Fatal error during loading of AnchorPoint from serialized data: missing name and/or ID!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!anchorPoint.rootOutput())
        {
            errorReporting.add("Fatal error during loading of AnchorPoint from serialized data: missing root output!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootOutput = PropertyImpl::Deserialize(*anchorPoint.rootOutput(), EPropertySemantics::ScriptOutput, errorReporting, deserializationMap);
        if (!deserializedRootOutput)
            return nullptr;

        if (deserializedRootOutput->getType() != EPropertyType::Struct)
        {
            errorReporting.add("Fatal error during loading of AnchorPoint from serialized data: root output has unexpected type!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (deserializedRootOutput->getChildCount() != 2u ||
            !deserializedRootOutput->getChild("viewportCoords") || deserializedRootOutput->getChild("viewportCoords")->getType() != EPropertyType::Vec2f ||
            !deserializedRootOutput->getChild("depth") || deserializedRootOutput->getChild("depth")->getType() != EPropertyType::Float)
        {
            errorReporting.add("Fatal error during loading of AnchorPoint: missing or invalid properties!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto* nodeBinding = deserializationMap.resolveLogicObject<RamsesNodeBindingImpl>(anchorPoint.nodeBindingId());
        auto* cameraBinding = deserializationMap.resolveLogicObject<RamsesCameraBindingImpl>(anchorPoint.cameraBindingId());
        if (!nodeBinding || !cameraBinding)
        {
            errorReporting.add("Fatal error during loading of AnchorPoint: could not resolve NodeBinding and/or CameraBinding!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        auto binding = std::make_unique<AnchorPointImpl>(*nodeBinding, *cameraBinding, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootProperties({}, std::make_unique<Property>(std::move(deserializedRootOutput)));

        return binding;
    }

    std::optional<LogicNodeRuntimeError> AnchorPointImpl::update()
    {
        matrix44f projectionMatrix;
        matrix44f cameraViewMatrix;
        matrix44f modelMatrix;

        const auto& ramsesCam = m_cameraBinding.getRamsesCamera();
        if (ramsesCam.getProjectionMatrix(projectionMatrix) != ramses::StatusOK)
            return LogicNodeRuntimeError{"Failed to retrieve projection matrix from Ramses camera!"};

        if (ramsesCam.getInverseModelMatrix(cameraViewMatrix) != ramses::StatusOK)
            return LogicNodeRuntimeError{ "Failed to retrieve view matrix from Ramses camera!" };

        if (m_nodeBinding.getRamsesNode().getModelMatrix(modelMatrix) != ramses::StatusOK)
            return LogicNodeRuntimeError{ "Failed to retrieve model matrix from Ramses node!" };

        const vec4f localOrigin{ 0, 0, 0, 1 };
        const vec4f pointInClipSpace = projectionMatrix * cameraViewMatrix * modelMatrix * localOrigin;
        const vec4f pointInNDS = pointInClipSpace / pointInClipSpace.w;
        const vec4f pointNormalized = (pointInNDS + 1.f) / 2.f;
        const vec4f pointViewport = pointNormalized * vec4f{ float(ramsesCam.getViewportWidth()), float(ramsesCam.getViewportHeight()), 1.f, 1.f };

        getOutputs()->getChild(0u)->m_impl->setValue(vec2f{ pointViewport.x, pointViewport.y });
        getOutputs()->getChild(1u)->m_impl->setValue(pointViewport.z);

        return std::nullopt;
    }

    RamsesNodeBindingImpl& AnchorPointImpl::getRamsesNodeBinding()
    {
        return m_nodeBinding;
    }

    RamsesCameraBindingImpl& AnchorPointImpl::getRamsesCameraBinding()
    {
        return m_cameraBinding;
    }
}
