//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/AnimationTypes.h"
#include "ramses-logic/ERotationType.h"
#include "ramses-logic/EFeatureLevel.h"
#include "ramses-logic/PropertyLink.h"
#include "ramses-logic/DataTypes.h"
#include "ramses-logic/ELuaSavingMode.h"

#include "impl/LuaConfigImpl.h"

#include "internals/LuaCompilationUtils.h"
#include "internals/SolState.h"
#include "internals/LogicNodeDependencies.h"

#include <vector>
#include <memory>
#include <string_view>

namespace ramses
{
    class Scene;
    class Node;
    class Appearance;
    class Camera;
    class RenderPass;
    class RenderGroup;
    class UniformInput;
    class MeshNode;
}

namespace rlogic_serialization
{
    struct ApiObjects;
}

namespace flatbuffers
{
    template<typename T> struct Offset;
    class FlatBufferBuilder;
}

namespace rlogic
{
    class LogicObject;
    class LogicNode;
    class LuaScript;
    class LuaInterface;
    class LuaModule;
    class RamsesNodeBinding;
    class RamsesAppearanceBinding;
    class RamsesCameraBinding;
    class RamsesRenderPassBinding;
    class RamsesRenderGroupBinding;
    class RamsesRenderGroupBindingElements;
    class RamsesMeshNodeBinding;
    class SkinBinding;
    class DataArray;
    class AnimationNode;
    class TimerNode;
    class AnchorPoint;
}

namespace rlogic::internal
{
    class SolState;
    class IRamsesObjectResolver;
    class AnimationNodeConfigImpl;
    class ValidationResults;
    class SerializationMap;
    class RamsesNodeBindingImpl;
    class RamsesCameraBindingImpl;
    class RamsesAppearanceBindingImpl;

    template <typename T>
    using ApiObjectContainer = std::vector<T*>;
    using ApiObjectOwningContainer = std::vector<std::unique_ptr<LogicObject>>;

    class ApiObjects
    {
    public:
        // Not move-able and non-copyable
        explicit ApiObjects(EFeatureLevel featureLevel);
        ~ApiObjects() noexcept;
        // Not move-able because of the dependency between sol objects and their parent sol state
        // Moving those would require a custom move assignment operator which keeps both sol states alive
        // until the objects have been moved, and only then also moves the sol state - we don't need this
        // complexity because we never move-assign ApiObjects
        ApiObjects(ApiObjects&& other) = delete;
        ApiObjects& operator=(ApiObjects&& other) = delete;
        ApiObjects(const ApiObjects& other) = delete;
        ApiObjects& operator=(const ApiObjects& other) = delete;

        [[nodiscard]] size_t getTotalSerializedSize() const;

        template<typename T>
        [[nodiscard]] size_t getSerializedSize() const;

        // Serialization/Deserialization
        static flatbuffers::Offset<rlogic_serialization::ApiObjects> Serialize(
            const ApiObjects& apiObjects,
            flatbuffers::FlatBufferBuilder& builder,
            ELuaSavingMode luaSavingMode);
        static std::unique_ptr<ApiObjects> Deserialize(
            const rlogic_serialization::ApiObjects& apiObjects,
            const IRamsesObjectResolver* ramsesResolver,
            const std::string& dataSourceDescription,
            ErrorReporting& errorReporting,
            EFeatureLevel featureLevel);

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
            ErrorReporting& errorReporting,
            bool verifyModules);
        LuaModule* createLuaModule(
            std::string_view source,
            const LuaConfigImpl& config,
            std::string_view moduleName,
            ErrorReporting& errorReporting);
        RamsesNodeBinding* createRamsesNodeBinding(ramses::Node& ramsesNode, ERotationType rotationType, std::string_view name);
        RamsesAppearanceBinding* createRamsesAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name);
        RamsesCameraBinding* createRamsesCameraBinding(ramses::Camera& ramsesCamera, bool withFrustumPlanes, std::string_view name);
        RamsesRenderPassBinding* createRamsesRenderPassBinding(ramses::RenderPass& renderPass, std::string_view name);
        RamsesRenderGroupBinding* createRamsesRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RamsesRenderGroupBindingElements& elements, std::string_view name);
        RamsesMeshNodeBinding* createRamsesMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name);
        SkinBinding* createSkinBinding(
            std::vector<const RamsesNodeBindingImpl*> joints,
            const std::vector<matrix44f>& inverseBindMatrices,
            RamsesAppearanceBindingImpl& appearanceBinding,
            const ramses::UniformInput& jointMatInput,
            std::string_view name);
        template <typename T>
        DataArray* createDataArray(const std::vector<T>& data, std::string_view name);
        AnimationNode* createAnimationNode(const AnimationNodeConfigImpl& config, std::string_view name);
        TimerNode* createTimerNode(std::string_view name);
        AnchorPoint* createAnchorPoint(RamsesNodeBindingImpl& nodeBinding, RamsesCameraBindingImpl& cameraBinding, std::string_view name);
        bool destroy(LogicObject& object, ErrorReporting& errorReporting);

        // Invariance checks
        [[nodiscard]] bool checkBindingsReferToSameRamsesScene(ErrorReporting& errorReporting) const;
        void validateInterfaces(ValidationResults& validationResults) const;
        void validateDanglingNodes(ValidationResults& validationResults) const;

        // Getters
        template <typename T>
        [[nodiscard]] const ApiObjectContainer<T>& getApiObjectContainer() const;
        template <typename T>
        [[nodiscard]] ApiObjectContainer<T>& getApiObjectContainer();
        [[nodiscard]] const ApiObjectOwningContainer& getApiObjectOwningContainer() const;
        [[nodiscard]] const LogicNodeDependencies& getLogicNodeDependencies() const;
        [[nodiscard]] LogicNodeDependencies& getLogicNodeDependencies();

        [[nodiscard]] LogicNode* getApiObject(LogicNodeImpl& impl) const;
        [[nodiscard]] LogicObject* getApiObjectById(uint64_t id) const;

        // Internally used
        [[nodiscard]] bool bindingsDirty() const;
        [[nodiscard]] uint64_t getNextLogicObjectId();

        // Strictly for testing purposes (inverse mappings require extra attention and test coverage)
        [[nodiscard]] const std::unordered_map<LogicNodeImpl*, LogicNode*>& getReverseImplMapping() const;

        [[nodiscard]] int getNumElementsInLuaStack() const;

        [[nodiscard]] const std::vector<PropertyLink>& getAllPropertyLinks() const;

    private:
        // Handle internal data structures and mappings
        void registerLogicNode(LogicNode& logicNode);
        void unregisterLogicNode(LogicNode& logicNode);
        void registerLogicObject(std::unique_ptr<LogicObject> obj);
        void unregisterLogicObject(LogicObject& objToDelete);

        bool checkLuaModules(
            const ModuleMapping& moduleMapping,
            ErrorReporting& errorReporting);

        // Type-specific destruction logic
        [[nodiscard]] bool destroyInternal(RamsesNodeBinding& ramsesNodeBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(LuaScript& luaScript, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(LuaInterface& luaInterface, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(LuaModule& luaModule, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(RamsesAppearanceBinding& ramsesAppearanceBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(RamsesCameraBinding& ramsesCameraBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(RamsesRenderPassBinding& ramsesRenderPassBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(RamsesRenderGroupBinding& ramsesRenderGroupBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(RamsesMeshNodeBinding& ramsesMeshNodeBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(SkinBinding& skinBinding, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(AnimationNode& node, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(DataArray& dataArray, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(TimerNode& node, ErrorReporting& errorReporting);
        [[nodiscard]] bool destroyInternal(AnchorPoint& node, ErrorReporting& errorReporting);

        std::vector<PropertyLink> collectPropertyLinks() const;

        std::unique_ptr<SolState> m_solState {std::make_unique<SolState>()};

        ApiObjectContainer<LuaScript>                m_scripts;
        ApiObjectContainer<LuaInterface>             m_interfaces;
        ApiObjectContainer<LuaModule>                m_luaModules;
        ApiObjectContainer<RamsesNodeBinding>        m_ramsesNodeBindings;
        ApiObjectContainer<RamsesAppearanceBinding>  m_ramsesAppearanceBindings;
        ApiObjectContainer<RamsesCameraBinding>      m_ramsesCameraBindings;
        ApiObjectContainer<RamsesRenderPassBinding>  m_ramsesRenderPassBindings;
        ApiObjectContainer<RamsesRenderGroupBinding> m_ramsesRenderGroupBindings;
        ApiObjectContainer<RamsesMeshNodeBinding>    m_ramsesMeshNodeBindings;
        ApiObjectContainer<SkinBinding>              m_skinBindings;
        ApiObjectContainer<DataArray>                m_dataArrays;
        ApiObjectContainer<AnimationNode>            m_animationNodes;
        ApiObjectContainer<TimerNode>                m_timerNodes;
        ApiObjectContainer<AnchorPoint>              m_anchorPoints;
        ApiObjectContainer<LogicObject>              m_logicObjects;
        ApiObjectOwningContainer                     m_objectsOwningContainer;

        LogicNodeDependencies                        m_logicNodeDependencies;
        uint64_t                                     m_lastObjectId = 0;

        std::unordered_map<LogicNodeImpl*, LogicNode*> m_reverseImplMapping;
        std::unordered_map<uint64_t, LogicObject*>     m_logicObjectIdMapping;

        // persistent storage for links to be given out via public API getPropertyLinks()
        mutable std::vector<PropertyLink> m_collectedLinks;

        EFeatureLevel m_featureLevel;
    };
}
