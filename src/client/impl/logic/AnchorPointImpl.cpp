//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/AnchorPointImpl.h"

#include "ramses/client/Node.h"
#include "ramses/client/Camera.h"

#include "ramses/client/logic/Property.h"

#include "impl/logic/PropertyImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/CameraBindingImpl.h"

#include "impl/ErrorReporting.h"

#include "internal/logic/flatbuffers/generated/AnchorPointGen.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses::internal
{
    AnchorPointImpl::AnchorPointImpl(SceneImpl& scene, NodeBindingImpl& nodeBinding, CameraBindingImpl& cameraBinding, std::string_view name, sceneObjectId_t id)
        : LogicNodeImpl{ scene, name, id }
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
        auto outputs = std::make_unique<PropertyImpl>(std::move(outputsType), EPropertySemantics::ScriptOutput);

        setRootProperties({}, std::move(outputs));
    }

    flatbuffers::Offset<rlogic_serialization::AnchorPoint> AnchorPointImpl::Serialize(
        const AnchorPointImpl& anchorPoint,
        flatbuffers::FlatBufferBuilder& builder,
        SerializationMap& serializationMap)
    {
        const auto fbLogicObject = LogicObjectImpl::Serialize(anchorPoint, builder);
        const auto fbOutputs = PropertyImpl::Serialize(anchorPoint.getOutputs()->impl(), builder, serializationMap);
        auto fbAnchorPoint = rlogic_serialization::CreateAnchorPoint(builder,
            fbLogicObject,
            anchorPoint.m_nodeBinding.getSceneObjectId().getValue(),
            anchorPoint.m_cameraBinding.getSceneObjectId().getValue(),
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
        sceneObjectId_t id{};
        uint64_t userIdHigh = 0u;
        uint64_t userIdLow = 0u;
        if (!LogicObjectImpl::Deserialize(anchorPoint.base(), name, id, userIdHigh, userIdLow, errorReporting))
        {
            errorReporting.set("Fatal error during loading of AnchorPoint from serialized data: missing name and/or ID!", nullptr);
            return nullptr;
        }

        if (!anchorPoint.rootOutput())
        {
            errorReporting.set("Fatal error during loading of AnchorPoint from serialized data: missing root output!", nullptr);
            return nullptr;
        }

        std::unique_ptr<PropertyImpl> deserializedRootOutput = PropertyImpl::Deserialize(*anchorPoint.rootOutput(), EPropertySemantics::ScriptOutput, errorReporting, deserializationMap);
        if (!deserializedRootOutput)
            return nullptr;

        if (deserializedRootOutput->getType() != EPropertyType::Struct)
        {
            errorReporting.set("Fatal error during loading of AnchorPoint from serialized data: root output has unexpected type!", nullptr);
            return nullptr;
        }

        if (deserializedRootOutput->getChildCount() != 2u ||
            !deserializedRootOutput->getChild("viewportCoords") || deserializedRootOutput->getChild("viewportCoords")->getType() != EPropertyType::Vec2f ||
            !deserializedRootOutput->getChild("depth") || deserializedRootOutput->getChild("depth")->getType() != EPropertyType::Float)
        {
            errorReporting.set("Fatal error during loading of AnchorPoint: missing or invalid properties!", nullptr);
            return nullptr;
        }

        auto* nodeBinding = deserializationMap.resolveLogicObject<NodeBindingImpl>(sceneObjectId_t{ anchorPoint.nodeBindingId() });
        auto* cameraBinding = deserializationMap.resolveLogicObject<CameraBindingImpl>(sceneObjectId_t{ anchorPoint.cameraBindingId() });
        if (!nodeBinding || !cameraBinding)
        {
            errorReporting.set("Fatal error during loading of AnchorPoint: could not resolve NodeBinding and/or CameraBinding!", nullptr);
            return nullptr;
        }

        auto binding = std::make_unique<AnchorPointImpl>(deserializationMap.getScene(), *nodeBinding, *cameraBinding, name, id);
        binding->setUserId(userIdHigh, userIdLow);
        binding->setRootProperties({}, std::move(deserializedRootOutput));

        return binding;
    }

    std::optional<LogicNodeRuntimeError> AnchorPointImpl::update()
    {
        matrix44f projectionMatrix;
        matrix44f cameraViewMatrix;
        matrix44f modelMatrix;

        const auto& ramsesCam = m_cameraBinding.getRamsesCamera();
        if (!ramsesCam.getProjectionMatrix(projectionMatrix))
            return LogicNodeRuntimeError{"Failed to retrieve projection matrix from Ramses camera!"};

        if (!ramsesCam.getInverseModelMatrix(cameraViewMatrix))
            return LogicNodeRuntimeError{ "Failed to retrieve view matrix from Ramses camera!" };

        if (!m_nodeBinding.getRamsesNode().getModelMatrix(modelMatrix))
            return LogicNodeRuntimeError{ "Failed to retrieve model matrix from Ramses node!" };

        const vec4f localOrigin{ 0, 0, 0, 1 };
        const vec4f pointInClipSpace = projectionMatrix * cameraViewMatrix * modelMatrix * localOrigin;
        const vec4f pointInNDS = pointInClipSpace / pointInClipSpace.w; // NOLINT(cppcoreguidelines-pro-type-union-access)
        const vec4f pointNormalized = (pointInNDS + 1.f) / 2.f;
        const vec4f pointViewport = pointNormalized * vec4f{ float(ramsesCam.getViewportWidth()), float(ramsesCam.getViewportHeight()), 1.f, 1.f };

        getOutputs()->getChild(0u)->impl().setValue(vec2f{ pointViewport.x, pointViewport.y }); // NOLINT(cppcoreguidelines-pro-type-union-access)
        getOutputs()->getChild(1u)->impl().setValue(pointViewport.z); // NOLINT(cppcoreguidelines-pro-type-union-access)

        return std::nullopt;
    }

    NodeBindingImpl& AnchorPointImpl::getNodeBinding()
    {
        return m_nodeBinding;
    }

    CameraBindingImpl& AnchorPointImpl::getCameraBinding()
    {
        return m_cameraBinding;
    }
}
