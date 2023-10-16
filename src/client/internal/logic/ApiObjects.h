//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/logic/AnimationTypes.h"
#include "ramses/client/logic/PropertyLink.h"
#include "ramses/framework/DataTypes.h"
#include "ramses/client/logic/ELuaSavingMode.h"
#include "ramses/framework/EFeatureLevel.h"

#include "impl/logic/LuaConfigImpl.h"

#include "internal/logic/LuaCompilationUtils.h"
#include "internal/logic/SolState.h"
#include "internal/logic/LogicNodeDependencies.h"

#include "ramses/framework/ERotationType.h"

#include <vector>
#include <memory>
#include <string_view>

namespace rlogic_serialization
{
    struct ApiObjects;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    class FlatBufferBuilder;
}

namespace ramses
{
    class Node;
    class Appearance;
    class Camera;
    class RenderPass;
    class RenderGroup;
    class UniformInput;
    class MeshNode;

    class LogicObject;
    class LogicNode;
    class LuaScript;
    class LuaInterface;
    class LuaModule;
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
    class TimerNode;
    class AnchorPoint;
}

namespace ramses::internal
{
    class SolState;
    class IRamsesObjectResolver;
    class AnimationNodeConfigImpl;
    class SerializationMap;
    class NodeBindingImpl;
    class CameraBindingImpl;
    class AppearanceBindingImpl;
    class SceneImpl;
    class ValidationReportImpl;

    template <typename T>
    using ApiObjectContainer = std::vector<T*>;
    using ApiObjectOwningContainer = std::vector<std::unique_ptr<LogicObject, std::function<void(LogicObject*)>>>;

    class ApiObjects
    {
    public:
        // Not move-able and non-copyable
        explicit ApiObjects(ramses::EFeatureLevel featureLevel, SceneImpl& scene);
        ~ApiObjects() noexcept;
        // Not move-able because of the dependency between sol objects and their parent sol state
        // Moving those would require a custom move assignment operator which keeps both sol states alive
        // until the objects have been moved, and only then also moves the sol state - we don't need this
        // complexity because we never move-assign ApiObjects
        ApiObjects(ApiObjects&& other) = delete;
        ApiObjects& operator=(ApiObjects&& other) = delete;
        ApiObjects(const ApiObjects& other) = delete;
        ApiObjects& operator=(const ApiObjects& other) = delete;

        // Serialization/Deserialization
        static flatbuffers::Offset<rlogic_serialization::ApiObjects> Serialize(
            const ApiObjects& apiObjects,
            flatbuffers::FlatBufferBuilder& builder,
            ELuaSavingMode luaSavingMode);
        static std::unique_ptr<ApiObjects> Deserialize(
            SceneImpl& scene,
            const rlogic_serialization::ApiObjects& apiObjects,
            const IRamsesObjectResolver& ramsesResolver,
            const std::string& dataSourceDescription,
            ErrorReporting& errorReporting,
            ramses::EFeatureLevel featureLevel);

        // Create/destroy API objects
        LuaScript* createLuaScript(
            std::string_view source,
            const LuaConfigImpl& config,
            std::string_view scriptName,
            ErrorReporting& errorReporting);
        LuaInterface* createLuaInterface(
            std::string_view source,
            const LuaConfigImpl& config,
            std::string_view interfaceName,
            ErrorReporting& errorReporting);
        LuaModule* createLuaModule(
            std::string_view source,
            const LuaConfigImpl& config,
            std::string_view moduleName,
            ErrorReporting& errorReporting);
        NodeBinding* createNodeBinding(ramses::Node& ramsesNode, ramses::ERotationType rotationType, std::string_view name);
        AppearanceBinding* createAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name);
        CameraBinding* createCameraBinding(ramses::Camera& ramsesCamera, bool withFrustumPlanes, std::string_view name);
        RenderPassBinding* createRenderPassBinding(ramses::RenderPass& renderPass, std::string_view name);
        RenderGroupBinding* createRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RenderGroupBindingElements& elements, std::string_view name);
        MeshNodeBinding* createMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name);
        SkinBinding* createSkinBinding(
            std::vector<const NodeBindingImpl*> joints,
            const std::vector<matrix44f>& inverseBindMatrices,
            AppearanceBindingImpl& appearanceBinding,
            const ramses::UniformInput& jointMatInput,
            std::string_view name);
        template <typename T>
        DataArray* createDataArray(const std::vector<T>& data, std::string_view name);
        AnimationNode* createAnimationNode(const AnimationNodeConfigImpl& config, std::string_view name);
        TimerNode* createTimerNode(std::string_view name);
        AnchorPoint* createAnchorPoint(NodeBindingImpl& nodeBinding, CameraBindingImpl& cameraBinding, std::string_view name);
        bool destroy(LogicObject& object, ErrorReporting& errorReporting);

        // Invariance checks
        void validateInterfaces(ValidationReportImpl& report) const;
        void validateDanglingNodes(ValidationReportImpl& report) const;

        // Getters
        template <typename T>
        [[nodiscard]] const ApiObjectContainer<T>& getApiObjectContainer() const;
        template <typename T>
        [[nodiscard]] ApiObjectContainer<T>& getApiObjectContainer();
        [[nodiscard]] const ApiObjectOwningContainer& getApiObjectOwningContainer() const;
        [[nodiscard]] const LogicNodeDependencies& getLogicNodeDependencies() const;
        [[nodiscard]] LogicNodeDependencies& getLogicNodeDependencies();

        // Internally used
        [[nodiscard]] bool bindingsDirty() const;

        [[nodiscard]] int getNumElementsInLuaStack() const;

        [[nodiscard]] const std::vector<PropertyLinkConst>& getAllPropertyLinks() const;
        [[nodiscard]] const std::vector<PropertyLink>& getAllPropertyLinks();

    private:
        template <typename T, typename ImplT>
        T& createAndRegisterObject(std::unique_ptr<ImplT> impl);
        template <typename T>
        [[nodiscard]] bool destroyAndUnregisterObject(T& objToDelete, ErrorReporting& errorReporting);

        // Type-specific destruction logic
        [[nodiscard]] bool destroyInternal(NodeBinding& nodeBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(LuaModule& luaModule, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(AppearanceBinding& appearanceBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(CameraBinding& cameraBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(DataArray& dataArray, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(AnchorPoint& node, ErrorReporting& errorReporting);

        [[nodiscard]] bool checkLuaModules(const ModuleMapping& moduleMapping, ErrorReporting& errorReporting);
        [[nodiscard]] std::vector<PropertyLink> collectPropertyLinks() const;

        std::unique_ptr<SolState> m_solState {std::make_unique<SolState>()};

        ApiObjectContainer<LuaScript>                m_scripts;
        ApiObjectContainer<LuaInterface>             m_interfaces;
        ApiObjectContainer<LuaModule>                m_luaModules;
        ApiObjectContainer<NodeBinding>              m_nodeBindings;
        ApiObjectContainer<AppearanceBinding>        m_appearanceBindings;
        ApiObjectContainer<CameraBinding>            m_cameraBindings;
        ApiObjectContainer<RenderPassBinding>        m_renderPassBindings;
        ApiObjectContainer<RenderGroupBinding>       m_renderGroupBindings;
        ApiObjectContainer<MeshNodeBinding>          m_meshNodeBindings;
        ApiObjectContainer<SkinBinding>              m_skinBindings;
        ApiObjectContainer<DataArray>                m_dataArrays;
        ApiObjectContainer<AnimationNode>            m_animationNodes;
        ApiObjectContainer<TimerNode>                m_timerNodes;
        ApiObjectContainer<AnchorPoint>              m_anchorPoints;
        ApiObjectContainer<LogicObject>              m_logicObjects;
        ApiObjectOwningContainer                     m_objectsOwningContainer;

        LogicNodeDependencies                        m_logicNodeDependencies;

        // persistent storage for links to be given out via public API getPropertyLinks()
        mutable std::vector<PropertyLink> m_collectedLinks;
        mutable std::vector<PropertyLinkConst> m_collectedLinksConst;

        ramses::EFeatureLevel m_featureLevel;
        SceneImpl& m_scene;
    };
}
