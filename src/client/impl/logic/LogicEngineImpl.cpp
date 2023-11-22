//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/logic/LogicEngineImpl.h"
#include "impl/ValidationReportImpl.h"

#include "ramses/framework/RamsesVersion.h"
#include "ramses/client/logic/LogicNode.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaModule.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/TimerNode.h"
#include "ramses/client/logic/AnchorPoint.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "ramses/client/logic/MeshNodeBinding.h"
#include "ramses/client/logic/SkinBinding.h"
#include "ramses/client/logic/RenderBufferBinding.h"

#include "impl/logic/AnchorPointImpl.h"
#include "impl/logic/LogicNodeImpl.h"
#include "impl/logic/LuaScriptImpl.h"
#include "impl/logic/LuaModuleImpl.h"
#include "impl/logic/LuaConfigImpl.h"
#include "impl/SaveFileConfigImpl.h"
#include "impl/logic/TimerNodeImpl.h"
#include "impl/logic/SkinBindingImpl.h"
#include "impl/logic/LogicEngineReportImpl.h"
#include "impl/logic/RenderGroupBindingElementsImpl.h"
#include "impl/SceneImpl.h"
#include "impl/NodeImpl.h"
#include "impl/CameraNodeImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/RenderPassImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/EFeatureLevelImpl.h"
#include "impl/ErrorReporting.h"
#include "impl/SerializationContext.h"

#include "internal/logic/FileUtils.h"
#include "internal/logic/TypeUtils.h"
#include "internal/logic/RamsesObjectResolver.h"

#include "ramses/client/Node.h"
#include "ramses/client/Camera.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Effect.h"
#include "ramses/client/RenderBuffer.h"
#include "ramses/client/ramses-utils.h"
#include "internal/Core/Utils/LogMacros.h"

#include "internal/logic/flatbuffers/generated/LogicEngineGen.h"
#include "ramses-sdk-build-config.h"

#include "fmt/format.h"

#include <string>
#include <fstream>
#include <streambuf>

namespace ramses::internal
{
    LogicEngineImpl::LogicEngineImpl(SceneImpl& sceneImpl, std::string_view name)
        : SceneObjectImpl{ sceneImpl, ERamsesObjectType::LogicEngine, name }
        , m_featureLevel{ sceneImpl.getClientImpl().getFramework().getFeatureLevel() }
        , m_apiObjects{ std::make_unique<ApiObjects>(m_featureLevel, sceneImpl) }
    {
    }

    LuaScript* LogicEngineImpl::createLuaScript(std::string_view source, const LuaConfigImpl& config, std::string_view scriptName)
    {
        return m_apiObjects->createLuaScript(source, config, scriptName, getErrorReporting());
    }

    LuaInterface* LogicEngineImpl::createLuaInterface(std::string_view source, const LuaConfigImpl& config, std::string_view interfaceName)
    {
        return m_apiObjects->createLuaInterface(source, config, interfaceName, getErrorReporting());
    }

    LuaModule* LogicEngineImpl::createLuaModule(std::string_view source, const LuaConfigImpl& config, std::string_view moduleName)
    {
        return m_apiObjects->createLuaModule(source, config, moduleName, getErrorReporting());
    }

    bool LogicEngineImpl::extractLuaDependencies(std::string_view source, const std::function<void(const std::string&)>& callbackFunc)
    {
        const std::optional<std::vector<std::string>> extractedDependencies = LuaCompilationUtils::ExtractModuleDependencies(source, getErrorReporting());
        if (!extractedDependencies)
            return false;

        for (const auto& dep : *extractedDependencies)
            callbackFunc(dep);

        return true;
    }

    NodeBinding* LogicEngineImpl::createNodeBinding(ramses::Node& ramsesNode, ramses::ERotationType rotationType, std::string_view name)
    {
        if (!isFromTheSameSceneAs(ramsesNode.impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create NodeBinding, object is from sceneId={} but LogicEngine is from sceneId={}", ramsesNode.impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        return m_apiObjects->createNodeBinding(ramsesNode, rotationType,  name);
    }

    AppearanceBinding* LogicEngineImpl::createAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name)
    {
        if (!isFromTheSameSceneAs(ramsesAppearance.impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create AppearanceBinding, object is from sceneId={} but LogicEngine is from sceneId={}", ramsesAppearance.impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        return m_apiObjects->createAppearanceBinding(ramsesAppearance, name);
    }

    CameraBinding* LogicEngineImpl::createCameraBinding(ramses::Camera& ramsesCamera, std::string_view name)
    {
        if (!isFromTheSameSceneAs(ramsesCamera.impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create CameraBinding, object is from sceneId={} but LogicEngine is from sceneId={}", ramsesCamera.impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        return m_apiObjects->createCameraBinding(ramsesCamera, false, name);
    }

    CameraBinding* LogicEngineImpl::createCameraBindingWithFrustumPlanes(ramses::Camera& ramsesCamera, std::string_view name)
    {
        if (!isFromTheSameSceneAs(ramsesCamera.impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create CameraBinding, object is from sceneId={} but LogicEngine is from sceneId={}", ramsesCamera.impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        return m_apiObjects->createCameraBinding(ramsesCamera, true, name);
    }

    RenderPassBinding* LogicEngineImpl::createRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name)
    {
        if (!isFromTheSameSceneAs(ramsesRenderPass.impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create RenderPassBinding, object is from sceneId={} but LogicEngine is from sceneId={}", ramsesRenderPass.impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        return m_apiObjects->createRenderPassBinding(ramsesRenderPass, name);
    }

    RenderGroupBinding* LogicEngineImpl::createRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RenderGroupBindingElements& elements, std::string_view name)
    {
        if (!isFromTheSameSceneAs(ramsesRenderGroup.impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create RenderGroupBinding, object is from sceneId={} but LogicEngine is from sceneId={}", ramsesRenderGroup.impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        if (elements.impl().getElements().empty())
        {
            getErrorReporting().set("Cannot create RenderGroupBinding, there were no elements provided.", *this);
            return nullptr;
        }

        if (!isFromTheSameSceneAs(elements.impl().getElements().front().second->impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create RenderGroupBinding, elements are from sceneId={} but LogicEngine is from sceneId={}",
                elements.impl().getElements().front().second->impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        for (const auto& element : elements.impl().getElements())
        {
            bool isContained = false;
            if (element.second->isOfType(ramses::ERamsesObjectType::MeshNode))
            {
                isContained = ramsesRenderGroup.containsMeshNode(*element.second->as<MeshNode>());
            }
            else if (element.second->isOfType(ramses::ERamsesObjectType::RenderGroup))
            {
                isContained = ramsesRenderGroup.containsRenderGroup(*element.second->as<ramses::RenderGroup>());
            }

            if (!isContained)
            {
                getErrorReporting().set("Cannot create RenderGroupBinding, one or more of the provided elements is not contained in the RenderGroup to bind.", *this);
                return nullptr;
            }
        }

        return m_apiObjects->createRenderGroupBinding(ramsesRenderGroup, elements, name);
    }

    MeshNodeBinding* LogicEngineImpl::createMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name)
    {
        if (!isFromTheSameSceneAs(ramsesMeshNode.impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create MeshNodeBinding, object is from sceneId={} but LogicEngine is from sceneId={}", ramsesMeshNode.impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        return m_apiObjects->createMeshNodeBinding(ramsesMeshNode, name);
    }

    SkinBinding* LogicEngineImpl::createSkinBinding(
        const std::vector<const NodeBinding*>& joints,
        const std::vector<matrix44f>& inverseBindMatrices,
        AppearanceBinding& appearanceBinding,
        const UniformInput& jointMatInput,
        std::string_view name)
    {
        if (joints.empty() || std::find(joints.cbegin(), joints.cend(), nullptr) != joints.cend())
        {
            getErrorReporting().set("Cannot create SkinBinding, no or null joint node bindings provided.", *this);
            return nullptr;
        }

        if (joints.size() != inverseBindMatrices.size())
        {
            getErrorReporting().set("Cannot create SkinBinding, number of inverse matrices must match the number of joints.", *this);
            return nullptr;
        }

        for (const auto nodeBinding : joints)
        {
            const auto& nodeBindings = m_apiObjects->getApiObjectContainer<NodeBinding>();
            if (std::find(nodeBindings.cbegin(), nodeBindings.cend(), nodeBinding) == nodeBindings.cend())
            {
                getErrorReporting().set(fmt::format("Failed to create SkinBinding '{}': one or more of the provided Ramses node bindings was not found in this logic instance.", name), *this);
                return nullptr;
            }
        }

        const auto& appearanceBindings = m_apiObjects->getApiObjectContainer<AppearanceBinding>();
        if (std::find(appearanceBindings.cbegin(), appearanceBindings.cend(), &appearanceBinding) == appearanceBindings.cend())
        {
            getErrorReporting().set(fmt::format("Failed to create SkinBinding '{}': provided Ramses appearance binding was not found in this logic instance.", name), *this);
            return nullptr;
        }

        const auto actualUniformInputOpt = appearanceBinding.getRamsesAppearance().getEffect().findUniformInput(jointMatInput.getName());
        if (!actualUniformInputOpt.has_value() || appearanceBinding.getRamsesAppearance().isInputBound(*actualUniformInputOpt))
        {
            getErrorReporting().set("Cannot create SkinBinding, provided uniform input must be pointing to valid uniform of the provided appearance's effect and must not be bound.", *this);
            return nullptr;
        }

        if (actualUniformInputOpt->getDataType() != ramses::EDataType::Matrix44F
            || actualUniformInputOpt->getElementCount() != joints.size())
        {
            getErrorReporting().set("Cannot create SkinBinding, provided uniform input must be of type array of Matrix4x4 with element count matching number of joints.", *this);
            return nullptr;
        }

        std::vector<const NodeBindingImpl*> jointsAsImpls;
        jointsAsImpls.reserve(joints.size());
        for (const auto j : joints)
            jointsAsImpls.push_back(&j->impl());

        return m_apiObjects->createSkinBinding(std::move(jointsAsImpls), inverseBindMatrices, appearanceBinding.impl(), *actualUniformInputOpt, name);
    }

    RenderBufferBinding* LogicEngineImpl::createRenderBufferBinding(ramses::RenderBuffer& renderBuffer, std::string_view name)
    {
        if (!isFromTheSameSceneAs(renderBuffer.impl()))
        {
            getErrorReporting().set(fmt::format("Failed to create RenderBufferBinding, object is from sceneId={} but LogicEngine is from sceneId={}", renderBuffer.impl().getSceneImpl().getSceneId(), getSceneImpl().getSceneId()), *this);
            return nullptr;
        }

        return m_apiObjects->createRenderBufferBinding(renderBuffer, name);
    }

    template <typename T>
    DataArray* LogicEngineImpl::createDataArray(const std::vector<T>& data, std::string_view name)
    {
        static_assert(CanPropertyTypeBeStoredInDataArray(PropertyTypeToEnum<T>::TYPE));

        if (data.empty())
        {
            getErrorReporting().set(fmt::format("Cannot create DataArray '{}' with empty data.", name), *this);
            return nullptr;
        }

        if constexpr (std::is_same_v<T, std::vector<float>>)
        {
            for (const auto& vec : data)
            {
                if (vec.size() != data.front().size())
                {
                    getErrorReporting().set("Failed to create DataArray of float arrays: all arrays must be of same size.", *this);
                    return nullptr;
                }
            }
        }

        // NOLINTNEXTLINE(readability-misleading-indentation) for some reason clang is confused about constexpr branch above
        return m_apiObjects->createDataArray(data, name);
    }

    ramses::AnimationNode* LogicEngineImpl::createAnimationNode(const AnimationNodeConfig& config, std::string_view name)
    {
        auto containsDataArray = [this](const DataArray* da) {
            const auto& dataArrays = m_apiObjects->getApiObjectContainer<DataArray>();
            const auto it = std::find_if(dataArrays.cbegin(), dataArrays.cend(),
                [da](const auto& d) { return d == da; });
            return it != dataArrays.cend();
        };

        if (config.getChannels().empty())
        {
            getErrorReporting().set(fmt::format("Failed to create AnimationNode '{}': must provide at least one channel.", name), *this);
            return nullptr;
        }

        for (const auto& channel : config.getChannels())
        {
            if (!containsDataArray(channel.timeStamps) ||
                !containsDataArray(channel.keyframes))
            {
                getErrorReporting().set(fmt::format("Failed to create AnimationNode '{}': timestamps or keyframes were not found in this logic instance.", name), *this);
                return nullptr;
            }

            if ((channel.tangentsIn && !containsDataArray(channel.tangentsIn)) ||
                (channel.tangentsOut && !containsDataArray(channel.tangentsOut)))
            {
                getErrorReporting().set(fmt::format("Failed to create AnimationNode '{}': tangents were not found in this logic instance.", name), *this);
                return nullptr;
            }
        }

        return m_apiObjects->createAnimationNode(config.impl(), name);
    }

    TimerNode* LogicEngineImpl::createTimerNode(std::string_view name)
    {
        return m_apiObjects->createTimerNode(name);
    }

    AnchorPoint* LogicEngineImpl::createAnchorPoint(NodeBinding& nodeBinding, CameraBinding& cameraBinding, std::string_view name)
    {
        const auto& nodeBindings = m_apiObjects->getApiObjectContainer<NodeBinding>();
        const auto& cameraBindings = m_apiObjects->getApiObjectContainer<CameraBinding>();
        if (std::find(nodeBindings.cbegin(), nodeBindings.cend(), &nodeBinding) == nodeBindings.cend() ||
            std::find(cameraBindings.cbegin(), cameraBindings.cend(), &cameraBinding) == cameraBindings.cend())
        {
            getErrorReporting().set(fmt::format("Failed to create AnchorPoint '{}': provided Ramses node binding and/or camera binding were not found in this logic instance.", name), *this);
            return nullptr;
        }

        return m_apiObjects->createAnchorPoint(nodeBinding.impl(), cameraBinding.impl(), name);
    }

    bool LogicEngineImpl::destroy(LogicObject& object)
    {
        return m_apiObjects->destroy(object, getErrorReporting());
    }

    bool LogicEngineImpl::isLinked(const LogicNode& logicNode) const
    {
        return m_apiObjects->getLogicNodeDependencies().isLinked(logicNode.impl());
    }

    size_t LogicEngineImpl::activateLinksRecursive(PropertyImpl& output)
    {
        size_t activatedLinks = 0u;

        const auto childCount = output.getChildCount();
        for (size_t i = 0; i < childCount; ++i)
        {
            PropertyImpl& child = output.getChild(i)->impl();

            if (TypeUtils::CanHaveChildren(child.getType()))
            {
                activatedLinks += activateLinksRecursive(child);
            }
            else
            {
                const auto& outgoingLinks = child.getOutgoingLinks();
                for (const auto& outLink : outgoingLinks)
                {
                    PropertyImpl* linkedProp = outLink.property;
                    const bool valueChanged = linkedProp->setValue(child.getValue());
                    if (valueChanged || linkedProp->getPropertySemantics() == EPropertySemantics::AnimationInput)
                    {
                        linkedProp->getLogicNode().setDirty(true);
                        ++activatedLinks;
                    }
                }
            }
        }

        return activatedLinks;
    }

    bool LogicEngineImpl::update()
    {
        if (m_statisticsEnabled || m_updateReportEnabled)
        {
            m_updateReport.clear();
            m_updateReport.sectionStarted(UpdateReport::ETimingSection::TotalUpdate);
        }
        if (m_updateReportEnabled)
        {
            m_updateReport.sectionStarted(UpdateReport::ETimingSection::TopologySort);
        }

        const std::optional<NodeVector>& sortedNodes = m_apiObjects->getLogicNodeDependencies().getTopologicallySortedNodes();
        if (!sortedNodes)
        {
            getErrorReporting().set("Failed to sort logic nodes based on links between their properties. Create a loop-free link graph before calling update()!", *this);
            return false;
        }

        if (m_updateReportEnabled)
            m_updateReport.sectionFinished(UpdateReport::ETimingSection::TopologySort);

        // force dirty all timer nodes, anchor points and skinbindings
        setNodeToBeAlwaysUpdatedDirty();

        const bool success = updateNodes(*sortedNodes);

        if (m_statisticsEnabled || m_updateReportEnabled)
        {
            m_updateReport.sectionFinished(UpdateReport::ETimingSection::TotalUpdate);
            m_statistics.collect(m_updateReport, sortedNodes->size());
            if (m_statistics.checkUpdateFrameFinished())
                m_statistics.calculateAndLog();
        }

        return success;
    }

    bool LogicEngineImpl::updateNodes(const NodeVector& sortedNodes)
    {
        for (LogicNodeImpl* nodeIter : sortedNodes)
        {
            LogicNodeImpl& node = *nodeIter;

            if (!node.isDirty())
            {
                if (m_updateReportEnabled)
                    m_updateReport.nodeSkippedExecution(node);

                if(m_nodeDirtyMechanismEnabled)
                    continue;
            }

            if (m_updateReportEnabled)
                m_updateReport.nodeExecutionStarted(node);
            if (m_statisticsEnabled)
                m_statistics.nodeExecuted();

            const std::optional<LogicNodeRuntimeError> potentialError = node.update();
            if (potentialError)
            {
                getErrorReporting().set(potentialError->message, &node.getLogicObject());
                return false;
            }

            Property* outputs = node.getOutputs();
            if (outputs != nullptr)
            {
                const size_t activatedLinks = activateLinksRecursive(outputs->impl());

                if (m_statisticsEnabled || m_updateReportEnabled)
                    m_updateReport.linksActivated(activatedLinks);
            }

            if (m_updateReportEnabled)
                m_updateReport.nodeExecutionFinished();

            node.setDirty(false);
        }

        return true;
    }

    void LogicEngineImpl::setNodeToBeAlwaysUpdatedDirty()
    {
        // force timer nodes dirty so they can update their ticker
        for (TimerNode* timerNode : m_apiObjects->getApiObjectContainer<TimerNode>())
            timerNode->impl().setDirty(true);
        // force anchor points dirty because they depend on set of ramses states which cannot be monitored
        for (AnchorPoint* anchorPoint : m_apiObjects->getApiObjectContainer<AnchorPoint>())
            anchorPoint->impl().setDirty(true);
        // force skinbindings dirty because they depend on set of ramses states which cannot be monitored
        for (SkinBinding* skinBinding : m_apiObjects->getApiObjectContainer<SkinBinding>())
            skinBinding->impl().setDirty(true);
    }

    void LogicEngineImpl::onValidate(ValidationReportImpl& report) const
    {
        if (m_apiObjects->bindingsDirty())
            report.add(EIssueType::Warning, "Saving logic engine content with manually updated binding values without calling update() will result in those values being lost!", nullptr);

        m_apiObjects->validateInterfaces(report);
        m_apiObjects->validateDanglingNodes(report);
    }

    bool LogicEngineImpl::loadFromByteData(const void* byteData, size_t byteSize, bool enableMemoryVerification, const std::string& dataSourceDescription)
    {
        if (byteSize < 8)
        {
            getErrorReporting().set(fmt::format("{} contains corrupted data! Data should be at least 8 bytes", dataSourceDescription), *this);
            return false;
        }

        auto* uint8Data(static_cast<const uint8_t*>(byteData));
        if (enableMemoryVerification)
        {
            flatbuffers::Verifier bufferVerifier(uint8Data, byteSize);
            const bool bufferOK = bufferVerifier.VerifyBuffer<rlogic_serialization::LogicEngine>();

            if (!bufferOK)
            {
                getErrorReporting().set(fmt::format("{} contains corrupted data!", dataSourceDescription), *this);
                return false;
            }
        }

        const auto* logicEngine = rlogic_serialization::GetLogicEngine(byteData);

        if (nullptr == logicEngine)
        {
            getErrorReporting().set(fmt::format("{} doesn't contain logic engine data with readable version specifiers", dataSourceDescription), *this);
            return false;
        }

        LOG_INFO(CONTEXT_CLIENT, "Loading logic engine content from '{}'", dataSourceDescription);

        if (nullptr == logicEngine->apiObjects())
        {
            getErrorReporting().set(fmt::format("Fatal error while loading {}: doesn't contain API objects!", dataSourceDescription), *this);
            return false;
        }

        RamsesObjectResolver ramsesResolver{ getErrorReporting(), getSceneImpl() };
        std::unique_ptr<ApiObjects> deserializedObjects = ApiObjects::Deserialize(getSceneImpl(), *logicEngine->apiObjects(), ramsesResolver, dataSourceDescription, getErrorReporting(), m_featureLevel);

        if (!deserializedObjects)
        {
            return false;
        }

        // No errors -> move data into member
        m_apiObjects = std::move(deserializedObjects);

        return true;
    }

    bool LogicEngineImpl::save(flatbuffers::FlatBufferBuilder& builder, const SaveFileConfigImpl& config)
    {
        // Refuse save() if logic graph has loops
        if (!m_apiObjects->getLogicNodeDependencies().getTopologicallySortedNodes())
        {
            getErrorReporting().set("Failed to sort logic nodes based on links between their properties. Create a loop-free link graph before calling saveToFile()!", *this);
            return false;
        }

        const auto& scripts = m_apiObjects->getApiObjectContainer<LuaScript>();
        const auto sIt = std::find_if(scripts.cbegin(), scripts.cend(), [](const LuaScript* s) { return s->impl().hasDebugLogFunctions(); });
        if (sIt != scripts.cend())
        {
            getErrorReporting().set(fmt::format("Cannot save to file, Lua script '{}' has enabled debug log functions, remove this script before saving.", (*sIt)->impl().getIdentificationString()), *sIt);
            return false;
        }
        const auto& modules = m_apiObjects->getApiObjectContainer<LuaModule>();
        const auto mIt = std::find_if(modules.cbegin(), modules.cend(), [](const LuaModule* m) { return m->impl().hasDebugLogFunctions(); });
        if (mIt != modules.cend())
        {
            getErrorReporting().set(fmt::format("Cannot save to file, Lua module '{}' has enabled debug log functions, remove this module before saving.", (*mIt)->impl().getIdentificationString()), *mIt);
            return false;
        }

        const auto logicEngine = rlogic_serialization::CreateLogicEngine(builder,
            ApiObjects::Serialize(*m_apiObjects, builder, config.getLuaSavingMode()));

        builder.Finish(logicEngine);

        return true;
    }

    bool LogicEngineImpl::link(Property& sourceProperty, Property& targetProperty)
    {
        return m_apiObjects->getLogicNodeDependencies().link(sourceProperty.impl(), targetProperty.impl(), false, getErrorReporting());
    }

    bool LogicEngineImpl::linkWeak(Property& sourceProperty, Property& targetProperty)
    {
        return m_apiObjects->getLogicNodeDependencies().link(sourceProperty.impl(), targetProperty.impl(), true, getErrorReporting());
    }

    bool LogicEngineImpl::unlink(Property& sourceProperty, Property& targetProperty)
    {
        return m_apiObjects->getLogicNodeDependencies().unlink(sourceProperty.impl(), targetProperty.impl(), getErrorReporting());
    }

    const ApiObjects& LogicEngineImpl::getApiObjects() const
    {
        return *m_apiObjects;
    }

    ApiObjects& LogicEngineImpl::getApiObjects()
    {
        return *m_apiObjects;
    }

    void LogicEngineImpl::disableTrackingDirtyNodes()
    {
        m_nodeDirtyMechanismEnabled = false;
    }

    void LogicEngineImpl::enableUpdateReport(bool enable)
    {
        m_updateReportEnabled = enable;
        if (!m_updateReportEnabled)
            m_updateReport.clear();
    }

    LogicEngineReport LogicEngineImpl::getLastUpdateReport() const
    {
        return LogicEngineReport{ std::make_unique<LogicEngineReportImpl>(m_updateReport) };
    }

    void LogicEngineImpl::setStatisticsLoggingRate(size_t loggingRate, EStatisticsLogMode mode)
    {
        m_statistics.setLoggingRate(loggingRate);
        if (loggingRate == 0u)
        {
            m_statisticsEnabled = false;
            enableUpdateReport(false);
            return;
        }

        m_statisticsEnabled = true;
        switch (mode)
        {
        case EStatisticsLogMode::Compact:
            enableUpdateReport(false);
            break;
        case EStatisticsLogMode::Detailed:
            enableUpdateReport(true);
            break;
        }
    }

    size_t LogicEngineImpl::getTotalSerializedSize(ELuaSavingMode luaSavingMode) const
    {
        return ApiObjectsSerializedSize::GetTotalSerializedSize(*m_apiObjects, luaSavingMode);
    }

    bool LogicEngineImpl::serialize(ramses::internal::IOutputStream& outStream, SerializationContext& serializationContext) const
    {
        if (!SceneObjectImpl::serialize(outStream, serializationContext))
            return false;

        flatbuffers::FlatBufferBuilder builder;

        if (!const_cast<LogicEngineImpl*>(this)->save(builder, serializationContext.getSaveConfig()))
            return false;

        uint32_t size = builder.GetSize();
        outStream << size;
        outStream.write(builder.GetBufferPointer(), builder.GetSize());
        return true;
    }

    bool LogicEngineImpl::deserialize(ramses::internal::IInputStream& inStream, DeserializationContext& serializationContext)
    {
        if (!SceneObjectImpl::deserialize(inStream, serializationContext))
            return false;

        uint32_t size = 0u;
        inStream >> size;
        m_byteBuffer.resize(size);
        inStream.read(m_byteBuffer.data(), size);
        // we need to parse the byte buffer later when all scene objects are available
        serializationContext.addForDependencyResolve(this);
        return true;
    }

    bool LogicEngineImpl::resolveDeserializationDependencies(DeserializationContext& serializationContext)
    {
        const bool enableMemoryVerification = serializationContext.getLoadConfig().getMemoryVerificationEnabled();
        if (!loadFromByteData(m_byteBuffer.data(), m_byteBuffer.size(), enableMemoryVerification, fmt::format("data buffer '{}' (size: {})", m_byteBuffer.data(), m_byteBuffer.size())))
            return false;

        std::vector<char>().swap(m_byteBuffer);
        return true;
    }

    template DataArray* LogicEngineImpl::createDataArray<float>(const std::vector<float>&, std::string_view name);
    template DataArray* LogicEngineImpl::createDataArray<vec2f>(const std::vector<vec2f>&, std::string_view name);
    template DataArray* LogicEngineImpl::createDataArray<vec3f>(const std::vector<vec3f>&, std::string_view name);
    template DataArray* LogicEngineImpl::createDataArray<vec4f>(const std::vector<vec4f>&, std::string_view name);
    template DataArray* LogicEngineImpl::createDataArray<int32_t>(const std::vector<int32_t>&, std::string_view name);
    template DataArray* LogicEngineImpl::createDataArray<vec2i>(const std::vector<vec2i>&, std::string_view name);
    template DataArray* LogicEngineImpl::createDataArray<vec3i>(const std::vector<vec3i>&, std::string_view name);
    template DataArray* LogicEngineImpl::createDataArray<vec4i>(const std::vector<vec4i>&, std::string_view name);
    template DataArray* LogicEngineImpl::createDataArray<std::vector<float>>(const std::vector<std::vector<float>>&, std::string_view name);
}
