//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internals/ApiObjects.h"
#include "internals/ErrorReporting.h"

#include "ramses-sdk-build-config.h"

#include "ramses-logic/LogicObject.h"
#include "ramses-logic/LogicNode.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesRenderPassBinding.h"
#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/RamsesRenderGroupBindingElements.h"
#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "ramses-logic/SkinBinding.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/AnimationTypes.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/AnchorPoint.h"

#include "impl/PropertyImpl.h"
#include "impl/LuaScriptImpl.h"
#include "impl/LuaInterfaceImpl.h"
#include "impl/LuaModuleImpl.h"
#include "impl/RamsesNodeBindingImpl.h"
#include "impl/RamsesAppearanceBindingImpl.h"
#include "impl/RamsesCameraBindingImpl.h"
#include "impl/RamsesRenderPassBindingImpl.h"
#include "impl/RamsesRenderGroupBindingImpl.h"
#include "impl/RamsesMeshNodeBindingImpl.h"
#include "impl/SkinBindingImpl.h"
#include "impl/DataArrayImpl.h"
#include "impl/AnimationNodeImpl.h"
#include "impl/AnimationNodeConfigImpl.h"
#include "impl/TimerNodeImpl.h"
#include "impl/AnchorPointImpl.h"

#include "ramses-client-api/Node.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Camera.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/MeshNode.h"

#include "generated/ApiObjectsGen.h"
#include "generated/RamsesAppearanceBindingGen.h"
#include "generated/RamsesBindingGen.h"
#include "generated/RamsesCameraBindingGen.h"
#include "generated/RamsesNodeBindingGen.h"
#include "generated/LinkGen.h"
#include "generated/DataArrayGen.h"
#include "generated/AnimationNodeGen.h"
#include "generated/TimerNodeGen.h"

#include "fmt/format.h"
#include "TypeUtils.h"
#include "ValidationResults.h"
#include <deque>

namespace ramses::internal
{
    ApiObjects::ApiObjects(ramses::EFeatureLevel featureLevel)
        : m_featureLevel{ featureLevel }
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
                errorReporting.add(fmt::format("Failed to map Lua module '{}'! It was created on a different instance of LogicEngine.", module.first), module.second, EErrorType::IllegalArgument);
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

        auto impl = std::make_unique<LuaScriptImpl>(std::move(*compiledScript), scriptName, getNextLogicObjectId());
        impl->createRootProperties();

        return &createAndRegisterObject<LuaScript, LuaScriptImpl>(std::move(impl));
    }

    LuaInterface* ApiObjects::createLuaInterface(
        std::string_view source,
        const LuaConfigImpl& config,
        std::string_view interfaceName,
        ErrorReporting& errorReporting,
        bool verifyModules)
    {
        if (interfaceName.empty())
        {
            errorReporting.add("Can't create interface with empty name!", nullptr, EErrorType::IllegalArgument);
            return nullptr;
        }

        const ModuleMapping& modules = config.getModuleMapping();
        if (!checkLuaModules(modules, errorReporting))
            return nullptr;

        std::optional<LuaCompiledInterface> compiledInterface = LuaCompilationUtils::CompileInterface(
            *m_solState,
            modules,
            config.getStandardModules(),
            verifyModules,
            std::string{ source },
            interfaceName,
            errorReporting);

        if (!compiledInterface)
            return nullptr;

        auto impl = std::make_unique<LuaInterfaceImpl>(std::move(*compiledInterface), interfaceName, getNextLogicObjectId());
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

        auto impl = std::make_unique<LuaModuleImpl>(std::move(*compiledModule), moduleName, getNextLogicObjectId());
        return &createAndRegisterObject<LuaModule, LuaModuleImpl>(std::move(impl));
    }

    RamsesNodeBinding* ApiObjects::createRamsesNodeBinding(ramses::Node& ramsesNode, ramses::ERotationType rotationType, std::string_view name)
    {
        auto impl = std::make_unique<RamsesNodeBindingImpl>(ramsesNode, rotationType, name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<RamsesNodeBinding, RamsesNodeBindingImpl>(std::move(impl));
    }

    RamsesAppearanceBinding* ApiObjects::createRamsesAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name)
    {
        auto impl = std::make_unique<RamsesAppearanceBindingImpl>(ramsesAppearance, name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<RamsesAppearanceBinding, RamsesAppearanceBindingImpl>(std::move(impl));
    }

    RamsesCameraBinding* ApiObjects::createRamsesCameraBinding(ramses::Camera& ramsesCamera, bool withFrustumPlanes, std::string_view name)
    {
        auto impl = std::make_unique<RamsesCameraBindingImpl>(ramsesCamera, withFrustumPlanes, name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<RamsesCameraBinding, RamsesCameraBindingImpl>(std::move(impl));
    }

    RamsesRenderPassBinding* ApiObjects::createRamsesRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name)
    {
        auto impl = std::make_unique<RamsesRenderPassBindingImpl>(ramsesRenderPass, name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<RamsesRenderPassBinding, RamsesRenderPassBindingImpl>(std::move(impl));
    }

    RamsesRenderGroupBinding* ApiObjects::createRamsesRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RamsesRenderGroupBindingElements& elements, std::string_view name)
    {
        auto impl = std::make_unique<RamsesRenderGroupBindingImpl>(ramsesRenderGroup, *elements.m_impl, name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<RamsesRenderGroupBinding, RamsesRenderGroupBindingImpl>(std::move(impl));
    }

    RamsesMeshNodeBinding* ApiObjects::createRamsesMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name)
    {
        auto impl = std::make_unique<RamsesMeshNodeBindingImpl>(ramsesMeshNode, name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<RamsesMeshNodeBinding, RamsesMeshNodeBindingImpl>(std::move(impl));
    }

    SkinBinding* ApiObjects::createSkinBinding(
        std::vector<const RamsesNodeBindingImpl*> joints,
        const std::vector<matrix44f>& inverseBindMatrices,
        RamsesAppearanceBindingImpl& appearanceBinding,
        const ramses::UniformInput& jointMatInput,
        std::string_view name)
    {
        auto impl = std::make_unique<SkinBindingImpl>(std::move(joints), inverseBindMatrices, appearanceBinding, jointMatInput, name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<SkinBinding, SkinBindingImpl>(std::move(impl));
    }

    template <typename T>
    DataArray* ApiObjects::createDataArray(const std::vector<T>& data, std::string_view name)
    {
        static_assert(CanPropertyTypeBeStoredInDataArray(PropertyTypeToEnum<T>::TYPE));
        // make copy of users data and move into data array
        std::vector<T> dataCopy = data;
        auto impl = std::make_unique<DataArrayImpl>(std::move(dataCopy), name, getNextLogicObjectId());
        return &createAndRegisterObject<DataArray, DataArrayImpl>(std::move(impl));
    }

    AnimationNode* ApiObjects::createAnimationNode(const AnimationNodeConfigImpl& config, std::string_view name)
    {
        auto impl = std::make_unique<AnimationNodeImpl>(config.getChannels(), config.getExposingOfChannelDataAsProperties(), name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<AnimationNode, AnimationNodeImpl>(std::move(impl));
    }

    TimerNode* ApiObjects::createTimerNode(std::string_view name)
    {
        auto impl = std::make_unique<TimerNodeImpl>(name, getNextLogicObjectId());
        impl->createRootProperties();
        return &createAndRegisterObject<TimerNode, TimerNodeImpl>(std::move(impl));
    }

    AnchorPoint* ApiObjects::createAnchorPoint(RamsesNodeBindingImpl& nodeBinding, RamsesCameraBindingImpl& cameraBinding, std::string_view name)
    {
        auto impl = std::make_unique<AnchorPointImpl>(nodeBinding, cameraBinding, name, getNextLogicObjectId());
        impl->createRootProperties();
        auto& anchor = createAndRegisterObject<AnchorPoint, AnchorPointImpl>(std::move(impl));

        m_logicNodeDependencies.addBindingDependency(nodeBinding, anchor.m_impl);
        m_logicNodeDependencies.addBindingDependency(cameraBinding, anchor.m_impl);

        return &anchor;
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

        auto ramsesNodeBinding = dynamic_cast<RamsesNodeBinding*>(&object);
        if (ramsesNodeBinding)
            return destroyInternal(*ramsesNodeBinding, errorReporting);

        auto ramsesAppearanceBinding = dynamic_cast<RamsesAppearanceBinding*>(&object);
        if (ramsesAppearanceBinding)
            return destroyInternal(*ramsesAppearanceBinding, errorReporting);

        auto ramsesCameraBinding = dynamic_cast<RamsesCameraBinding*>(&object);
        if (ramsesCameraBinding)
            return destroyInternal(*ramsesCameraBinding, errorReporting);

        auto ramsesRenderPassBinding = dynamic_cast<RamsesRenderPassBinding*>(&object);
        if (ramsesRenderPassBinding)
            return destroyAndUnregisterObject(*ramsesRenderPassBinding, errorReporting);

        auto ramsesRenderGroupBinding = dynamic_cast<RamsesRenderGroupBinding*>(&object);
        if (ramsesRenderGroupBinding)
            return destroyAndUnregisterObject(*ramsesRenderGroupBinding, errorReporting);

        auto ramsesMeshNodeBinding = dynamic_cast<RamsesMeshNodeBinding*>(&object);
        if (ramsesMeshNodeBinding)
            return destroyAndUnregisterObject(*ramsesMeshNodeBinding, errorReporting);

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

        errorReporting.add(fmt::format("Tried to destroy object '{}' with unknown type", object.getName()), &object, EErrorType::IllegalArgument);

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
                    errorReporting.add(fmt::format("Failed to destroy data array '{}', it is used in animation node '{}' channel '{}'", dataArray.getName(), animNode->getName(), channel.name), &dataArray, EErrorType::IllegalArgument);
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
            for (const auto& moduleInUse : script->m_script.getModules())
            {
                if (moduleInUse.second == &luaModule)
                {
                    errorReporting.add(fmt::format("Failed to destroy LuaModule '{}', it is used in LuaScript '{}'", luaModule.getName(), script->getName()), &luaModule, EErrorType::IllegalArgument);
                    return false;
                }
            }
        }

        return destroyAndUnregisterObject(luaModule, errorReporting);
    }

    bool ApiObjects::destroyInternal(RamsesNodeBinding& ramsesNodeBinding, ErrorReporting& errorReporting)
    {
        for (const auto& anchor : m_anchorPoints)
        {
            if (anchor->m_anchorPointImpl.getRamsesNodeBinding().getId() == ramsesNodeBinding.getId())
            {
                errorReporting.add(fmt::format("Failed to destroy Ramses node binding '{}', it is used in anchor point '{}'", ramsesNodeBinding.getName(), anchor->getName()), &ramsesNodeBinding, EErrorType::Other);
                return false;
            }
        }

        for (const auto& skin : m_skinBindings)
        {
            for (const auto node : skin->m_skinBinding.getJoints())
            {
                if (node->getId() == ramsesNodeBinding.getId())
                {
                    errorReporting.add(fmt::format("Failed to destroy Ramses node binding '{}', it is used in skin binding '{}'", ramsesNodeBinding.getName(), skin->getName()), &ramsesNodeBinding, EErrorType::Other);
                    return false;
                }
            }
        }

        return destroyAndUnregisterObject(ramsesNodeBinding, errorReporting);
    }

    bool ApiObjects::destroyInternal(RamsesAppearanceBinding& ramsesAppearanceBinding, ErrorReporting& errorReporting)
    {
        for (const auto& skin : m_skinBindings)
        {
            if (skin->m_skinBinding.getAppearanceBinding().getId() == ramsesAppearanceBinding.getId())
            {
                errorReporting.add(fmt::format("Failed to destroy Ramses appearance binding '{}', it is used in skin binding '{}'", ramsesAppearanceBinding.getName(), skin->getName()), &ramsesAppearanceBinding, EErrorType::Other);
                return false;
            }
        }

        return destroyAndUnregisterObject(ramsesAppearanceBinding, errorReporting);
    }

    bool ApiObjects::destroyInternal(RamsesCameraBinding& ramsesCameraBinding, ErrorReporting& errorReporting)
    {
        for (const auto& anchor : m_anchorPoints)
        {
            if (anchor->m_anchorPointImpl.getRamsesCameraBinding().getId() == ramsesCameraBinding.getId())
            {
                errorReporting.add(fmt::format("Failed to destroy Ramses camera binding '{}', it is used in anchor point '{}'", ramsesCameraBinding.getName(), anchor->getName()), &ramsesCameraBinding, EErrorType::Other);
                return false;
            }
        }

        return destroyAndUnregisterObject(ramsesCameraBinding, errorReporting);
    }

    bool ApiObjects::destroyInternal(AnchorPoint& node, ErrorReporting& errorReporting)
    {
        if (std::find(m_anchorPoints.cbegin(), m_anchorPoints.cend(), &node) != m_anchorPoints.end())
        {
            m_logicNodeDependencies.removeBindingDependency(node.m_anchorPointImpl.getRamsesNodeBinding(), node.m_impl);
            m_logicNodeDependencies.removeBindingDependency(node.m_anchorPointImpl.getRamsesCameraBinding(), node.m_impl);
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
        objRaw.LogicObject::m_impl->setLogicObject(objRaw);

        // store in general pool of logic objects
        assert(std::find(m_logicObjects.cbegin(), m_logicObjects.cend(), &objRaw) == m_logicObjects.cend());
        m_logicObjects.push_back(&objRaw);
        // store in map to retrieve object by ID
        assert(m_logicObjectIdMapping.count(objRaw.getId()) == 0u);
        m_logicObjectIdMapping.emplace(objRaw.getId(), &objRaw);

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
            else if constexpr (std::is_same_v<RamsesNodeBinding, T>)
            {
                this->m_ramsesNodeBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<RamsesAppearanceBinding, T>)
            {
                this->m_ramsesAppearanceBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<RamsesCameraBinding, T>)
            {
                this->m_ramsesCameraBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<RamsesRenderPassBinding, T>)
            {
                this->m_ramsesRenderPassBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<RamsesRenderGroupBinding, T>)
            {
                this->m_ramsesRenderGroupBindings.push_back(&objRaw);
            }
            else if constexpr (std::is_same_v<RamsesMeshNodeBinding, T>)
            {
                this->m_ramsesMeshNodeBindings.push_back(&objRaw);
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
            errorReporting.add(fmt::format("Failed to destroy object '{}', cannot find it in this LogicEngine instance.", objToDelete.LogicObject::m_impl->getIdentificationString()), &objToDelete, EErrorType::IllegalArgument);
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
            else if constexpr (std::is_same_v<RamsesNodeBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_ramsesNodeBindings);
            }
            else if constexpr (std::is_same_v<RamsesAppearanceBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_ramsesAppearanceBindings);
            }
            else if constexpr (std::is_same_v<RamsesCameraBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_ramsesCameraBindings);
            }
            else if constexpr (std::is_same_v<RamsesRenderPassBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_ramsesRenderPassBindings);
            }
            else if constexpr (std::is_same_v<RamsesRenderGroupBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_ramsesRenderGroupBindings);
            }
            else if constexpr (std::is_same_v<RamsesMeshNodeBinding, T>)
            {
                eraseFromPool(objToDelete, this->m_ramsesMeshNodeBindings);
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
            else
            {
                assert(!"unhandled type");
            }
        }

        assert(m_logicObjectIdMapping.count(objToDelete.getId()) != 0u);
        m_logicObjectIdMapping.erase(objToDelete.getId());

        const auto findLogicObj = std::find(m_logicObjects.cbegin(), m_logicObjects.cend(), &objToDelete);
        assert(findLogicObj != m_logicObjects.cend() && "Can't find LogicObject in logic objects!");
        m_logicObjects.erase(findLogicObj);

        m_objectsOwningContainer.erase(findOwnedObj);

        return true;
    }

    bool ApiObjects::checkBindingsReferToSameRamsesScene(ErrorReporting& errorReporting) const
    {
        // Optional because it's OK that no Ramses object is referenced at all (and thus no ramses scene)
        std::optional<ramses::sceneId_t> sceneId;

        for (const auto& binding : m_ramsesNodeBindings)
        {
            const ramses::Node& node = binding->m_nodeBinding.getRamsesNode();
            const ramses::sceneId_t nodeSceneId = node.getSceneId();
            if (!sceneId)
            {
                sceneId = nodeSceneId;
            }

            if (*sceneId != nodeSceneId)
            {
                errorReporting.add(fmt::format("Ramses node '{}' is from scene with id:{} but other objects are from scene with id:{}!",
                    node.getName(), nodeSceneId.getValue(), sceneId->getValue()), binding, EErrorType::IllegalArgument);
                return false;
            }
        }

        for (const auto& binding : m_ramsesAppearanceBindings)
        {
            const ramses::Appearance& appearance = binding->m_appearanceBinding.getRamsesAppearance();
            const ramses::sceneId_t appearanceSceneId = appearance.getSceneId();
            if (!sceneId)
            {
                sceneId = appearanceSceneId;
            }

            if (*sceneId != appearanceSceneId)
            {
                errorReporting.add(fmt::format("Ramses appearance '{}' is from scene with id:{} but other objects are from scene with id:{}!",
                    appearance.getName(), appearanceSceneId.getValue(), sceneId->getValue()), binding, EErrorType::IllegalArgument);
                return false;
            }
        }

        for (const auto& binding : m_ramsesCameraBindings)
        {
            const ramses::Camera& camera = binding->m_cameraBinding.getRamsesCamera();
            const ramses::sceneId_t cameraSceneId = camera.getSceneId();
            if (!sceneId)
            {
                sceneId = cameraSceneId;
            }

            if (*sceneId != cameraSceneId)
            {
                errorReporting.add(fmt::format("Ramses camera '{}' is from scene with id:{} but other objects are from scene with id:{}!",
                    camera.getName(), cameraSceneId.getValue(), sceneId->getValue()), binding, EErrorType::IllegalArgument);
                return false;
            }
        }

        for (const auto& binding : m_ramsesRenderPassBindings)
        {
            const ramses::RenderPass& rp = binding->m_renderPassBinding.getRamsesRenderPass();
            const ramses::sceneId_t rpSceneId = rp.getSceneId();
            if (!sceneId)
                sceneId = rpSceneId;

            if (*sceneId != rpSceneId)
            {
                errorReporting.add(fmt::format("Ramses render pass '{}' is from scene with id:{} but other objects are from scene with id:{}!",
                    rp.getName(), rpSceneId.getValue(), sceneId->getValue()), binding, EErrorType::IllegalArgument);
                return false;
            }
        }

        for (const auto& binding : m_ramsesRenderGroupBindings)
        {
            const auto& rg = binding->m_renderGroupBinding.getRamsesRenderGroup();
            const ramses::sceneId_t rgSceneId = rg.getSceneId();
            if (!sceneId)
                sceneId = rgSceneId;

            if (*sceneId != rgSceneId)
            {
                errorReporting.add(fmt::format("Ramses render group '{}' is from scene with id:{} but other objects are from scene with id:{}!",
                    rg.getName(), rgSceneId.getValue(), sceneId->getValue()), binding, EErrorType::IllegalArgument);
                return false;
            }
        }

        for (const auto& binding : m_ramsesMeshNodeBindings)
        {
            const auto& mn = binding->m_meshNodeBinding.getRamsesMeshNode();
            const ramses::sceneId_t mnSceneId = mn.getSceneId();
            if (!sceneId)
                sceneId = mnSceneId;

            if (*sceneId != mnSceneId)
            {
                errorReporting.add(fmt::format("Ramses mesh node '{}' is from scene with id:{} but other objects are from scene with id:{}!",
                    mn.getName(), mnSceneId.getValue(), sceneId->getValue()), binding, EErrorType::IllegalArgument);
                return false;
            }
        }

        return true;
    }

    void ApiObjects::validateInterfaces(ValidationResults& validationResults) const
    {
        // check if there are any outputs without link
        // Note: this is different from the check for dangling nodes/content, where nodes are checked if they have ANY
        // linked inputs and/or outputs depending on node type. Interfaces on the other hand must have ALL of their outputs
        // linked to be valid.

        for (const auto* intf : m_interfaces)
        {
            std::vector<const Property*> unlinkedProperties = intf->m_interface.collectUnlinkedProperties();
            for (const auto* output : unlinkedProperties)
                validationResults.add(::fmt::format("Interface [{}] has unlinked output [{}]", intf->getName(), output->getName()), intf, EWarningType::UnusedContent);
        }

        // check if there are any name conflicts
        auto interfacesByName = m_interfaces;
        std::sort(interfacesByName.begin(), interfacesByName.end(), [](const auto i1, const auto i2) { return i1->getName() < i2->getName(); });
        const auto duplicateIt = std::adjacent_find(interfacesByName.cbegin(), interfacesByName.cend(), [](const auto i1, const auto i2) { return i1->getName() == i2->getName(); });
        if (duplicateIt != interfacesByName.cend())
            validationResults.add(fmt::format("Interface [{}] does not have a unique name", (*duplicateIt)->getName()), *duplicateIt, EWarningType::Other);
    }

    void ApiObjects::validateDanglingNodes(ValidationResults& validationResults) const
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
                    for (const auto* output : objAsNode->getOutputs()->m_impl->collectLeafChildren())
                        anyOutputLinked |= (output->hasOutgoingLink());

                    if (!anyOutputLinked)
                        validationResults.add(fmt::format("Node [{}] has no outgoing links! Node should be deleted or properly linked!", objAsNode->getName()), objAsNode, EWarningType::UnusedContent);
                }
            }
        }

        // collect node bindings used in anchor points and skin bindings
        // - these are allowed to have no incoming links because they are being read from
        std::unordered_set<const RamsesBinding*> bindingsInUse;
        for (const auto* anchor : m_anchorPoints)
        {
            bindingsInUse.insert(anchor->m_anchorPointImpl.getRamsesNodeBinding().getLogicObject().as<RamsesBinding>());
            bindingsInUse.insert(anchor->m_anchorPointImpl.getRamsesCameraBinding().getLogicObject().as<RamsesBinding>());
        }
        for (const auto* skin : m_skinBindings)
        {
            bindingsInUse.insert(skin->m_skinBinding.getAppearanceBinding().getLogicObject().as<RamsesBinding>());
            for (const auto* joint : skin->m_skinBinding.getJoints())
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
                    for (const auto* input : objAsNode->getInputs()->m_impl->collectLeafChildren())
                        anyInputLinked |= (input->hasIncomingLink());

                    if (!anyInputLinked)
                        validationResults.add(fmt::format("Node [{}] has no ingoing links! Node should be deleted or properly linked!", objAsNode->getName()), objAsNode, EWarningType::UnusedContent);
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
        else if constexpr (std::is_same_v<T, RamsesNodeBinding>)
        {
            return m_ramsesNodeBindings;
        }
        else if constexpr (std::is_same_v<T, RamsesAppearanceBinding>)
        {
            return m_ramsesAppearanceBindings;
        }
        else if constexpr (std::is_same_v<T, RamsesCameraBinding>)
        {
            return m_ramsesCameraBindings;
        }
        else if constexpr (std::is_same_v<T, RamsesRenderPassBinding>)
        {
            return m_ramsesRenderPassBindings;
        }
        else if constexpr (std::is_same_v<T, RamsesRenderGroupBinding>)
        {
            return m_ramsesRenderGroupBindings;
        }
        else if constexpr (std::is_same_v<T, RamsesMeshNodeBinding>)
        {
            return m_ramsesMeshNodeBindings;
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

    LogicObject* ApiObjects::getApiObjectById(uint64_t id) const
    {
        auto apiObjectIter = m_logicObjectIdMapping.find(id);
        if (apiObjectIter != m_logicObjectIdMapping.end())
        {
            assert(apiObjectIter->second->getId() == id);
            return apiObjectIter->second;
        }
        return nullptr;
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
                return LuaScriptImpl::Serialize(it->m_script, builder, serializationMap, luaSavingMode);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::LuaInterface>> luaInterfaces;
        luascripts.reserve(apiObjects.m_interfaces.size());
        std::transform(apiObjects.m_interfaces.cbegin(), apiObjects.m_interfaces.cend(), std::back_inserter(luaInterfaces),
            [&builder, &serializationMap](const std::vector<LuaInterface*>::value_type& it) {
                return LuaInterfaceImpl::Serialize(it->m_interface, builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::RamsesNodeBinding>> ramsesnodebindings;
        ramsesnodebindings.reserve(apiObjects.m_ramsesNodeBindings.size());
        std::transform(apiObjects.m_ramsesNodeBindings.begin(),
            apiObjects.m_ramsesNodeBindings.end(),
            std::back_inserter(ramsesnodebindings),
            [&builder, &serializationMap](const std::vector<RamsesNodeBinding*>::value_type& it) {
                return RamsesNodeBindingImpl::Serialize(it->m_nodeBinding, builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::RamsesAppearanceBinding>> ramsesappearancebindings;
        ramsesappearancebindings.reserve(apiObjects.m_ramsesAppearanceBindings.size());
        std::transform(apiObjects.m_ramsesAppearanceBindings.begin(),
            apiObjects.m_ramsesAppearanceBindings.end(),
            std::back_inserter(ramsesappearancebindings),
            [&builder, &serializationMap](const std::vector<RamsesAppearanceBinding*>::value_type& it) {
                return RamsesAppearanceBindingImpl::Serialize(it->m_appearanceBinding, builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::RamsesCameraBinding>> ramsescamerabindings;
        ramsescamerabindings.reserve(apiObjects.m_ramsesCameraBindings.size());
        std::transform(apiObjects.m_ramsesCameraBindings.begin(),
            apiObjects.m_ramsesCameraBindings.end(),
            std::back_inserter(ramsescamerabindings),
            [&builder, &serializationMap](const std::vector<RamsesCameraBinding*>::value_type& it) {
                return RamsesCameraBindingImpl::Serialize(it->m_cameraBinding, builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderPassBinding>> ramsesrenderpassbindings;
        ramsesrenderpassbindings.reserve(apiObjects.m_ramsesRenderPassBindings.size());
        std::transform(apiObjects.m_ramsesRenderPassBindings.begin(),
            apiObjects.m_ramsesRenderPassBindings.end(),
            std::back_inserter(ramsesrenderpassbindings),
            [&builder, &serializationMap](const std::vector<RamsesRenderPassBinding*>::value_type& it) {
                return RamsesRenderPassBindingImpl::Serialize(it->m_renderPassBinding, builder, serializationMap);
            });

        std::vector<flatbuffers::Offset<rlogic_serialization::DataArray>> dataArrays;
        dataArrays.reserve(apiObjects.m_dataArrays.size());
        for (const auto& da : apiObjects.m_dataArrays)
        {
            dataArrays.push_back(DataArrayImpl::Serialize(da->m_impl, builder, serializationMap));
            serializationMap.storeDataArray(da->getId(), dataArrays.back());
        }

        // animation nodes must go after data arrays because they reference them
        std::vector<flatbuffers::Offset<rlogic_serialization::AnimationNode>> animationNodes;
        animationNodes.reserve(apiObjects.m_animationNodes.size());
        for (const auto& animNode : apiObjects.m_animationNodes)
            animationNodes.push_back(AnimationNodeImpl::Serialize(animNode->m_animationNodeImpl, builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::TimerNode>> timerNodes;
        timerNodes.reserve(apiObjects.m_timerNodes.size());
        for (const auto& timerNode : apiObjects.m_timerNodes)
            timerNodes.push_back(TimerNodeImpl::Serialize(timerNode->m_timerNodeImpl, builder, serializationMap));

        // anchor points must go after node and camera bindings because they reference them
        std::vector<flatbuffers::Offset<rlogic_serialization::AnchorPoint>> anchorPoints;
        anchorPoints.reserve(apiObjects.m_anchorPoints.size());
        for (const auto& anchorPoint : apiObjects.m_anchorPoints)
            anchorPoints.push_back(AnchorPointImpl::Serialize(anchorPoint->m_anchorPointImpl, builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::RamsesRenderGroupBinding>> ramsesRenderGroupBindings;
        ramsesRenderGroupBindings.reserve(apiObjects.m_ramsesRenderGroupBindings.size());
        for (const auto& rgBinding : apiObjects.m_ramsesRenderGroupBindings)
            ramsesRenderGroupBindings.push_back(RamsesRenderGroupBindingImpl::Serialize(rgBinding->m_renderGroupBinding, builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::RamsesMeshNodeBinding>> ramsesMeshNodeBindings;
        ramsesMeshNodeBindings.reserve(apiObjects.m_ramsesMeshNodeBindings.size());
        for (const auto& mnBinding : apiObjects.m_ramsesMeshNodeBindings)
            ramsesMeshNodeBindings.push_back(RamsesMeshNodeBindingImpl::Serialize(mnBinding->m_meshNodeBinding, builder, serializationMap));

        std::vector<flatbuffers::Offset<rlogic_serialization::SkinBinding>> skinBindings;
        skinBindings.reserve(apiObjects.m_skinBindings.size());
        for (const auto& skinBinding : apiObjects.m_skinBindings)
            skinBindings.push_back(SkinBindingImpl::Serialize(skinBinding->m_skinBinding, builder, serializationMap));

        // links must go last due to dependency on serialized properties
        const auto collectedLinks = apiObjects.collectPropertyLinks();
        std::vector<flatbuffers::Offset<rlogic_serialization::Link>> links;
        links.reserve(collectedLinks.size());
        for (const auto& link : collectedLinks)
        {
            links.push_back(rlogic_serialization::CreateLink(builder,
                serializationMap.resolvePropertyOffset(*link.source->m_impl),
                serializationMap.resolvePropertyOffset(*link.target->m_impl),
                link.isWeakLink));
        }

        const auto fbModules = builder.CreateVector(luaModules);
        const auto fbScripts = builder.CreateVector(luascripts);
        const auto fbInterfaces = builder.CreateVector(luaInterfaces);
        const auto fbNodeBindings = builder.CreateVector(ramsesnodebindings);
        const auto fbAppearanceBindings = builder.CreateVector(ramsesappearancebindings);
        const auto fbCameraBindings = builder.CreateVector(ramsescamerabindings);
        const auto fbDataArrays = builder.CreateVector(dataArrays);
        const auto fbAnimations = builder.CreateVector(animationNodes);
        const auto fbTimers = builder.CreateVector(timerNodes);
        const auto fbLinks = builder.CreateVector(links);
        const auto fbRenderPasses = builder.CreateVector(ramsesrenderpassbindings);
        const auto fbAnchorPoints = builder.CreateVector(anchorPoints);
        const auto fbRenderGroupBindings = builder.CreateVector(ramsesRenderGroupBindings);
        const auto fbMeshNodeBindings = builder.CreateVector(ramsesMeshNodeBindings);
        const auto fbSkinBindings = builder.CreateVector(skinBindings);

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
            apiObjects.m_lastObjectId,
            fbRenderPasses,
            fbAnchorPoints,
            fbRenderGroupBindings,
            fbSkinBindings,
            fbMeshNodeBindings
            );

        builder.Finish(logicEngine);

        return logicEngine;
    }

    std::unique_ptr<ApiObjects> ApiObjects::Deserialize(
        const rlogic_serialization::ApiObjects& apiObjects,
        const IRamsesObjectResolver* ramsesResolver,
        const std::string& dataSourceDescription,
        ErrorReporting& errorReporting,
        ramses::EFeatureLevel featureLevel)
    {
        // Collect data here, only return if no error occurred
        auto deserialized = std::make_unique<ApiObjects>(featureLevel);

        // Collect deserialized object mappings to resolve dependencies
        DeserializationMap deserializationMap;

        if (!apiObjects.luaModules())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing Lua modules container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.luaScripts())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing Lua scripts container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.luaInterfaces())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing Lua interfaces container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.nodeBindings())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing node bindings container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.appearanceBindings())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing appearance bindings container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.cameraBindings())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing camera bindings container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.renderPassBindings())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing renderpass bindings container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.links())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing links container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.dataArrays())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing data arrays container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.animationNodes())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing animation nodes container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.timerNodes())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing timer nodes container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.anchorPoints())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing anchor points container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.renderGroupBindings())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing rendergroup bindings container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.meshNodeBindings())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing meshnode bindings container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        if (!apiObjects.skinBindings())
        {
            errorReporting.add("Fatal error during loading from serialized data: missing skin bindings container!", nullptr, EErrorType::BinaryVersionMismatch);
            return nullptr;
        }

        deserialized->m_lastObjectId = apiObjects.lastObjectId();

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
            static_cast<size_t>(apiObjects.skinBindings()->size());

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
            deserializationMap.storeLogicObject(obj.getId(), obj.m_impl);
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

        if (apiObjects.nodeBindings()->size() != 0u ||
            apiObjects.appearanceBindings()->size() != 0u ||
            apiObjects.cameraBindings()->size() != 0u ||
            apiObjects.renderPassBindings()->size() != 0u ||
            apiObjects.renderGroupBindings()->size() != 0u ||
            apiObjects.meshNodeBindings()->size() != 0u)
        {
            if (ramsesResolver == nullptr)
            {
                errorReporting.add("Fatal error during loading from file! File contains references to Ramses objects but no Ramses scene was provided!", nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }
        }

        const auto& ramsesNodeBindings = *apiObjects.nodeBindings();
        deserialized->m_ramsesNodeBindings.reserve(ramsesNodeBindings.size());
        for (const auto* binding : ramsesNodeBindings)
        {
            assert(binding);
            assert(ramsesResolver);
            std::unique_ptr<RamsesNodeBindingImpl> deserializedBinding = RamsesNodeBindingImpl::Deserialize(*binding, *ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<RamsesNodeBinding, RamsesNodeBindingImpl>(std::move(deserializedBinding));
            deserializationMap.storeLogicObject(obj.getId(), obj.m_impl);
        }

        const auto& ramsesAppearanceBindings = *apiObjects.appearanceBindings();
        deserialized->m_ramsesAppearanceBindings.reserve(ramsesAppearanceBindings.size());
        for (const auto* binding : ramsesAppearanceBindings)
        {
            assert(binding);
            assert(ramsesResolver);
            std::unique_ptr<RamsesAppearanceBindingImpl> deserializedBinding = RamsesAppearanceBindingImpl::Deserialize(*binding, *ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<RamsesAppearanceBinding, RamsesAppearanceBindingImpl>(std::move(deserializedBinding));
            deserializationMap.storeLogicObject(obj.getId(), obj.m_impl);
        }

        const auto& ramsesCameraBindings = *apiObjects.cameraBindings();
        deserialized->m_ramsesCameraBindings.reserve(ramsesCameraBindings.size());
        for (const auto* binding : ramsesCameraBindings)
        {
            assert(binding);
            assert(ramsesResolver);
            std::unique_ptr<RamsesCameraBindingImpl> deserializedBinding = RamsesCameraBindingImpl::Deserialize(*binding, *ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<RamsesCameraBinding, RamsesCameraBindingImpl>(std::move(deserializedBinding));
            deserializationMap.storeLogicObject(obj.getId(), obj.m_impl);
        }

        const auto& ramsesRenderPassBindings = *apiObjects.renderPassBindings();
        deserialized->m_ramsesRenderPassBindings.reserve(ramsesRenderPassBindings.size());
        for (const auto* binding : ramsesRenderPassBindings)
        {
            assert(binding);
            assert(ramsesResolver);
            std::unique_ptr<RamsesRenderPassBindingImpl> deserializedBinding = RamsesRenderPassBindingImpl::Deserialize(*binding, *ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            auto& obj = deserialized->createAndRegisterObject<RamsesRenderPassBinding, RamsesRenderPassBindingImpl>(std::move(deserializedBinding));
            deserializationMap.storeLogicObject(obj.getId(), obj.m_impl);
        }

        const auto& dataArrays = *apiObjects.dataArrays();
        deserialized->m_dataArrays.reserve(dataArrays.size());
        for (const auto* fbData : dataArrays)
        {
            assert(fbData);
            auto deserializedDataArray = DataArrayImpl::Deserialize(*fbData, errorReporting);
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
            deserializationMap.storeLogicObject(obj.getId(), obj.m_impl);
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
            assert(ramsesResolver);
            std::unique_ptr<AnchorPointImpl> deserializedAnchor = AnchorPointImpl::Deserialize(*fbAnchor, errorReporting, deserializationMap);
            if (!deserializedAnchor)
                return nullptr;

            deserialized->createAndRegisterObject<AnchorPoint, AnchorPointImpl>(std::move(deserializedAnchor));
        }

        const auto& ramsesRenderGroupBindings = *apiObjects.renderGroupBindings();
        deserialized->m_ramsesRenderGroupBindings.reserve(ramsesRenderGroupBindings.size());
        for (const auto* binding : ramsesRenderGroupBindings)
        {
            assert(binding);
            assert(ramsesResolver);
            std::unique_ptr<RamsesRenderGroupBindingImpl> deserializedBinding = RamsesRenderGroupBindingImpl::Deserialize(*binding, *ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            deserialized->createAndRegisterObject<RamsesRenderGroupBinding, RamsesRenderGroupBindingImpl>(std::move(deserializedBinding));
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

        const auto& ramsesMeshNodeBindings = *apiObjects.meshNodeBindings();
        deserialized->m_ramsesMeshNodeBindings.reserve(ramsesMeshNodeBindings.size());
        for (const auto* binding : ramsesMeshNodeBindings)
        {
            assert(binding);
            assert(ramsesResolver);
            std::unique_ptr<RamsesMeshNodeBindingImpl> deserializedBinding = RamsesMeshNodeBindingImpl::Deserialize(*binding, *ramsesResolver, errorReporting, deserializationMap);
            if (!deserializedBinding)
                return nullptr;

            deserialized->createAndRegisterObject<RamsesMeshNodeBinding, RamsesMeshNodeBindingImpl>(std::move(deserializedBinding));
        }

        // links must go last due to dependency on deserialized properties
        const auto& links = *apiObjects.links();
        // TODO Violin move this code (serialization parts too) to LogicNodeDependencies
        for (const auto* rLink : links)
        {
            assert(rLink);

            if (!rLink->sourceProperty())
            {
                errorReporting.add("Fatal error during loading from serialized data: missing link source property!", nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }

            if (!rLink->targetProperty())
            {
                errorReporting.add("Fatal error during loading from serialized data: missing link target property!", nullptr, EErrorType::BinaryVersionMismatch);
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
                errorReporting.add(
                    fmt::format("Fatal error during loading from {}! Could not link property '{}' to property '{}'!",
                        dataSourceDescription,
                        sourceProp->name()->string_view(),
                        targetProp->name()->string_view()
                    ), nullptr, EErrorType::BinaryVersionMismatch);
                return nullptr;
            }
        }

        return deserialized;
    }

    bool ApiObjects::bindingsDirty() const
    {
        return
            std::any_of(m_ramsesNodeBindings.cbegin(), m_ramsesNodeBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_ramsesAppearanceBindings.cbegin(), m_ramsesAppearanceBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_ramsesCameraBindings.cbegin(), m_ramsesCameraBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_ramsesRenderPassBindings.cbegin(), m_ramsesRenderPassBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_ramsesRenderGroupBindings.cbegin(), m_ramsesRenderGroupBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_ramsesMeshNodeBindings.cbegin(), m_ramsesMeshNodeBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); }) ||
            std::any_of(m_skinBindings.cbegin(), m_skinBindings.cend(), [](const auto& b) { return b->m_impl.isDirty(); });
    }

    uint64_t ApiObjects::getNextLogicObjectId()
    {
        return ++m_lastObjectId;
    }

    int ApiObjects::getNumElementsInLuaStack() const
    {
        return m_solState->getNumElementsInLuaStack();
    }

    const std::vector<PropertyLink>& ApiObjects::getAllPropertyLinks() const
    {
        m_collectedLinks = collectPropertyLinks();
        return m_collectedLinks;
    }

    std::vector<PropertyLink> ApiObjects::collectPropertyLinks() const
    {
        std::vector<PropertyLink> links;

        std::deque<const Property*> propsStack;
        for (const auto& obj : m_logicObjects)
        {
            const auto logicNode = obj->as<LogicNode>();
            if (!logicNode)
                continue;

            propsStack.clear();
            propsStack.push_back(logicNode->getInputs());
            propsStack.push_back(logicNode->getOutputs());
            while (!propsStack.empty())
            {
                const auto prop = propsStack.back();
                propsStack.pop_back();
                if (prop == nullptr)
                    continue;

                const auto incomingLink = prop->getIncomingLink();
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

    template ApiObjectContainer<LogicObject>&              ApiObjects::getApiObjectContainer<LogicObject>();
    template ApiObjectContainer<LuaScript>&                ApiObjects::getApiObjectContainer<LuaScript>();
    template ApiObjectContainer<LuaInterface>&             ApiObjects::getApiObjectContainer<LuaInterface>();
    template ApiObjectContainer<LuaModule>&                ApiObjects::getApiObjectContainer<LuaModule>();
    template ApiObjectContainer<RamsesNodeBinding>&        ApiObjects::getApiObjectContainer<RamsesNodeBinding>();
    template ApiObjectContainer<RamsesAppearanceBinding>&  ApiObjects::getApiObjectContainer<RamsesAppearanceBinding>();
    template ApiObjectContainer<RamsesCameraBinding>&      ApiObjects::getApiObjectContainer<RamsesCameraBinding>();
    template ApiObjectContainer<RamsesRenderPassBinding>&  ApiObjects::getApiObjectContainer<RamsesRenderPassBinding>();
    template ApiObjectContainer<RamsesRenderGroupBinding>& ApiObjects::getApiObjectContainer<RamsesRenderGroupBinding>();
    template ApiObjectContainer<RamsesMeshNodeBinding>&    ApiObjects::getApiObjectContainer<RamsesMeshNodeBinding>();
    template ApiObjectContainer<SkinBinding>&              ApiObjects::getApiObjectContainer<SkinBinding>();
    template ApiObjectContainer<DataArray>&                ApiObjects::getApiObjectContainer<DataArray>();
    template ApiObjectContainer<AnimationNode>&            ApiObjects::getApiObjectContainer<AnimationNode>();
    template ApiObjectContainer<TimerNode>&                ApiObjects::getApiObjectContainer<TimerNode>();
    template ApiObjectContainer<AnchorPoint>&              ApiObjects::getApiObjectContainer<AnchorPoint>();

    template const ApiObjectContainer<LogicObject>&              ApiObjects::getApiObjectContainer<LogicObject>() const;
    template const ApiObjectContainer<LuaScript>&                ApiObjects::getApiObjectContainer<LuaScript>() const;
    template const ApiObjectContainer<LuaInterface>&             ApiObjects::getApiObjectContainer<LuaInterface>() const;
    template const ApiObjectContainer<LuaModule>&                ApiObjects::getApiObjectContainer<LuaModule>() const;
    template const ApiObjectContainer<RamsesNodeBinding>&        ApiObjects::getApiObjectContainer<RamsesNodeBinding>() const;
    template const ApiObjectContainer<RamsesAppearanceBinding>&  ApiObjects::getApiObjectContainer<RamsesAppearanceBinding>() const;
    template const ApiObjectContainer<RamsesCameraBinding>&      ApiObjects::getApiObjectContainer<RamsesCameraBinding>() const;
    template const ApiObjectContainer<RamsesRenderPassBinding>&  ApiObjects::getApiObjectContainer<RamsesRenderPassBinding>() const;
    template const ApiObjectContainer<RamsesRenderGroupBinding>& ApiObjects::getApiObjectContainer<RamsesRenderGroupBinding>() const;
    template const ApiObjectContainer<RamsesMeshNodeBinding>&    ApiObjects::getApiObjectContainer<RamsesMeshNodeBinding>() const;
    template const ApiObjectContainer<SkinBinding>&              ApiObjects::getApiObjectContainer<SkinBinding>() const;
    template const ApiObjectContainer<DataArray>&                ApiObjects::getApiObjectContainer<DataArray>() const;
    template const ApiObjectContainer<AnimationNode>&            ApiObjects::getApiObjectContainer<AnimationNode>() const;
    template const ApiObjectContainer<TimerNode>&                ApiObjects::getApiObjectContainer<TimerNode>() const;
    template const ApiObjectContainer<AnchorPoint>&              ApiObjects::getApiObjectContainer<AnchorPoint>() const;
}
