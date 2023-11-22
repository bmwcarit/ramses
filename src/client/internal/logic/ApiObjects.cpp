//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/logic/ApiObjects.h"
#include "impl/ErrorReporting.h"

#include "ramses-sdk-build-config.h"

#include "ramses/client/logic/LogicObject.h"
#include "ramses/client/logic/LogicNode.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationTypes.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/RenderBufferBinding.h"

#include "impl/ValidationReportImpl.h"
#include "impl/logic/PropertyImpl.h"
#include "impl/logic/LuaScriptImpl.h"
#include "impl/logic/LuaInterfaceImpl.h"
#include "impl/logic/LuaModuleImpl.h"
#include "impl/logic/NodeBindingImpl.h"
#include "impl/logic/AppearanceBindingImpl.h"
#include "impl/logic/CameraBindingImpl.h"
#include "impl/logic/RenderPassBindingImpl.h"
#include "impl/logic/RenderGroupBindingImpl.h"
#include "impl/logic/MeshNodeBindingImpl.h"
#include "impl/logic/SkinBindingImpl.h"
#include "impl/logic/DataArrayImpl.h"
#include "impl/logic/AnimationNodeImpl.h"
#include "impl/logic/AnimationNodeConfigImpl.h"
#include "impl/logic/TimerNodeImpl.h"
#include "impl/logic/AnchorPointImpl.h"
#include "impl/logic/RenderBufferBindingImpl.h"

#include "ramses/client/Scene.h"
#include "ramses/client/Node.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Camera.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/MeshNode.h"

#include "internal/logic/flatbuffers/generated/ApiObjectsGen.h"
#include "internal/logic/flatbuffers/generated/AppearanceBindingGen.h"
#include "internal/logic/flatbuffers/generated/RamsesBindingGen.h"
#include "internal/logic/flatbuffers/generated/CameraBindingGen.h"
#include "internal/logic/flatbuffers/generated/NodeBindingGen.h"
#include "internal/logic/flatbuffers/generated/LinkGen.h"
#include "internal/logic/flatbuffers/generated/DataArrayGen.h"
#include "internal/logic/flatbuffers/generated/AnimationNodeGen.h"
#include "internal/logic/flatbuffers/generated/TimerNodeGen.h"
#include "internal/logic/flatbuffers/generated/RenderBufferBindingGen.h"

#include "fmt/format.h"
#include "TypeUtils.h"
#include <deque>

namespace ramses::internal
{
    ApiObjects::ApiObjects(ramses::EFeatureLevel featureLevel, SceneImpl& scene)
        : m_featureLevel{ featureLevel }
        , m_scene{ scene }
    {
        (void)m_featureLevel; // maybe unused if not affecting any internal objects but kept for future levels
    }

    ApiObjects::~ApiObjects() noexcept = default;

    bool ApiObjects::checkLuaModules(const ModuleMapping& moduleMapping, ErrorReporting& errorReporting)
    {
        for (const auto& module : moduleMapping)
        {
            if (m_luaModules.cend() == std::find_if(m_luaModules.cbegin(), m_luaModules.cend(),
                [&module](const auto& m) { return m == module.second; }))
            {
                errorReporting.set(fmt::format("Failed to map Lua module '{}'! It was created on a different instance of LogicEngine.", module.first), module.second);
                return false;
            }
        }

        return true;
    }

    LuaScript* ApiObjects::createLuaScript(
        std::string_view source,
        const LuaConfigImpl& config,
        std::string_view scriptName,
        ErrorReporting& errorReporting)
    {
        const ModuleMapping& modules = config.getModuleMapping();
        if (!checkLuaModules(modules, errorReporting))
            return nullptr;

        std::optional<LuaCompiledScript> compiledScript = LuaCompilationUtils::CompileScriptOrImportPrecompiled(
            *m_solState,
            modules,
            config.getStandardModules(),
            std::string{ source },
            scriptName,
            errorReporting,
            {}, {}, {},
            config.hasDebugLogFunctionsEnabled());

        if (!compiledScript)
            return nullptr;

        auto impl = std::make_unique<LuaScriptImpl>(m_scene, std::move(*compiledScript), scriptName, sceneObjectId_t{});
        impl->createRootProperties();

        return &createAndRegisterObject<LuaScript, LuaScriptImpl>(std::move(impl));
    }

    LuaInterface* ApiObjects::createLuaInterface(
        std::string_view source,
        const LuaConfigImpl& config,
        std::string_view interfaceName,
        ErrorReporting& errorReporting)
    {
        if (interfaceName.empty())
        {
            errorReporting.set("Can't create interface with empty name!", nullptr);
            return nullptr;
        }

        const ModuleMapping& modules = config.getModuleMapping();
        if (!checkLuaModules(modules, errorReporting))
            return nullptr;

        std::optional<LuaCompiledInterface> compiledInterface = LuaCompilationUtils::CompileInterface(
            *m_solState,
            modules,
            config.getStandardModules(),
            std::string{ source },
            interfaceName,
            errorReporting);

        if (!compiledInterface)
            return nullptr;

        auto impl = std::make_unique<LuaInterfaceImpl>(m_scene, std::move(*compiledInterface), interfaceName, sceneObjectId_t{});
        impl->createRootProperties();

        return &createAndRegisterObject<LuaInterface, LuaInterfaceImpl>(std::move(impl));
    }

    LuaModule* ApiObjects::createLuaModule(
        std::string_view source,
        const LuaConfigImpl& config,
        std::string_view moduleName,
        ErrorReporting& errorReporting)
    {
        const ModuleMapping& modules = config.getModuleMapping();
        if (!checkLuaModules(modules, errorReporting))
            return nullptr;

        std::optional<LuaCompiledModule> compiledModule = LuaCompilationUtils::CompileModuleOrImportPrecompiled(
            *m_solState,
            modules,
            config.getStandardModules(),
            std::string{source},
            moduleName,
            errorReporting,
            {},
            config.hasDebugLogFunctionsEnabled());

        if (!compiledModule)
            return nullptr;

        auto impl = std::make_unique<LuaModuleImpl>(m_scene, std::move(*compiledModule), moduleName, sceneObjectId_t{});
        return &createAndRegisterObject<LuaModule, LuaModuleImpl>(std::move(impl));
    }

    NodeBinding* ApiObjects::createNodeBinding(ramses::Node& ramsesNode, ramses::ERotationType rotationType, std::string_view name)
    {
        auto impl = std::make_unique<NodeBindingImpl>(m_scene, ramsesNode, rotationType, name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<NodeBinding, NodeBindingImpl>(std::move(impl));
    }

    AppearanceBinding* ApiObjects::createAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name)
    {
        auto impl = std::make_unique<AppearanceBindingImpl>(m_scene, ramsesAppearance, name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<AppearanceBinding, AppearanceBindingImpl>(std::move(impl));
    }

    CameraBinding* ApiObjects::createCameraBinding(ramses::Camera& ramsesCamera, bool withFrustumPlanes, std::string_view name)
    {
        auto impl = std::make_unique<CameraBindingImpl>(m_scene, ramsesCamera, withFrustumPlanes, name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<CameraBinding, CameraBindingImpl>(std::move(impl));
    }

    RenderPassBinding* ApiObjects::createRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name)
    {
        auto impl = std::make_unique<RenderPassBindingImpl>(m_scene, ramsesRenderPass, name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<RenderPassBinding, RenderPassBindingImpl>(std::move(impl));
    }

    RenderGroupBinding* ApiObjects::createRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RenderGroupBindingElements& elements, std::string_view name)
    {
        auto impl = std::make_unique<RenderGroupBindingImpl>(m_scene, ramsesRenderGroup, elements.impl(), name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<RenderGroupBinding, RenderGroupBindingImpl>(std::move(impl));
    }

    MeshNodeBinding* ApiObjects::createMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name)
    {
        auto impl = std::make_unique<MeshNodeBindingImpl>(m_scene, ramsesMeshNode, name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<MeshNodeBinding, MeshNodeBindingImpl>(std::move(impl));
    }

    SkinBinding* ApiObjects::createSkinBinding(
        std::vector<const NodeBindingImpl*> joints,
        const std::vector<matrix44f>& inverseBindMatrices,
        AppearanceBindingImpl& appearanceBinding,
        const ramses::UniformInput& jointMatInput,
        std::string_view name)
    {
        auto impl = std::make_unique<SkinBindingImpl>(m_scene, std::move(joints), inverseBindMatrices, appearanceBinding, jointMatInput, name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<SkinBinding, SkinBindingImpl>(std::move(impl));
    }

    template <typename T>
    DataArray* ApiObjects::createDataArray(const std::vector<T>& data, std::string_view name)
    {
        static_assert(CanPropertyTypeBeStoredInDataArray(PropertyTypeToEnum<T>::TYPE));
        // make copy of users data and move into data array
        std::vector<T> dataCopy = data;
        auto impl = std::make_unique<DataArrayImpl>(m_scene, std::move(dataCopy), name, sceneObjectId_t{});
        return &createAndRegisterObject<DataArray, DataArrayImpl>(std::move(impl));
    }

    AnimationNode* ApiObjects::createAnimationNode(const AnimationNodeConfigImpl& config, std::string_view name)
    {
        auto impl = std::make_unique<AnimationNodeImpl>(m_scene, config.getChannels(), config.getExposingOfChannelDataAsProperties(), name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<AnimationNode, AnimationNodeImpl>(std::move(impl));
    }

    TimerNode* ApiObjects::createTimerNode(std::string_view name)
    {
        auto impl = std::make_unique<TimerNodeImpl>(m_scene, name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<TimerNode, TimerNodeImpl>(std::move(impl));
    }

    AnchorPoint* ApiObjects::createAnchorPoint(NodeBindingImpl& nodeBinding, CameraBindingImpl& cameraBinding, std::string_view name)
    {
        auto impl = std::make_unique<AnchorPointImpl>(m_scene, nodeBinding, cameraBinding, name, sceneObjectId_t{});
        impl->createRootProperties();
        auto& anchor = createAndRegisterObject<AnchorPoint, AnchorPointImpl>(std::move(impl));

        m_logicNodeDependencies.addBindingDependency(nodeBinding, anchor.m_impl);
        m_logicNodeDependencies.addBindingDependency(cameraBinding, anchor.m_impl);

        return &anchor;
    }

    RenderBufferBinding* ApiObjects::createRenderBufferBinding(ramses::RenderBuffer& renderBuffer, std::string_view name)
    {
        auto impl = std::make_unique<RenderBufferBindingImpl>(m_scene, renderBuffer, name, sceneObjectId_t{});
        impl->createRootProperties();
        return &createAndRegisterObject<RenderBufferBinding, RenderBufferBindingImpl>(std::move(impl));
    }

    bool ApiObjects::destroy(LogicObject& object, ErrorReporting& errorReporting)
    {
        auto luaScript = dynamic_cast<LuaScript*>(&object);
        if (luaScript)
            return destroyAndUnregisterObject(*luaScript, errorReporting);

        auto luaInterface = dynamic_cast<LuaInterface*>(&object);
        if (luaInterface)
            return destroyAndUnregisterObject(*luaInterface, errorReporting);

        auto luaModule = dynamic_cast<LuaModule*>(&object);
        if (luaModule)
            return destroyInternal(*luaModule, errorReporting);

        auto nodeBinding = dynamic_cast<NodeBinding*>(&object);
        if (nodeBinding)
            return destroyInternal(*nodeBinding, errorReporting);

        auto appearanceBinding = dynamic_cast<AppearanceBinding*>(&object);
        if (appearanceBinding)
            return destroyInternal(*appearanceBinding, errorReporting);

        auto cameraBinding = dynamic_cast<CameraBinding*>(&object);
        if (cameraBinding)
            return destroyInternal(*cameraBinding, errorReporting);

        auto renderPassBinding = dynamic_cast<RenderPassBinding*>(&object);
        if (renderPassBinding)
            return destroyAndUnregisterObject(*renderPassBinding, errorReporting);

        auto renderGroupBinding = dynamic_cast<RenderGroupBinding*>(&object);
        if (renderGroupBinding)
            return destroyAndUnregisterObject(*renderGroupBinding, errorReporting);

        auto meshNodeBinding = dynamic_cast<MeshNodeBinding*>(&object);
        if (meshNodeBinding)
            return destroyAndUnregisterObject(*meshNodeBinding, errorReporting);

        auto skinBinding = dynamic_cast<SkinBinding*>(&object);
        if (skinBinding)
            return destroyAndUnregisterObject(*skinBinding, errorReporting);

        auto animNode = dynamic_cast<AnimationNode*>(&object);
        if (animNode)
            return destroyAndUnregisterObject(*animNode, errorReporting);

        auto dataArray = dynamic_cast<DataArray*>(&object);
        if (dataArray)
            return destroyInternal(*dataArray, errorReporting);

        auto timer = dynamic_cast<TimerNode*>(&object);
        if (timer)
            return destroyAndUnregisterObject(*timer, errorReporting);

        auto anchor = dynamic_cast<AnchorPoint*>(&object);
        if (anchor)
            return destroyInternal(*anchor, errorReporting);

        auto renderBufferBinding = dynamic_cast<RenderBufferBinding*>(&object);
        if (renderBufferBinding)
            return destroyAndUnregisterObject(*renderBufferBinding, errorReporting);

        errorReporting.set(fmt::format("Tried to destroy object '{}' with unknown type", object.getName()), &object);

        return false;
    }

    bool ApiObjects::destroyInternal(DataArray& dataArray, ErrorReporting& errorReporting)
    {
        for (const auto& animNode : m_animationNodes)
        {
            for (const auto& channel : animNode->getChannels())
            {
                if (channel.timeStamps == &dataArray ||
                    channel.keyframes == &dataArray ||
                    channel.tangentsIn == &dataArray ||
                    channel.tangentsOut == &dataArray)
                {
                    errorReporting.set(fmt::format("Failed to destroy data array '{}', it is used in animation node '{}' channel '{}'", dataArray.getName(), animNode->getName(), channel.name), &dataArray);
                    return false;
                }
            }
        }

        return destroyAndUnregisterObject(dataArray, errorReporting);
    }

    bool ApiObjects::destroyInternal(LuaModule& luaModule, ErrorReporting& errorReporting)
    {
        for (const auto& script : m_scripts)
        {
            for (const auto& moduleInUse : script->impl().getModules())
            {
                if (moduleInUse.second == &luaModule)
                {
                    errorReporting.set(fmt::format("Failed to destroy LuaModule '{}', it is used in LuaScript '{}'", luaModule.getName(), script->getName()), &luaModule);
                    return false;
                }
            }
        }

        return destroyAndUnregisterObject(luaModule, errorReporting);
    }

    bool ApiObjects::destroyInternal(NodeBinding& nodeBinding, ErrorReporting& errorReporting)
    {
        for (const auto& anchor : m_anchorPoints)
        {
            if (anchor->impl().getNodeBinding().getSceneObjectId() == nodeBinding.getSceneObjectId())
            {
                errorReporting.set(fmt::format("Failed to destroy Ramses node binding '{}', it is used in anchor point '{}'", nodeBinding.getName(), anchor->getName()), &nodeBinding);
                return false;
            }
        }

        for (const auto& skin : m_skinBindings)
        {
            for (const auto node : skin->impl().getJoints())
            {
                if (node->getSceneObjectId() == nodeBinding.getSceneObjectId())
                {
                    errorReporting.set(fmt::format("Failed to destroy Ramses node binding '{}', it is used in skin binding '{}'", nodeBinding.getName(), skin->getName()), &nodeBinding);
                    return false;
                }
            }
        }

        return destroyAndUnregisterObject(nodeBinding, errorReporting);
    }

    bool ApiObjects::destroyInternal(AppearanceBinding& appearanceBinding, ErrorReporting& errorReporting)
    {
        for (const auto& skin : m_skinBindings)
        {
            if (skin->impl().getAppearanceBinding().getSceneObjectId() == appearanceBinding.getSceneObjectId())
            {
                errorReporting.set(fmt::format("Failed to destroy Ramses appearance binding '{}', it is used in skin binding '{}'", appearanceBinding.getName(), skin->getName()), &appearanceBinding);
                return false;
            }
        }

        return destroyAndUnregisterObject(appearanceBinding, errorReporting);
    }

    bool ApiObjects::destroyInternal(CameraBinding& cameraBinding, ErrorReporting& errorReporting)
    {
        for (const auto& anchor : m_anchorPoints)
        {
            if (anchor->impl().getCameraBinding().getSceneObjectId() == cameraBinding.getSceneObjectId())
            {
                errorReporting.set(fmt::format("Failed to destroy Ramses camera binding '{}', it is used in anchor point '{}'", cameraBinding.getName(), anchor->getName()), &cameraBinding);
                return false;
            }
        }

        return destroyAndUnregisterObject(cameraBinding, errorReporting);
    }

    bool ApiObjects::destroyInternal(AnchorPoint& node, ErrorReporting& errorReporting)
    {
        if (std::find(m_anchorPoints.cbegin(), m_anchorPoints.cend(), &node) != m_anchorPoints.end())
        {
            m_logicNodeDependencies.removeBindingDependency(node.impl().getNodeBinding(), node.m_impl);
            m_logicNodeDependencies.removeBindingDependency(node.impl().getCameraBinding(), node.m_impl);
        }

        return destroyAndUnregisterObject(node, errorReporting);
    }

    template <typename T, typename ImplT>
    T& ApiObjects::createAndRegisterObject(std::unique_ptr<ImplT> impl)
    {
        static_assert(std::is_base_of_v<LogicObject, T>, "Meant for LogicObject instances only");
        std::unique_ptr<T, std::function<void(LogicObject*)>> object{ new T{ std::move(impl) }, [](LogicObject* obj_) { delete obj_; } };

        T& objRaw = *object;

        // move to owning pool
        assert(std::find_if(m_objectsOwningContainer.cbegin(), m_objectsOwningContainer.cend(), [&](const auto& obj) { return obj.get() == &objRaw; }) == m_objectsOwningContainer.cend());
        m_objectsOwningContainer.push_back(std::move(object));

        // set reference to HL object so impl can access its HL instance
        objRaw.impl().setLogicObject(objRaw);

        // store in general pool of logic objects
        assert(std::find(m_logicObjects.cbegin(), m_logicObjects.cend(), &objRaw) == m_logicObjects.cend());
        assert(std::find_if(m_logicObjects.cbegin(), m_logicObjects.cend(), [&objRaw](const auto o) { return o->getSceneObjectId() == objRaw.getSceneObjectId(); }) == m_logicObjects.cend());
        m_logicObjects.push_back(&objRaw);

        // put into own scope as clang-tidy is confused with constexpr branches and reports indentation warning
        {
            // store in its typed pool
            if constexpr (std::is_same_v<LuaScript, T>)
            {
                this->m_scripts.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<LuaInterface, T>)
            {
                this->m_interfaces.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<LuaModule, T>)
            {
                this->m_luaModules.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<NodeBinding, T>)
            {
                this->m_nodeBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<AppearanceBinding, T>)
            {
                this->m_appearanceBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<CameraBinding, T>)
            {
                this->m_cameraBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<RenderPassBinding, T>)
            {
                this->m_renderPassBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<RenderGroupBinding, T>)
            {
                this->m_renderGroupBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<MeshNodeBinding, T>)
            {
                this->m_meshNodeBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<SkinBinding, T>)
            {
                this->m_skinBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<DataArray, T>)
            {
                this->m_dataArrays.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<AnimationNode, T>)
            {
                this->m_animationNodes.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<TimerNode, T>)
            {
                this->m_timerNodes.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<AnchorPoint, T>)
            {
                this->m_anchorPoints.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<RenderBufferBinding, T>)
            {
                this->m_renderBufferBindings.push_back(&objRaw);
            }
            else
            {
                assert(!"unhandled type");
            }

            // types derived from LogicNode are added to node dependency graph
            if constexpr (std::is_base_of_v<LogicNode, T>)
                m_logicNodeDependencies.addNode(objRaw.LogicNode::m_impl);
        }

        return objRaw;
    }

    template <typename T>
    bool ApiObjects::destroyAndUnregisterObject(T& objToDelete, ErrorReporting& errorReporting)
    {
        static_assert(std::is_base_of_v<LogicObject, T>, "Meant for LogicObject instances only");

        const auto findOwnedObj = std::find_if(m_objectsOwningContainer.cbegin(), m_objectsOwningContainer.cend(), [&](const auto& obj) { return obj.get() == &objToDelete; });
        if (findOwnedObj == m_objectsOwningContainer.cend())
        {
            errorReporting.set(fmt::format("Failed to destroy object '{}', cannot find it in this LogicEngine instance.", objToDelete.impl().getIdentificationString()), &objToDelete);
            return false;
        }

        auto eraseFromPool = [](auto& obj, auto& pool) { pool.erase(std::find(pool.begin(), pool.end(), &obj)); };

        // put into own scope as clang-tidy is confused with constexpr branches and reports indentation warning
        {
            if constexpr (std::is_base_of_v<LogicNode, T>)
            {
                LogicNodeImpl& logicNodeImpl = objToDelete.LogicNode::m_impl;
                m_logicNodeDependencies.removeNode(logicNodeImpl);
            }
        }

        // put into own scope as clang-tidy is confused with constexpr branches and reports indentation warning
        {
            if constexpr (std::is_same_v<LuaScript, T>)
            {
                eraseFromPool(objToDelete, this->m_scripts);
            }
            else if constexpr (std::is_same_v<LuaInterface, T>)
            {
                eraseFromPool(objToDelete, this->m_interfaces);
            }
            else if constexpr (std::is_same_v<LuaModule, T>)
            {
                eraseFromPool(objToDelete, this->m_luaModules);
            }
            else if constexpr (std::is_same_v<NodeBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_nodeBindings);
            }
            else if constexpr (std::is_same_v<AppearanceBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_appearanceBindings);
            }
            else if constexpr (std::is_same_v<CameraBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_cameraBindings);
            }
            else if constexpr (std::is_same_v<RenderPassBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_renderPassBindings);
            }
            else if constexpr (std::is_same_v<RenderGroupBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_renderGroupBindings);
            }
            else if constexpr (std::is_same_v<MeshNodeBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_meshNodeBindings);
            }
            else if constexpr (std::is_same_v<SkinBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_skinBindings);
            }
            else if constexpr (std::is_same_v<DataArray, T>)
            {
                eraseFromPool(objToDelete, this->m_dataArrays);
            }
            else if constexpr (std::is_same_v<AnimationNode, T>)
            {
                eraseFromPool(objToDelete, this->m_animationNodes);
            }
            else if constexpr (std::is_same_v<TimerNode, T>)
            {
                eraseFromPool(objToDelete, this->m_timerNodes);
            }
            else if constexpr (std::is_same_v<AnchorPoint, T>)
            {
                eraseFromPool(objToDelete, this->m_anchorPoints);
            }
            else if constexpr (std::is_same_v<RenderBufferBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_renderBufferBindings);
            }
            else
            {
                assert(!"unhandled type");
            }
        }

        const auto findLogicObj = std::find(m_logicObjects.cbegin(), m_logicObjects.cend(), &objToDelete);
        assert(findLogicObj != m_logicObjects.cend() && "Can't find LogicObject in logic objects!");
        m_logicObjects.erase(findLogicObj);

        m_objectsOwningContainer.erase(findOwnedObj);

        return true;
    }

    void ApiObjects::validateInterfaces(ValidationReportImpl& report) const
    {
        // check if there are any outputs without link
        // Note: this is different from the check for dangling nodes/content, where nodes are checked if they have ANY
        // linked inputs and/or outputs depending on node type. Interfaces on the other hand must have ALL of their outputs
        // linked to be valid.

        for (const auto* intf : m_interfaces)
        {
            std::vector<const Property*> unlinkedProperties = intf->m_interface.collectUnlinkedProperties();
            for (const auto* output : unlinkedProperties)
                report.add(EIssueType::Warning, ::fmt::format("Interface [{}] has unlinked output [{}]", intf->getName(), output->getName()), intf);
        }

        // check if there are any name conflicts
        auto interfacesByName = m_interfaces;
        std::sort(interfacesByName.begin(), interfacesByName.end(), [](const auto i1, const auto i2) { return i1->getName() < i2->getName(); });
        const auto duplicateIt = std::adjacent_find(interfacesByName.cbegin(), interfacesByName.cend(), [](const auto i1, const auto i2) { return i1->getName() == i2->getName(); });
        if (duplicateIt != interfacesByName.cend())
            report.add(EIssueType::Error, fmt::format("Interface [{}] does not have a unique name", (*duplicateIt)->getName()), *duplicateIt);
    }

    void ApiObjects::validateDanglingNodes(ValidationReportImpl& report) const
    {
        //nodes with no outputs linked
        for (const auto* logicObj : m_logicObjects)
        {
            const auto* objAsNode = dynamic_cast<const ramses::LogicNode*>(logicObj);
            if (objAsNode != nullptr)
            {
                if (dynamic_cast<const ramses::LuaInterface*>(logicObj) ||  // interfaces have their own validation logic in ApiObjects::validateInterfaces
                    dynamic_cast<const ramses::RamsesBinding*>(logicObj) || // bindings have no outputs
                    dynamic_cast<const ramses::AnchorPoint*>(logicObj))     // anchor points are being used in special way which sometimes involves reading output value directly by application only
                    continue;

                assert(objAsNode->getOutputs() != nullptr);
                // only check for unlinked outputs if there are any
                if (objAsNode->getOutputs()->getChildCount() != 0u)
                {
                    bool anyOutputLinked = false;
                    for (const auto* output : objAsNode->getOutputs()->impl().collectLeafChildren())
                        anyOutputLinked |= (output->hasOutgoingLink());

                    if (!anyOutputLinked)
                        report.add(EIssueType::Warning, fmt::format("Node [{}] has no outgoing links! Node should be deleted or properly linked!", objAsNode->getName()), objAsNode);
                }
            }
        }

        // collect node bindings used in anchor points and skin bindings
        // - these are allowed to have no incoming links because they are being read from
        std::unordered_set<const RamsesBinding*> bindingsInUse;
        for (auto* anchor : m_anchorPoints)
        {
            bindingsInUse.insert(anchor->impl().getNodeBinding().getLogicObject().as<RamsesBinding>());
            bindingsInUse.insert(anchor->impl().getCameraBinding().getLogicObject().as<RamsesBinding>());
        }
        for (const auto* skin : m_skinBindings)
        {
            bindingsInUse.insert(skin->impl().getAppearanceBinding().getLogicObject().as<RamsesBinding>());
            for (const auto* joint : skin->impl().getJoints())
                bindingsInUse.insert(joint->getLogicObject().as<RamsesBinding>());
        }

        //nodes with no linked inputs
        for (const auto* logicObj : m_logicObjects)
        {
            const auto* objAsNode = dynamic_cast<const ramses::LogicNode*>(logicObj);
            if (objAsNode != nullptr)
            {
                if (dynamic_cast<const ramses::LuaInterface*>(logicObj) ||  // interfaces have their own validation logic in ApiObjects::validateInterfaces
                    dynamic_cast<const ramses::TimerNode*>(logicObj) ||     // timer not having input is valid use case which enables internal clock ticker
                    dynamic_cast<const ramses::AnchorPoint*>(logicObj) ||   // anchor points have no inputs
                    dynamic_cast<const ramses::SkinBinding*>(logicObj))     // skinbinding has no inputs
                    continue;

                if (bindingsInUse.count(dynamic_cast<const ramses::RamsesBinding*>(logicObj)) != 0) // bindings in use by other rlogic objects can be without incoming links
                    continue;

                assert(objAsNode->getInputs() != nullptr);
                // only check for unlinked inputs if there are any
                if (objAsNode->getInputs()->getChildCount() != 0u)
                {
                    bool anyInputLinked = false;
                    for (const auto* input : objAsNode->getInputs()->impl().collectLeafChildren())
                        anyInputLinked |= (input->hasIncomingLink());

                    if (!anyInputLinked)
                        report.add(EIssueType::Warning, fmt::format("Node [{}] has no incoming links! Node should be deleted or properly linked!", objAsNode->getName()), objAsNode);
                }
            }
        }
    }

    template <typename T>
    const ApiObjectContainer<T>& ApiObjects::getApiObjectContainer() const
    {
        if constexpr (std::is_same_v<T, LogicObject>)
        {
            return m_logicObjects;
        }
        else if constexpr (std::is_same_v<T, LuaScript>)
        {
            return m_scripts;
        }
        else if constexpr (std::is_same_v<T, LuaInterface>)
        {
            return m_interfaces;
        }
        else if constexpr (std::is_same_v<T, LuaModule>)
        {
            return m_luaModules;
        }
        else if constexpr (std::is_same_v<T, NodeBinding>)
        {
            return m_nodeBindings;
        }
        else if constexpr (std::is_same_v<T, AppearanceBinding>)
        {
            return m_appearanceBindings;
        }
        else if constexpr (std::is_same_v<T, CameraBinding>)
        {
            return m_cameraBindings;
        }
        else if constexpr (std::is_same_v<T, RenderPassBinding>)
        {
            return m_renderPassBindings;
        }
        else if constexpr (std::is_same_v<T, RenderGroupBinding>)
        {
            return m_renderGroupBindings;
        }
        else if constexpr (std::is_same_v<T, MeshNodeBinding>)
        {
            return m_meshNodeBindings;
        }
        else if constexpr (std::is_same_v<T, SkinBinding>)
        {
            return m_skinBindings;
        }
        else if constexpr (std::is_same_v<T, DataArray>)
        {
            return m_dataArrays;
        }
        else if constexpr (std::is_same_v<T, AnimationNode>)
        {
            return m_animationNodes;
        }
        else if constexpr (std::is_same_v<T, TimerNode>)
        {
            return m_timerNodes;
        }
        else if constexpr (std::is_same_v<T, AnchorPoint>)
        {
            return m_anchorPoints;
        }
        else if constexpr (std::is_same_v<T, RenderBufferBinding>)
        {
            return m_renderBufferBindings;
        }
    }

    template <typename T>
    ApiObjectContainer<T>& ApiObjects::getApiObjectContainer()
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) non-const version of getApiObjectContainer cast to its const version to avoid duplicating code
        return const_cast<ApiObjectContainer<T>&>((const_cast<const ApiObjects&>(*this)).getApiObjectContainer<T>());
    }

    const ApiObjectOwningContainer& ApiObjects::getApiObjectOwningContainer() const
    {
        return m_objectsOwningContainer;
    }

    LogicNodeDependencies& ApiObjects::getLogicNodeDependencies()
    {
        return m_logicNodeDependencies;
    }

    const LogicNodeDependencies& ApiObjects::getLogicNodeDependencies() const
    {
        return m_logicNodeDependencies;
    }

    flatbuffers::Offset<rlogic_serialization::ApiObjects> ApiObjects::Serialize(const ApiObjects& apiObjects, flatbuffers::FlatBufferBuilder& builder, ELuaSavingMode luaSavingMode)
    {
        SerializationMap serializationMap;

        std::vector<flatbuffers::Offset<rlogic_serialization::LuaModule>> luaModules;
        luaModules.reserve(apiObjects.m_luaModules.size());
        for (const auto& luaModule : apiObjects.m_luaModules)
            luaModules.push_back(LuaModuleImpl::Serialize(luaModule->m_impl, builder, serializationMap, luaSavingMode));

        std::vector<flatbuffers::Offset<rlogic_serialization::LuaScript>> luascripts;
        luascripts.reserve(apiObjects.m_scripts.size());
        std::transform(apiObjects.m_scripts.begin(), apiObjects.m_scripts.end(), std::back_inserter(luascripts),
            [&builder, &serializationMap, luaSavingMode](const std::vector<LuaScript*>::value_type& it) {
                return LuaScriptImpl::Serialize(it->impl(), builder, serializationMap, luaSavingMode);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>> luaInterfaces;
        luascripts.reserve(apiObjects.m_interfaces.size());
        std::transform(apiObjects.m_interfaces.cbegin(), apiObjects.m_interfaces.cend(), std::back_inserter(luaInterfaces),
            [&builder, &serializationMap](const std::vector<LuaInterface*>::value_type& it) {
                return LuaInterfaceImpl::Serialize(it->m_interface, builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::NodeBinding>> nodebindings;
        nodebindings.reserve(apiObjects.m_nodeBindings.size());
        std::transform(apiObjects.m_nodeBindings.begin(),
            apiObjects.m_nodeBindings.end(),
            std::back_inserter(nodebindings),
            [&builder, &serializationMap](const std::vector<NodeBinding*>::value_type& it) {
                return NodeBindingImpl::Serialize(it->impl(), builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::AppearanceBinding>> appearancebindings;
        appearancebindings.reserve(apiObjects.m_appearanceBindings.size());
        std::transform(apiObjects.m_appearanceBindings.begin(),
            apiObjects.m_appearanceBindings.end(),
            std::back_inserter(appearancebindings),
            [&builder, &serializationMap](const std::vector<AppearanceBinding*>::value_type& it) {
                return AppearanceBindingImpl::Serialize(it->impl(), builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::CameraBinding>> camerabindings;
        camerabindings.reserve(apiObjects.m_cameraBindings.size());
        std::transform(apiObjects.m_cameraBindings.begin(),
            apiObjects.m_cameraBindings.end(),
            std::back_inserter(camerabindings),
            [&builder, &serializationMap](const std::vector<CameraBinding*>::value_type& it) {
                return CameraBindingImpl::Serialize(it->impl(), builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::RenderPassBinding>> renderpassbindings;
        renderpassbindings.reserve(apiObjects.m_renderPassBindings.size());
        std::transform(apiObjects.m_renderPassBindings.begin(),
            apiObjects.m_renderPassBindings.end(),
            std::back_inserter(renderpassbindings),
            [&builder, &serializationMap](const std::vector<RenderPassBinding*>::value_type& it) {
                return RenderPassBindingImpl::Serialize(it->impl(), builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>> dataArrays;
        dataArrays.reserve(apiObjects.m_dataArrays.size());
        for (const auto& da : apiObjects.m_dataArrays)
        {
            dataArrays.push_back(DataArrayImpl::Serialize(da->m_impl, builder, serializationMap));
            serializationMap.storeDataArray(da->getSceneObjectId(), dataArrays.back());
        }

        // animation nodes must go after data arrays because they reference them
        std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>> animationNodes;
        animationNodes.reserve(apiObjects.m_animationNodes.size());
        for (const auto& animNode : apiObjects.m_animationNodes)
            animationNodes.push_back(AnimationNodeImpl::Serialize(animNode->m_animationNodeImpl, builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>> timerNodes;
        timerNodes.reserve(apiObjects.m_timerNodes.size());
        for (const auto& timerNode : apiObjects.m_timerNodes)
            timerNodes.push_back(TimerNodeImpl::Serialize(timerNode->impl(), builder, serializationMap));

        // anchor points must go after node and camera bindings because they reference them
        std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>> anchorPoints;
        anchorPoints.reserve(apiObjects.m_anchorPoints.size());
        for (const auto& anchorPoint : apiObjects.m_anchorPoints)
            anchorPoints.push_back(AnchorPointImpl::Serialize(anchorPoint->impl(), builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::RenderGroupBinding>> renderGroupBindings;
        renderGroupBindings.reserve(apiObjects.m_renderGroupBindings.size());
        for (const auto& rgBinding : apiObjects.m_renderGroupBindings)
            renderGroupBindings.push_back(RenderGroupBindingImpl::Serialize(rgBinding->impl(), builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::MeshNodeBinding>> meshNodeBindings;
        meshNodeBindings.reserve(apiObjects.m_meshNodeBindings.size());
        for (const auto& mnBinding : apiObjects.m_meshNodeBindings)
            meshNodeBindings.push_back(MeshNodeBindingImpl::Serialize(mnBinding->impl(), builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>> skinBindings;
        skinBindings.reserve(apiObjects.m_skinBindings.size());
        for (const auto& skinBinding : apiObjects.m_skinBindings)
            skinBindings.push_back(SkinBindingImpl::Serialize(skinBinding->impl(), builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::RenderBufferBinding>> rbBindings;
        rbBindings.reserve(apiObjects.m_renderBufferBindings.size());
        for (const auto& rbBinding : apiObjects.m_renderBufferBindings)
            rbBindings.push_back(RenderBufferBindingImpl::Serialize(rbBinding->impl(), builder, serializationMap));

        // links must go last due to dependency on serialized properties
        const auto collectedLinks = apiObjects.collectPropertyLinks();
        std::vector<flatbuffers::Offset<rlogic_serialization::Link>> links;
        links.reserve(collectedLinks.size());
        for (const auto& link : collectedLinks)
        {
            links.push_back(rlogic_serialization::CreateLink(builder,
                serializationMap.resolvePropertyOffset(link.source->impl()),
                serializationMap.resolvePropertyOffset(link.target->impl()),
                link.isWeakLink));
        }

        const auto fbModules = builder.CreateVector(luaModules);
        const auto fbScripts = builder.CreateVector(luascripts);
        const auto fbInterfaces = builder.CreateVector(luaInterfaces);
        const auto fbNodeBindings = builder.CreateVector(nodebindings);
        const auto fbAppearanceBindings = builder.CreateVector(appearancebindings);
        const auto fbCameraBindings = builder.CreateVector(camerabindings);
        const auto fbDataArrays = builder.CreateVector(dataArrays);
        const auto fbAnimations = builder.CreateVector(animationNodes);
        const auto fbTimers = builder.CreateVector(timerNodes);
        const auto fbLinks = builder.CreateVector(links);
        const auto fbRenderPasses = builder.CreateVector(renderpassbindings);
        const auto fbAnchorPoints = builder.CreateVector(anchorPoints);
        const auto fbRenderGroupBindings = builder.CreateVector(renderGroupBindings);
        const auto fbMeshNodeBindings = builder.CreateVector(meshNodeBindings);
        const auto fbSkinBindings = builder.CreateVector(skinBindings);
        const auto fbRbBindings = builder.CreateVector(rbBindings);

        const auto logicEngine = rlogic_serialization::CreateApiObjects(
            builder,
            fbModules,
            fbScripts,
            fbInterfaces,
            fbNodeBindings,
            fbAppearanceBindings,
            fbCameraBindings,
            fbDataArrays,
            fbAnimations,
            fbTimers,
            fbLinks,
            fbRenderPasses,
            fbAnchorPoints,
            fbRenderGroupBindings,
            fbSkinBindings,
            fbMeshNodeBindings,
            fbRbBindings
            );

        builder.Finish(logicEngine);

        return logicEngine;
    }

    std::unique_ptr<ApiObjects> ApiObjects::Deserialize(
        SceneImpl& scene,
        const rlogic_serialization::ApiObjects& apiObjects,
        const IRamsesObjectResolver& ramsesResolver,
        const std::string& dataSourceDescription,
        ErrorReporting& errorReporting,
        ramses::EFeatureLevel featureLevel)
    {
        // Collect data here, only return if no error occurred
        auto deserialized = std::make_unique<ApiObjects>(featureLevel, scene);

        // Collect deserialized object mappings to resolve dependencies
        DeserializationMap deserializationMap{ scene };

        if (!apiObjects.luaModules())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing Lua modules container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.luaScripts())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing Lua scripts container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.luaInterfaces())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing Lua interfaces container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.nodeBindings())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing node bindings container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.appearanceBindings())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing appearance bindings container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.cameraBindings())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing camera bindings container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.renderPassBindings())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing renderpass bindings container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.links())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing links container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.dataArrays())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing data arrays container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.animationNodes())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing animation nodes container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.timerNodes())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing timer nodes container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.anchorPoints())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing anchor points container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.renderGroupBindings())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing rendergroup bindings container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.meshNodeBindings())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing meshnode bindings container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.skinBindings())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing skin bindings container!", nullptr);
            return nullptr;
        }

        if (!apiObjects.renderBufferBindings())
        {
            errorReporting.set("Fatal error during loading from serialized data: missing render buffer bindings container!", nullptr);
            return nullptr;
        }

        const size_t logicObjectsTotalSize =
            static_cast<size_t>(apiObjects.luaModules()->size()) +
            static_cast<size_t>(apiObjects.luaScripts()->size()) +
            static_cast<size_t>(apiObjects.luaInterfaces()->size()) +
            static_cast<size_t>(apiObjects.nodeBindings()->size()) +
            static_cast<size_t>(apiObjects.appearanceBindings()->size()) +
            static_cast<size_t>(apiObjects.cameraBindings()->size()) +
            static_cast<size_t>(apiObjects.renderPassBindings()->size()) +
            static_cast<size_t>(apiObjects.dataArrays()->size()) +
            static_cast<size_t>(apiObjects.animationNodes()->size()) +
            static_cast<size_t>(apiObjects.timerNodes()->size()) +
            static_cast<size_t>(apiObjects.anchorPoints()->size()) +
            static_cast<size_t>(apiObjects.renderGroupBindings()->size()) +
            static_cast<size_t>(apiObjects.meshNodeBindings()->size()) +
            static_cast<size_t>(apiObjects.skinBindings()->size()) +
            static_cast<size_t>(apiObjects.renderBufferBindings()->size());

        deserialized->m_objectsOwningContainer.reserve(logicObjectsTotalSize);
        deserialized->m_logicObjects.reserve(logicObjectsTotalSize);

        const auto& luaModules = *apiObjects.luaModules();
        deserialized->m_luaModules.reserve(luaModules.size());
        for (const auto* module : luaModules)
        {
            assert(module);
            std::unique_ptr<LuaModuleImpl> deserializedModule = LuaModuleImpl::Deserialize(*deserialized->m_solState, *module, errorReporting, deserializationMap);
            if (!deserializedModule)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<LuaModule, LuaModuleImpl>(std::move(deserializedModule));
            deserializationMap.storeLogicObject(obj.getSceneObjectId(), obj.m_impl);
        }

        const auto& luascripts = *apiObjects.luaScripts();
        deserialized->m_scripts.reserve(luascripts.size());
        for (const auto* script : luascripts)
        {
            assert(script);
            std::unique_ptr<LuaScriptImpl> deserializedScript = LuaScriptImpl::Deserialize(*deserialized->m_solState, *script, errorReporting, deserializationMap);
            if (!deserializedScript)
                return nullptr;

            deserialized->createAndRegisterObject<LuaScript, LuaScriptImpl>(std::move(deserializedScript));
        }

        const auto& luaInterfaces = *apiObjects.luaInterfaces();
        deserialized->m_interfaces.reserve(luaInterfaces.size());
        for (const auto* intf : luaInterfaces)
        {
            assert(intf);
            std::unique_ptr<LuaInterfaceImpl> deserializedInterface = LuaInterfaceImpl::Deserialize(*intf, errorReporting, deserializationMap);
            if (!deserializedInterface)
                return nullptr;

            deserialized->createAndRegisterObject<LuaInterface, LuaInterfaceImpl>(std::move(deserializedInterface));
        }

        const auto& nodeBindings = *apiObjects.nodeBindings();
        deserialized->m_nodeBindings.reserve(nodeBindings.size());
        for (const auto* binding : nodeBindings)
        {
            assert(binding);
            std::unique_ptr<NodeBindingImpl> deserializedBinding = NodeBindingImpl::Deserialize(*binding, ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<NodeBinding, NodeBindingImpl>(std::move(deserializedBinding));
            deserializationMap.storeLogicObject(obj.getSceneObjectId(), obj.m_impl);
        }

        const auto& appearanceBindings = *apiObjects.appearanceBindings();
        deserialized->m_appearanceBindings.reserve(appearanceBindings.size());
        for (const auto* binding : appearanceBindings)
        {
            assert(binding);
            std::unique_ptr<AppearanceBindingImpl> deserializedBinding = AppearanceBindingImpl::Deserialize(*binding, ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<AppearanceBinding, AppearanceBindingImpl>(std::move(deserializedBinding));
            deserializationMap.storeLogicObject(obj.getSceneObjectId(), obj.m_impl);
        }

        const auto& cameraBindings = *apiObjects.cameraBindings();
        deserialized->m_cameraBindings.reserve(cameraBindings.size());
        for (const auto* binding : cameraBindings)
        {
            assert(binding);
            std::unique_ptr<CameraBindingImpl> deserializedBinding = CameraBindingImpl::Deserialize(*binding, ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<CameraBinding, CameraBindingImpl>(std::move(deserializedBinding));
            deserializationMap.storeLogicObject(obj.getSceneObjectId(), obj.m_impl);
        }

        const auto& renderPassBindings = *apiObjects.renderPassBindings();
        deserialized->m_renderPassBindings.reserve(renderPassBindings.size());
        for (const auto* binding : renderPassBindings)
        {
            assert(binding);
            std::unique_ptr<RenderPassBindingImpl> deserializedBinding = RenderPassBindingImpl::Deserialize(*binding, ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<RenderPassBinding, RenderPassBindingImpl>(std::move(deserializedBinding));
            deserializationMap.storeLogicObject(obj.getSceneObjectId(), obj.m_impl);
        }

        const auto& dataArrays = *apiObjects.dataArrays();
        deserialized->m_dataArrays.reserve(dataArrays.size());
        for (const auto* fbData : dataArrays)
        {
            assert(fbData);
            auto deserializedDataArray = DataArrayImpl::Deserialize(*fbData, errorReporting, deserializationMap);
            if (!deserializedDataArray)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<DataArray, DataArrayImpl>(std::move(deserializedDataArray));
            deserializationMap.storeDataArray(*fbData, obj);
        }

        // animation nodes must go after data arrays because they need to resolve references
        const auto& animNodes = *apiObjects.animationNodes();
        deserialized->m_animationNodes.reserve(animNodes.size());
        for (const auto* fbData : animNodes)
        {
            assert(fbData);
            auto deserializedAnimNode = AnimationNodeImpl::Deserialize(*fbData, errorReporting, deserializationMap);
            if (!deserializedAnimNode)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<AnimationNode, AnimationNodeImpl>(std::move(deserializedAnimNode));
            deserializationMap.storeLogicObject(obj.getSceneObjectId(), obj.m_impl);
        }

        const auto& timerNodes = *apiObjects.timerNodes();
        deserialized->m_timerNodes.reserve(timerNodes.size());
        for (const auto* fbData : timerNodes)
        {
            assert(fbData);
            auto deserializedTimer = TimerNodeImpl::Deserialize(*fbData, errorReporting, deserializationMap);
            if (!deserializedTimer)
                return nullptr;

            deserialized->createAndRegisterObject<TimerNode, TimerNodeImpl>(std::move(deserializedTimer));
        }

        // anchor points must go after node and camera bindings because they need to resolve references
        const auto& anchorPoints = *apiObjects.anchorPoints();
        deserialized->m_anchorPoints.reserve(anchorPoints.size());
        for (const auto* fbAnchor : anchorPoints)
        {
            assert(fbAnchor);
            std::unique_ptr<AnchorPointImpl> deserializedAnchor = AnchorPointImpl::Deserialize(*fbAnchor, errorReporting, deserializationMap);
            if (!deserializedAnchor)
                return nullptr;

            deserialized->createAndRegisterObject<AnchorPoint, AnchorPointImpl>(std::move(deserializedAnchor));
        }

        const auto& renderGroupBindings = *apiObjects.renderGroupBindings();
        deserialized->m_renderGroupBindings.reserve(renderGroupBindings.size());
        for (const auto* binding : renderGroupBindings)
        {
            assert(binding);
            std::unique_ptr<RenderGroupBindingImpl> deserializedBinding = RenderGroupBindingImpl::Deserialize(*binding, ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            deserialized->createAndRegisterObject<RenderGroupBinding, RenderGroupBindingImpl>(std::move(deserializedBinding));
        }

        // skin bindings must go after node and appearance bindings because they need to resolve references
        const auto& skinBindings = *apiObjects.skinBindings();
        deserialized->m_skinBindings.reserve(skinBindings.size());
        for (const auto* binding : skinBindings)
        {
            assert(binding);
            std::unique_ptr<SkinBindingImpl> deserializedBinding = SkinBindingImpl::Deserialize(*binding, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            deserialized->createAndRegisterObject<SkinBinding, SkinBindingImpl>(std::move(deserializedBinding));
        }

        const auto& meshNodeBindings = *apiObjects.meshNodeBindings();
        deserialized->m_meshNodeBindings.reserve(meshNodeBindings.size());
        for (const auto* binding : meshNodeBindings)
        {
            assert(binding);
            std::unique_ptr<MeshNodeBindingImpl> deserializedBinding = MeshNodeBindingImpl::Deserialize(*binding, ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            deserialized->createAndRegisterObject<MeshNodeBinding, MeshNodeBindingImpl>(std::move(deserializedBinding));
        }

        const auto& rbBindings = *apiObjects.renderBufferBindings();
        deserialized->m_renderBufferBindings.reserve(rbBindings.size());
        for (const auto* binding : rbBindings)
        {
            assert(binding);
            std::unique_ptr<RenderBufferBindingImpl> deserializedBinding = RenderBufferBindingImpl::Deserialize(*binding, ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            deserialized->createAndRegisterObject<RenderBufferBinding, RenderBufferBindingImpl>(std::move(deserializedBinding));
        }

        // links must go last due to dependency on deserialized properties
        const auto& links = *apiObjects.links();
        // TODO Violin move this code (serialization parts too) to LogicNodeDependencies
        for (const auto* rLink : links)
        {
            assert(rLink);

            if (!rLink->sourceProperty())
            {
                errorReporting.set("Fatal error during loading from serialized data: missing link source property!", nullptr);
                return nullptr;
            }

            if (!rLink->targetProperty())
            {
                errorReporting.set("Fatal error during loading from serialized data: missing link target property!", nullptr);
                return nullptr;
            }

            const rlogic_serialization::Property* sourceProp = rLink->sourceProperty();
            const rlogic_serialization::Property* targetProp = rLink->targetProperty();

            const bool success = deserialized->m_logicNodeDependencies.link(
                deserializationMap.resolvePropertyImpl(*sourceProp),
                deserializationMap.resolvePropertyImpl(*targetProp),
                rLink->isWeak(),
                errorReporting);
            if (!success)
            {
                errorReporting.set(
                    fmt::format("Fatal error during loading from {}! Could not link property '{}' to property '{}'!",
                        dataSourceDescription,
                        sourceProp->name()->string_view(),
                        targetProp->name()->string_view()
                    ), nullptr);
                return nullptr;
            }
        }

        return deserialized;
    }

    bool ApiObjects::bindingsDirty() const
    {
        return
            std::any_of(m_nodeBindings.cbegin(), m_nodeBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_appearanceBindings.cbegin(), m_appearanceBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_cameraBindings.cbegin(), m_cameraBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_renderPassBindings.cbegin(), m_renderPassBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_renderGroupBindings.cbegin(), m_renderGroupBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_meshNodeBindings.cbegin(), m_meshNodeBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_skinBindings.cbegin(), m_skinBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_renderBufferBindings.cbegin(), m_renderBufferBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); });
    }

    int ApiObjects::getNumElementsInLuaStack() const
    {
        return m_solState->getNumElementsInLuaStack();
    }

    const std::vector<PropertyLinkConst>& ApiObjects::getAllPropertyLinks() const
    {
        const std::vector<PropertyLink> links = collectPropertyLinks();
        m_collectedLinksConst.clear();
        m_collectedLinksConst.reserve(links.size());
        for (const auto& link : links)
            m_collectedLinksConst.push_back({link.source, link.target, link.isWeakLink});

        return m_collectedLinksConst;
    }

    const std::vector<PropertyLink>& ApiObjects::getAllPropertyLinks()
    {
        m_collectedLinks = collectPropertyLinks();
        return m_collectedLinks;
    }

    std::vector<PropertyLink> ApiObjects::collectPropertyLinks() const
    {
        std::vector<PropertyLink> links;

        std::deque<Property*> propsStack;
        for (auto& obj : m_logicObjects)
        {
            auto logicNode = obj->as<LogicNode>();
            if (!logicNode)
                continue;

            propsStack.clear();
            propsStack.push_back(logicNode->getInputs());
            propsStack.push_back(logicNode->getOutputs());
            while (!propsStack.empty())
            {
                auto prop = propsStack.back();
                propsStack.pop_back();
                if (prop == nullptr)
                    continue;

                auto incomingLink = prop->getIncomingLink();
                if (incomingLink)
                    links.push_back(*incomingLink);

                for (size_t i = 0u; i < prop->getChildCount(); ++i)
                    propsStack.push_back(prop->getChild(i));
            }
        }

        return links;
    }

    template DataArray* ApiObjects::createDataArray<float>(const std::vector<float>&, std::string_view);
    template DataArray* ApiObjects::createDataArray<vec2f>(const std::vector<vec2f>&, std::string_view);
    template DataArray* ApiObjects::createDataArray<vec3f>(const std::vector<vec3f>&, std::string_view);
    template DataArray* ApiObjects::createDataArray<vec4f>(const std::vector<vec4f>&, std::string_view);
    template DataArray* ApiObjects::createDataArray<int32_t>(const std::vector<int32_t>&, std::string_view);
    template DataArray* ApiObjects::createDataArray<vec2i>(const std::vector<vec2i>&, std::string_view);
    template DataArray* ApiObjects::createDataArray<vec3i>(const std::vector<vec3i>&, std::string_view);
    template DataArray* ApiObjects::createDataArray<vec4i>(const std::vector<vec4i>&, std::string_view);
    template DataArray* ApiObjects::createDataArray<std::vector<float>>(const std::vector<std::vector<float>>&, std::string_view);

    template ApiObjectContainer<LogicObject>&         ApiObjects::getApiObjectContainer<LogicObject>();
    template ApiObjectContainer<LuaScript>&           ApiObjects::getApiObjectContainer<LuaScript>();
    template ApiObjectContainer<LuaInterface>&        ApiObjects::getApiObjectContainer<LuaInterface>();
    template ApiObjectContainer<LuaModule>&           ApiObjects::getApiObjectContainer<LuaModule>();
    template ApiObjectContainer<NodeBinding>&         ApiObjects::getApiObjectContainer<NodeBinding>();
    template ApiObjectContainer<AppearanceBinding>&   ApiObjects::getApiObjectContainer<AppearanceBinding>();
    template ApiObjectContainer<CameraBinding>&       ApiObjects::getApiObjectContainer<CameraBinding>();
    template ApiObjectContainer<RenderPassBinding>&   ApiObjects::getApiObjectContainer<RenderPassBinding>();
    template ApiObjectContainer<RenderGroupBinding>&  ApiObjects::getApiObjectContainer<RenderGroupBinding>();
    template ApiObjectContainer<MeshNodeBinding>&     ApiObjects::getApiObjectContainer<MeshNodeBinding>();
    template ApiObjectContainer<SkinBinding>&         ApiObjects::getApiObjectContainer<SkinBinding>();
    template ApiObjectContainer<DataArray>&           ApiObjects::getApiObjectContainer<DataArray>();
    template ApiObjectContainer<AnimationNode>&       ApiObjects::getApiObjectContainer<AnimationNode>();
    template ApiObjectContainer<TimerNode>&           ApiObjects::getApiObjectContainer<TimerNode>();
    template ApiObjectContainer<AnchorPoint>&         ApiObjects::getApiObjectContainer<AnchorPoint>();
    template ApiObjectContainer<RenderBufferBinding>& ApiObjects::getApiObjectContainer<RenderBufferBinding>();

    template const ApiObjectContainer<LogicObject>&         ApiObjects::getApiObjectContainer<LogicObject>() const;
    template const ApiObjectContainer<LuaScript>&           ApiObjects::getApiObjectContainer<LuaScript>() const;
    template const ApiObjectContainer<LuaInterface>&        ApiObjects::getApiObjectContainer<LuaInterface>() const;
    template const ApiObjectContainer<LuaModule>&           ApiObjects::getApiObjectContainer<LuaModule>() const;
    template const ApiObjectContainer<NodeBinding>&         ApiObjects::getApiObjectContainer<NodeBinding>() const;
    template const ApiObjectContainer<AppearanceBinding>&   ApiObjects::getApiObjectContainer<AppearanceBinding>() const;
    template const ApiObjectContainer<CameraBinding>&       ApiObjects::getApiObjectContainer<CameraBinding>() const;
    template const ApiObjectContainer<RenderPassBinding>&   ApiObjects::getApiObjectContainer<RenderPassBinding>() const;
    template const ApiObjectContainer<RenderGroupBinding>&  ApiObjects::getApiObjectContainer<RenderGroupBinding>() const;
    template const ApiObjectContainer<MeshNodeBinding>&     ApiObjects::getApiObjectContainer<MeshNodeBinding>() const;
    template const ApiObjectContainer<SkinBinding>&         ApiObjects::getApiObjectContainer<SkinBinding>() const;
    template const ApiObjectContainer<DataArray>&           ApiObjects::getApiObjectContainer<DataArray>() const;
    template const ApiObjectContainer<AnimationNode>&       ApiObjects::getApiObjectContainer<AnimationNode>() const;
    template const ApiObjectContainer<TimerNode>&           ApiObjects::getApiObjectContainer<TimerNode>() const;
    template const ApiObjectContainer<AnchorPoint>&         ApiObjects::getApiObjectContainer<AnchorPoint>() const;
    template const ApiObjectContainer<RenderBufferBinding>& ApiObjects::getApiObjectContainer<RenderBufferBinding>() const;
}
