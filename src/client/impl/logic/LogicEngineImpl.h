//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "impl/SceneObjectImpl.h"
#include "ramses/client/logic/AnimationTypes.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LogicEngineReport.h"
#include "ramses/framework/DataTypes.h"
#include "ramses/framework/EFeatureLevel.h"
#include "internal/logic/ApiObjects.h"
#include "internal/logic/LogicNodeDependencies.h"
#include "internal/logic/UpdateReport.h"
#include "internal/logic/LogicNodeUpdateStatistics.h"
#include "internal/logic/ApiObjectsSerializedSize.h"

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/ERotationType.h"

#include <memory>
#include <vector>
#include <string>
#include <string_view>

namespace ramses
{
    class SceneObject;
    class Node;
    class Appearance;
    class Camera;
    class RenderPass;
    class RenderGroup;
    class UniformInput;
    class MeshNode;

    class NodeBinding;
    class AppearanceBinding;
    class CameraBinding;
    class RenderPassBinding;
    class RenderGroupBinding;
    class RenderGroupBindingElements;
    class MeshNodeBinding;
    class SkinBinding;
    class DataArray;
    class AnimationNode;
    class AnimationNodeConfig;
    class TimerNode;
    class AnchorPoint;
    class LuaScript;
    class LuaInterface;
    class LuaModule;
    class LogicNode;
    class Property;
}

namespace ramses::internal
{
    class SceneImpl;
    class LuaConfigImpl;
    class SaveFileConfigImpl;
    class LogicNodeImpl;
    class RamsesBindingImpl;
    class ValidationReportImpl;
    class ApiObjects;

    class LogicEngineImpl : public SceneObjectImpl
    {
    public:
        LogicEngineImpl(SceneImpl& sceneImpl, std::string_view name);

        // Public API
        LuaScript* createLuaScript(std::string_view source, const LuaConfigImpl& config, std::string_view scriptName);
        LuaInterface* createLuaInterface(std::string_view source, const LuaConfigImpl& config, std::string_view interfaceName);
        LuaModule* createLuaModule(std::string_view source, const LuaConfigImpl& config, std::string_view moduleName);
        bool extractLuaDependencies(std::string_view source, const std::function<void(const std::string&)>& callbackFunc);
        NodeBinding* createNodeBinding(ramses::Node& ramsesNode, ramses::ERotationType rotationType, std::string_view name);
        AppearanceBinding* createAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name);
        CameraBinding* createCameraBinding(ramses::Camera& ramsesCamera, std::string_view name);
        CameraBinding* createCameraBindingWithFrustumPlanes(ramses::Camera& ramsesCamera, std::string_view name);
        RenderPassBinding* createRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name);
        RenderGroupBinding* createRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RenderGroupBindingElements& elements, std::string_view name);
        MeshNodeBinding* createMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name);
        SkinBinding* createSkinBinding(
            const std::vector<const NodeBinding*>& joints,
            const std::vector<matrix44f>& inverseBindMatrices,
            AppearanceBinding& appearanceBinding,
            const UniformInput& jointMatInput,
            std::string_view name);
        template <typename T>
        DataArray* createDataArray(const std::vector<T>& data, std::string_view name);
        AnimationNode* createAnimationNode(const AnimationNodeConfig& config, std::string_view name);
        TimerNode* createTimerNode(std::string_view name);
        AnchorPoint* createAnchorPoint(NodeBinding& nodeBinding, CameraBinding& cameraBinding, std::string_view name);

        bool destroy(LogicObject& object);

        bool update();

        void onValidate(ValidationReportImpl& report) const override;

        bool link(Property& sourceProperty, Property& targetProperty);
        bool linkWeak(Property& sourceProperty, Property& targetProperty);
        bool unlink(Property& sourceProperty, Property& targetProperty);

        [[nodiscard]] bool isLinked(const LogicNode& logicNode) const;

        [[nodiscard]] const ApiObjects& getApiObjects() const;
        [[nodiscard]] ApiObjects& getApiObjects();

        // for benchmarking purposes only
        void disableTrackingDirtyNodes();

        void enableUpdateReport(bool enable);
        [[nodiscard]] LogicEngineReport getLastUpdateReport() const;

        void setStatisticsLoggingRate(size_t loggingRate, EStatisticsLogMode mode = EStatisticsLogMode::Compact);

        [[nodiscard]] size_t getTotalSerializedSize(ELuaSavingMode luaSavingMode) const;
        template<typename T>
        [[nodiscard]] size_t getSerializedSize(ELuaSavingMode luaSavingMode) const;

        // RamsesObject implementation
        bool serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const override;
        bool deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext) override;
        bool resolveDeserializationDependencies(DeserializationContext& serializationContext) override;
        void deinitializeFrameworkData() override { /*logic has no internal framework data*/ }

    private:
        bool save(flatbuffers::FlatBufferBuilder& builder, const SaveFileConfigImpl& config);
        size_t activateLinksRecursive(PropertyImpl& output);
        void setNodeToBeAlwaysUpdatedDirty();

        [[nodiscard]] bool updateNodes(const NodeVector& nodes);

        [[nodiscard]] bool loadFromByteData(const void* byteData, size_t byteSize, bool enableMemoryVerification, const std::string& dataSourceDescription);

        EFeatureLevel m_featureLevel;

        std::unique_ptr<ApiObjects> m_apiObjects;
        bool m_nodeDirtyMechanismEnabled = true;

        bool m_updateReportEnabled = false;
        bool m_statisticsEnabled   = true;
        UpdateReport m_updateReport;
        LogicNodeUpdateStatistics m_statistics;
        std::vector<char>         m_byteBuffer;
    };

    template<typename T>
    size_t LogicEngineImpl::getSerializedSize(ELuaSavingMode luaSavingMode) const
    {
        return ApiObjectsSerializedSize::GetSerializedSize<T>(*m_apiObjects, luaSavingMode);
    }
}
