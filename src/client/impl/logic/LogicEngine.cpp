//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"

#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderPassBinding.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"

#include "impl/logic/LogicEngineImpl.h"
#include "impl/logic/LuaConfigImpl.h"
#include "internal/logic/ApiObjects.h"

#include <string>

namespace ramses
{
    LogicEngine::LogicEngine(std::unique_ptr<internal::LogicEngineImpl> impl)
        : SceneObject{ std::move(impl) }
        , m_impl{ static_cast<internal::LogicEngineImpl&>(SceneObject::m_impl) }
    {
    }

    internal::LogicEngineImpl& LogicEngine::impl()
    {
        return m_impl;
    }

    const internal::LogicEngineImpl& LogicEngine::impl() const
    {
        return m_impl;
    }

    template <typename T>
    Collection<T> LogicEngine::getLogicObjectsInternal() const
    {
        return Collection<T>(m_impl.getApiObjects().getApiObjectContainer<T>());
    }

    template <typename T>
    const T* LogicEngine::findLogicObjectInternal(std::string_view name) const
    {
        // const version of findLogicObjectInternal cast to its non-const version to avoid duplicating code
        return (const_cast<LogicEngine&>(*this)).findLogicObjectInternal<T>(name);
    }

    template <typename T>
    T* LogicEngine::findLogicObjectInternal(std::string_view name)
    {
        auto& container = m_impl.getApiObjects().getApiObjectContainer<T>();
        const auto it = std::find_if(container.begin(), container.end(), [name](const auto& o) {
            return o->getName() == name; });

        return (it == container.end() ? nullptr : *it);
    }

    template <typename T>
    const T* LogicEngine::findLogicObjectInternal(sceneObjectId_t id) const
    {
        // const version of findLogicObjectInternal cast to its non-const version to avoid duplicating code
        return (const_cast<LogicEngine&>(*this)).findLogicObjectInternal<T>(id);
    }

    template <typename T>
    T* LogicEngine::findLogicObjectInternal(sceneObjectId_t id)
    {
        auto& container = m_impl.getApiObjects().getApiObjectContainer<T>();
        const auto it = std::find_if(container.begin(), container.end(), [id](const auto& o) {
            return o->getSceneObjectId() == id; });

        return (it == container.end() ? nullptr : *it);
    }

    LuaScript* LogicEngine::createLuaScript(std::string_view source, const LuaConfig& config, std::string_view scriptName)
    {
        return m_impl.createLuaScript(source, config.impl(), scriptName);
    }

    LuaInterface* LogicEngine::createLuaInterface(std::string_view source, std::string_view interfaceName, const LuaConfig& config)
    {
        return m_impl.createLuaInterface(source, config.impl(), interfaceName);
    }

    LuaModule* LogicEngine::createLuaModule(std::string_view source, const LuaConfig& config, std::string_view moduleName)
    {
        return m_impl.createLuaModule(source, config.impl(), moduleName);
    }

    bool LogicEngine::extractLuaDependencies(std::string_view source, const std::function<void(const std::string&)>& callbackFunc)
    {
        return m_impl.extractLuaDependencies(source, callbackFunc);
    }

    NodeBinding* LogicEngine::createNodeBinding(Node& ramsesNode, ERotationType rotationType /* = ramses::ERotationType::Euler_XYZ*/, std::string_view name)
    {
        return m_impl.createNodeBinding(ramsesNode, rotationType, name);
    }

    bool LogicEngine::destroy(LogicObject& object)
    {
        return m_impl.destroy(object);
    }

    AppearanceBinding* LogicEngine::createAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name)
    {
        return m_impl.createAppearanceBinding(ramsesAppearance, name);
    }

    CameraBinding* LogicEngine::createCameraBinding(ramses::Camera& ramsesCamera, std::string_view name)
    {
        return m_impl.createCameraBinding(ramsesCamera, name);
    }

    CameraBinding* LogicEngine::createCameraBindingWithFrustumPlanes(ramses::Camera& ramsesCamera, std::string_view name)
    {
        return m_impl.createCameraBindingWithFrustumPlanes(ramsesCamera, name);
    }

    RenderPassBinding* LogicEngine::createRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name)
    {
        return m_impl.createRenderPassBinding(ramsesRenderPass, name);
    }

    RenderGroupBinding* LogicEngine::createRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RenderGroupBindingElements& elements, std::string_view name)
    {
        return m_impl.createRenderGroupBinding(ramsesRenderGroup, elements, name);
    }

    MeshNodeBinding* LogicEngine::createMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name)
    {
        return m_impl.createMeshNodeBinding(ramsesMeshNode, name);
    }

    SkinBinding* LogicEngine::createSkinBinding(
        const std::vector<const NodeBinding*>& joints,
        const std::vector<matrix44f>& inverseBindMatrices,
        AppearanceBinding& appearanceBinding,
        const UniformInput& jointMatInput,
        std::string_view name)
    {
        return m_impl.createSkinBinding(joints, inverseBindMatrices, appearanceBinding, jointMatInput, name);
    }

    template <typename T>
    DataArray* LogicEngine::createDataArrayInternal(const std::vector<T>& data, std::string_view name)
    {
        static_assert(CanPropertyTypeBeStoredInDataArray(PropertyTypeToEnum<T>::TYPE));
        return m_impl.createDataArray(data, name);
    }

    AnimationNode* LogicEngine::createAnimationNode(const AnimationNodeConfig& config, std::string_view name)
    {
        return m_impl.createAnimationNode(config, name);
    }

    TimerNode* LogicEngine::createTimerNode(std::string_view name)
    {
        return m_impl.createTimerNode(name);
    }

    AnchorPoint* LogicEngine::createAnchorPoint(NodeBinding& nodeBinding, CameraBinding& cameraBinding, std::string_view name)
    {
        return m_impl.createAnchorPoint(nodeBinding, cameraBinding, name);
    }

    bool LogicEngine::update()
    {
        return m_impl.update();
    }

    void LogicEngine::enableUpdateReport(bool enable)
    {
        m_impl.enableUpdateReport(enable);
    }

    LogicEngineReport LogicEngine::getLastUpdateReport() const
    {
        return m_impl.getLastUpdateReport();
    }

    void LogicEngine::setStatisticsLoggingRate(size_t loggingRate, EStatisticsLogMode mode)
    {
        m_impl.setStatisticsLoggingRate(loggingRate, mode);
    }

    bool LogicEngine::link(Property& sourceProperty, Property& targetProperty)
    {
        return m_impl.link(sourceProperty, targetProperty);
    }

    bool LogicEngine::linkWeak(Property& sourceProperty, Property& targetProperty)
    {
        return m_impl.linkWeak(sourceProperty, targetProperty);
    }

    bool LogicEngine::unlink(Property& sourceProperty, Property& targetProperty)
    {
        return m_impl.unlink(sourceProperty, targetProperty);
    }

    bool LogicEngine::isLinked(const LogicNode& logicNode) const
    {
        return m_impl.isLinked(logicNode);
    }

    size_t LogicEngine::getTotalSerializedSize(ELuaSavingMode luaSavingMode) const
    {
        return m_impl.getTotalSerializedSize(luaSavingMode);
    }

    template<typename T>
    size_t LogicEngine::getSerializedSizeInternal(ELuaSavingMode luaSavingMode) const
    {
        return m_impl.getSerializedSize<T>(luaSavingMode);
    }

    const std::vector<PropertyLinkConst>& LogicEngine::getPropertyLinks() const
    {
        return impl().getApiObjects().getAllPropertyLinks();
    }

    const std::vector<PropertyLink>& LogicEngine::getPropertyLinks()
    {
        return m_impl.getApiObjects().getAllPropertyLinks();
    }

    template RAMSES_API Collection<LogicObject>        LogicEngine::getLogicObjectsInternal<LogicObject>() const;
    template RAMSES_API Collection<LuaScript>          LogicEngine::getLogicObjectsInternal<LuaScript>() const;
    template RAMSES_API Collection<LuaModule>          LogicEngine::getLogicObjectsInternal<LuaModule>() const;
    template RAMSES_API Collection<LuaInterface>       LogicEngine::getLogicObjectsInternal<LuaInterface>() const;
    template RAMSES_API Collection<NodeBinding>        LogicEngine::getLogicObjectsInternal<NodeBinding>() const;
    template RAMSES_API Collection<AppearanceBinding>  LogicEngine::getLogicObjectsInternal<AppearanceBinding>() const;
    template RAMSES_API Collection<CameraBinding>      LogicEngine::getLogicObjectsInternal<CameraBinding>() const;
    template RAMSES_API Collection<RenderPassBinding>  LogicEngine::getLogicObjectsInternal<RenderPassBinding>() const;
    template RAMSES_API Collection<RenderGroupBinding> LogicEngine::getLogicObjectsInternal<RenderGroupBinding>() const;
    template RAMSES_API Collection<MeshNodeBinding>    LogicEngine::getLogicObjectsInternal<MeshNodeBinding>() const;
    template RAMSES_API Collection<SkinBinding>        LogicEngine::getLogicObjectsInternal<SkinBinding>() const;
    template RAMSES_API Collection<DataArray>          LogicEngine::getLogicObjectsInternal<DataArray>() const;
    template RAMSES_API Collection<AnimationNode>      LogicEngine::getLogicObjectsInternal<AnimationNode>() const;
    template RAMSES_API Collection<TimerNode>          LogicEngine::getLogicObjectsInternal<TimerNode>() const;
    template RAMSES_API Collection<AnchorPoint>        LogicEngine::getLogicObjectsInternal<AnchorPoint>() const;

    template RAMSES_API const LogicObject*        LogicEngine::findLogicObjectInternal<LogicObject>(std::string_view) const;
    template RAMSES_API const LuaScript*          LogicEngine::findLogicObjectInternal<LuaScript>(std::string_view) const;
    template RAMSES_API const LuaModule*          LogicEngine::findLogicObjectInternal<LuaModule>(std::string_view) const;
    template RAMSES_API const LuaInterface*       LogicEngine::findLogicObjectInternal<LuaInterface>(std::string_view) const;
    template RAMSES_API const NodeBinding*        LogicEngine::findLogicObjectInternal<NodeBinding>(std::string_view) const;
    template RAMSES_API const AppearanceBinding*  LogicEngine::findLogicObjectInternal<AppearanceBinding>(std::string_view) const;
    template RAMSES_API const CameraBinding*      LogicEngine::findLogicObjectInternal<CameraBinding>(std::string_view) const;
    template RAMSES_API const RenderPassBinding*  LogicEngine::findLogicObjectInternal<RenderPassBinding>(std::string_view) const;
    template RAMSES_API const RenderGroupBinding* LogicEngine::findLogicObjectInternal<RenderGroupBinding>(std::string_view) const;
    template RAMSES_API const MeshNodeBinding*    LogicEngine::findLogicObjectInternal<MeshNodeBinding>(std::string_view) const;
    template RAMSES_API const SkinBinding*        LogicEngine::findLogicObjectInternal<SkinBinding>(std::string_view) const;
    template RAMSES_API const DataArray*          LogicEngine::findLogicObjectInternal<DataArray>(std::string_view) const;
    template RAMSES_API const AnimationNode*      LogicEngine::findLogicObjectInternal<AnimationNode>(std::string_view) const;
    template RAMSES_API const TimerNode*          LogicEngine::findLogicObjectInternal<TimerNode>(std::string_view) const;
    template RAMSES_API const AnchorPoint*        LogicEngine::findLogicObjectInternal<AnchorPoint>(std::string_view) const;

    template RAMSES_API LogicObject*        LogicEngine::findLogicObjectInternal<LogicObject>(std::string_view);
    template RAMSES_API LuaScript*          LogicEngine::findLogicObjectInternal<LuaScript>(std::string_view);
    template RAMSES_API LuaModule*          LogicEngine::findLogicObjectInternal<LuaModule>(std::string_view);
    template RAMSES_API LuaInterface*       LogicEngine::findLogicObjectInternal<LuaInterface>(std::string_view);
    template RAMSES_API NodeBinding*        LogicEngine::findLogicObjectInternal<NodeBinding>(std::string_view);
    template RAMSES_API AppearanceBinding*  LogicEngine::findLogicObjectInternal<AppearanceBinding>(std::string_view);
    template RAMSES_API CameraBinding*      LogicEngine::findLogicObjectInternal<CameraBinding>(std::string_view);
    template RAMSES_API RenderPassBinding*  LogicEngine::findLogicObjectInternal<RenderPassBinding>(std::string_view);
    template RAMSES_API RenderGroupBinding* LogicEngine::findLogicObjectInternal<RenderGroupBinding>(std::string_view);
    template RAMSES_API MeshNodeBinding*    LogicEngine::findLogicObjectInternal<MeshNodeBinding>(std::string_view);
    template RAMSES_API SkinBinding*        LogicEngine::findLogicObjectInternal<SkinBinding>(std::string_view);
    template RAMSES_API DataArray*          LogicEngine::findLogicObjectInternal<DataArray>(std::string_view);
    template RAMSES_API AnimationNode*      LogicEngine::findLogicObjectInternal<AnimationNode>(std::string_view);
    template RAMSES_API TimerNode*          LogicEngine::findLogicObjectInternal<TimerNode>(std::string_view);
    template RAMSES_API AnchorPoint*        LogicEngine::findLogicObjectInternal<AnchorPoint>(std::string_view);

    template RAMSES_API const LogicObject*        LogicEngine::findLogicObjectInternal<LogicObject>(sceneObjectId_t) const;
    template RAMSES_API const LuaScript*          LogicEngine::findLogicObjectInternal<LuaScript>(sceneObjectId_t) const;
    template RAMSES_API const LuaModule*          LogicEngine::findLogicObjectInternal<LuaModule>(sceneObjectId_t) const;
    template RAMSES_API const LuaInterface*       LogicEngine::findLogicObjectInternal<LuaInterface>(sceneObjectId_t) const;
    template RAMSES_API const NodeBinding*        LogicEngine::findLogicObjectInternal<NodeBinding>(sceneObjectId_t) const;
    template RAMSES_API const AppearanceBinding*  LogicEngine::findLogicObjectInternal<AppearanceBinding>(sceneObjectId_t) const;
    template RAMSES_API const CameraBinding*      LogicEngine::findLogicObjectInternal<CameraBinding>(sceneObjectId_t) const;
    template RAMSES_API const RenderPassBinding*  LogicEngine::findLogicObjectInternal<RenderPassBinding>(sceneObjectId_t) const;
    template RAMSES_API const RenderGroupBinding* LogicEngine::findLogicObjectInternal<RenderGroupBinding>(sceneObjectId_t) const;
    template RAMSES_API const MeshNodeBinding*    LogicEngine::findLogicObjectInternal<MeshNodeBinding>(sceneObjectId_t) const;
    template RAMSES_API const SkinBinding*        LogicEngine::findLogicObjectInternal<SkinBinding>(sceneObjectId_t) const;
    template RAMSES_API const DataArray*          LogicEngine::findLogicObjectInternal<DataArray>(sceneObjectId_t) const;
    template RAMSES_API const AnimationNode*      LogicEngine::findLogicObjectInternal<AnimationNode>(sceneObjectId_t) const;
    template RAMSES_API const TimerNode*          LogicEngine::findLogicObjectInternal<TimerNode>(sceneObjectId_t) const;
    template RAMSES_API const AnchorPoint*        LogicEngine::findLogicObjectInternal<AnchorPoint>(sceneObjectId_t) const;

    template RAMSES_API LogicObject*        LogicEngine::findLogicObjectInternal<LogicObject>(sceneObjectId_t);
    template RAMSES_API LuaScript*          LogicEngine::findLogicObjectInternal<LuaScript>(sceneObjectId_t);
    template RAMSES_API LuaModule*          LogicEngine::findLogicObjectInternal<LuaModule>(sceneObjectId_t);
    template RAMSES_API LuaInterface*       LogicEngine::findLogicObjectInternal<LuaInterface>(sceneObjectId_t);
    template RAMSES_API NodeBinding*        LogicEngine::findLogicObjectInternal<NodeBinding>(sceneObjectId_t);
    template RAMSES_API AppearanceBinding*  LogicEngine::findLogicObjectInternal<AppearanceBinding>(sceneObjectId_t);
    template RAMSES_API CameraBinding*      LogicEngine::findLogicObjectInternal<CameraBinding>(sceneObjectId_t);
    template RAMSES_API RenderPassBinding*  LogicEngine::findLogicObjectInternal<RenderPassBinding>(sceneObjectId_t);
    template RAMSES_API RenderGroupBinding* LogicEngine::findLogicObjectInternal<RenderGroupBinding>(sceneObjectId_t);
    template RAMSES_API MeshNodeBinding*    LogicEngine::findLogicObjectInternal<MeshNodeBinding>(sceneObjectId_t);
    template RAMSES_API SkinBinding*        LogicEngine::findLogicObjectInternal<SkinBinding>(sceneObjectId_t);
    template RAMSES_API DataArray*          LogicEngine::findLogicObjectInternal<DataArray>(sceneObjectId_t);
    template RAMSES_API AnimationNode*      LogicEngine::findLogicObjectInternal<AnimationNode>(sceneObjectId_t);
    template RAMSES_API TimerNode*          LogicEngine::findLogicObjectInternal<TimerNode>(sceneObjectId_t);
    template RAMSES_API AnchorPoint*        LogicEngine::findLogicObjectInternal<AnchorPoint>(sceneObjectId_t);

    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<float>(const std::vector<float>&, std::string_view);
    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<vec2f>(const std::vector<vec2f>&, std::string_view);
    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<vec3f>(const std::vector<vec3f>&, std::string_view);
    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<vec4f>(const std::vector<vec4f>&, std::string_view);
    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<int32_t>(const std::vector<int32_t>&, std::string_view);
    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<vec2i>(const std::vector<vec2i>&, std::string_view);
    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<vec3i>(const std::vector<vec3i>&, std::string_view);
    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<vec4i>(const std::vector<vec4i>&, std::string_view);
    template RAMSES_API DataArray* LogicEngine::createDataArrayInternal<std::vector<float>>(const std::vector<std::vector<float>>&, std::string_view);

    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<LogicObject>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<LuaScript>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<LuaModule>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<LuaInterface>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<NodeBinding>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<AppearanceBinding>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<CameraBinding>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<RenderPassBinding>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<RenderGroupBinding>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<MeshNodeBinding>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<SkinBinding>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<DataArray>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<AnimationNode>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<TimerNode>(ELuaSavingMode) const;
    template RAMSES_API size_t LogicEngine::getSerializedSizeInternal<AnchorPoint>(ELuaSavingMode) const;
}
