//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/LogicEngineImpl.h"

#include "ramses-framework-api/RamsesVersion.h"
#include "ramses-logic/LogicNode.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/LuaModule.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/RamsesCameraBinding.h"
#include "ramses-logic/RamsesAppearanceBinding.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/AnchorPoint.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/RamsesRenderGroupBinding.h"
#include "ramses-logic/RamsesRenderGroupBindingElements.h"
#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "ramses-logic/SkinBinding.h"

#include "impl/LogicNodeImpl.h"
#include "impl/LoggerImpl.h"
#include "impl/LuaScriptImpl.h"
#include "impl/LuaModuleImpl.h"
#include "impl/LuaConfigImpl.h"
#include "impl/SaveFileConfigImpl.h"
#include "impl/LogicEngineReportImpl.h"
#include "impl/RamsesRenderGroupBindingElementsImpl.h"

#include "internals/FileUtils.h"
#include "internals/TypeUtils.h"
#include "internals/RamsesObjectResolver.h"
#include "internals/ApiObjects.h"

#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"
#include "ramses-utils.h"

#include "generated/LogicEngineGen.h"
#include "ramses-sdk-build-config.h"

#include "fmt/format.h"

#include <string>
#include <fstream>
#include <streambuf>

namespace
{
    const char* const fileIdFeatureLevel01         = "rl01";
    const char* const fileIdFeatureLevel02orHigher = "rl02";
} // namespace

namespace rlogic::internal
{
    LogicEngineImpl::LogicEngineImpl(EFeatureLevel featureLevel)
        : m_apiObjects{ std::make_unique<ApiObjects>(featureLevel) }
        , m_featureLevel{ featureLevel }
    {
        if (std::find(AllFeatureLevels.cbegin(), AllFeatureLevels.cend(), m_featureLevel) == AllFeatureLevels.cend())
        {
            LOG_ERROR("Unrecognized feature level '0{}' provided, falling back to feature level 01", m_featureLevel);
            m_featureLevel = EFeatureLevel_01;
        }
    }

    LogicEngineImpl::~LogicEngineImpl() noexcept = default;

    LuaScript* LogicEngineImpl::createLuaScript(std::string_view source, const LuaConfigImpl& config, std::string_view scriptName)
    {
        m_errors.clear();
        return m_apiObjects->createLuaScript(source, config, scriptName, m_errors);
    }

    LuaInterface* LogicEngineImpl::createLuaInterface(std::string_view source, const LuaConfigImpl& config, std::string_view interfaceName, bool verifyModules)
    {
        m_errors.clear();
        return m_apiObjects->createLuaInterface(source, config, interfaceName, m_errors, verifyModules);
    }

    LuaModule* LogicEngineImpl::createLuaModule(std::string_view source, const LuaConfigImpl& config, std::string_view moduleName)
    {
        m_errors.clear();
        return m_apiObjects->createLuaModule(source, config, moduleName, m_errors);
    }

    bool LogicEngineImpl::extractLuaDependencies(std::string_view source, const std::function<void(const std::string&)>& callbackFunc)
    {
        m_errors.clear();
        const std::optional<std::vector<std::string>> extractedDependencies = LuaCompilationUtils::ExtractModuleDependencies(source, m_errors);
        if (!extractedDependencies)
            return false;

        for (const auto& dep : *extractedDependencies)
            callbackFunc(dep);

        return true;
    }

    RamsesNodeBinding* LogicEngineImpl::createRamsesNodeBinding(ramses::Node& ramsesNode, ERotationType rotationType, std::string_view name)
    {
        m_errors.clear();
        return m_apiObjects->createRamsesNodeBinding(ramsesNode, rotationType,  name);
    }

    RamsesAppearanceBinding* LogicEngineImpl::createRamsesAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name)
    {
        m_errors.clear();
        return m_apiObjects->createRamsesAppearanceBinding(ramsesAppearance, name);
    }

    RamsesCameraBinding* LogicEngineImpl::createRamsesCameraBinding(ramses::Camera& ramsesCamera, std::string_view name)
    {
        m_errors.clear();
        return m_apiObjects->createRamsesCameraBinding(ramsesCamera, false, name);
    }

    RamsesCameraBinding* LogicEngineImpl::createRamsesCameraBindingWithFrustumPlanes(ramses::Camera& ramsesCamera, std::string_view name)
    {
        m_errors.clear();
        if (m_featureLevel < EFeatureLevel_02)
        {
            m_errors.add(fmt::format("Cannot create RamsesCameraBinding with frustum planes properties, feature level 02 or higher is required, feature level in this runtime set to 0{}.", m_featureLevel), nullptr, EErrorType::Other);
            return nullptr;
        }

        return m_apiObjects->createRamsesCameraBinding(ramsesCamera, true, name);
    }

    RamsesRenderPassBinding* LogicEngineImpl::createRamsesRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name)
    {
        m_errors.clear();
        if (m_featureLevel < EFeatureLevel_02)
        {
            m_errors.add(fmt::format("Cannot create RamsesRenderPassBinding, feature level 02 or higher is required, feature level in this runtime set to 0{}.", m_featureLevel), nullptr, EErrorType::Other);
            return nullptr;
        }

        return m_apiObjects->createRamsesRenderPassBinding(ramsesRenderPass, name);
    }

    RamsesRenderGroupBinding* LogicEngineImpl::createRamsesRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RamsesRenderGroupBindingElements& elements, std::string_view name)
    {
        m_errors.clear();
        if (m_featureLevel < EFeatureLevel_03)
        {
            m_errors.add(fmt::format("Cannot create RamsesRenderGroupBinding, feature level 03 or higher is required, feature level in this runtime set to 0{}.", m_featureLevel), nullptr, EErrorType::Other);
            return nullptr;
        }

        if (elements.m_impl->getElements().empty())
        {
            m_errors.add("Cannot create RamsesRenderGroupBinding, there were no elements provided.", nullptr, EErrorType::Other);
            return nullptr;
        }

        for (const auto& element : elements.m_impl->getElements())
        {
            bool isContained = false;
            if (element.second->isOfType(ramses::ERamsesObjectType_MeshNode))
            {
                isContained = ramsesRenderGroup.containsMeshNode(*ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*element.second));
            }
            else if (element.second->isOfType(ramses::ERamsesObjectType_RenderGroup))
            {
                isContained = ramsesRenderGroup.containsRenderGroup(*ramses::RamsesUtils::TryConvert<ramses::RenderGroup>(*element.second));
            }

            if (!isContained)
            {
                m_errors.add("Cannot create RamsesRenderGroupBinding, one or more of the provided elements is not contained in the RenderGroup to bind.", nullptr, EErrorType::Other);
                return nullptr;
            }
        }

        return m_apiObjects->createRamsesRenderGroupBinding(ramsesRenderGroup, elements, name);
    }

    RamsesMeshNodeBinding* LogicEngineImpl::createRamsesMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name)
    {
        m_errors.clear();
        if (m_featureLevel < EFeatureLevel_05)
        {
            m_errors.add(fmt::format("Cannot create RamsesMeshNodeBinding, feature level 05 or higher is required, feature level in this runtime set to 0{}.", m_featureLevel), nullptr, EErrorType::Other);
            return nullptr;
        }

        return m_apiObjects->createRamsesMeshNodeBinding(ramsesMeshNode, name);
    }

    SkinBinding* LogicEngineImpl::createSkinBinding(
        const std::vector<const RamsesNodeBinding*>& joints,
        const std::vector<matrix44f>& inverseBindMatrices,
        RamsesAppearanceBinding& appearanceBinding,
        const ramses::UniformInput& jointMatInput,
        std::string_view name)
    {
        m_errors.clear();
        if (m_featureLevel < EFeatureLevel_04)
        {
            m_errors.add(fmt::format("Cannot create SkinBinding, feature level 04 or higher is required, feature level in this runtime set to 0{}.", m_featureLevel), nullptr, EErrorType::Other);
            return nullptr;
        }

        if (joints.empty() || std::find(joints.cbegin(), joints.cend(), nullptr) != joints.cend())
        {
            m_errors.add("Cannot create SkinBinding, no or null joint node bindings provided.", nullptr, EErrorType::Other);
            return nullptr;
        }

        if (joints.size() != inverseBindMatrices.size())
        {
            m_errors.add("Cannot create SkinBinding, number of inverse matrices must match the number of joints.", nullptr, EErrorType::Other);
            return nullptr;
        }

        for (const auto nodeBinding : joints)
        {
            const auto& nodeBindings = m_apiObjects->getApiObjectContainer<RamsesNodeBinding>();
            if (std::find(nodeBindings.cbegin(), nodeBindings.cend(), nodeBinding) == nodeBindings.cend())
            {
                m_errors.add(fmt::format("Failed to create SkinBinding '{}': one or more of the provided Ramses node bindings was not found in this logic instance.", name), nullptr, EErrorType::IllegalArgument);
                return nullptr;
            }
        }

        const auto& appearanceBindings = m_apiObjects->getApiObjectContainer<RamsesAppearanceBinding>();
        if (std::find(appearanceBindings.cbegin(), appearanceBindings.cend(), &appearanceBinding) == appearanceBindings.cend())
        {
            m_errors.add(fmt::format("Failed to create SkinBinding '{}': provided Ramses appearance binding was not found in this logic instance.", name), nullptr, EErrorType::IllegalArgument);
            return nullptr;
        }

        ramses::UniformInput actualUniformInput;
        appearanceBinding.getRamsesAppearance().getEffect().findUniformInput(jointMatInput.getName(), actualUniformInput);
        if (!actualUniformInput.isValid() || appearanceBinding.getRamsesAppearance().isInputBound(actualUniformInput))
        {
            m_errors.add("Cannot create SkinBinding, provided uniform input must be pointing to valid uniform of the provided appearance's effect and must not be bound.",
                nullptr, EErrorType::Other);
            return nullptr;
        }

        if (*actualUniformInput.getDataType() != ramses::EDataType::Matrix44F
            || actualUniformInput.getElementCount() != joints.size())
        {
            m_errors.add("Cannot create SkinBinding, provided uniform input must be of type array of Matrix4x4 with element count matching number of joints.", nullptr, EErrorType::Other);
            return nullptr;
        }

        std::vector<const RamsesNodeBindingImpl*> jointsAsImpls;
        jointsAsImpls.reserve(joints.size());
        for (const auto j : joints)
            jointsAsImpls.push_back(&j->m_nodeBinding);

        return m_apiObjects->createSkinBinding(std::move(jointsAsImpls), inverseBindMatrices, appearanceBinding.m_appearanceBinding, actualUniformInput, name);
    }

    template <typename T>
    DataArray* LogicEngineImpl::createDataArray(const std::vector<T>& data, std::string_view name)
    {
        static_assert(CanPropertyTypeBeStoredInDataArray(PropertyTypeToEnum<T>::TYPE));
        m_errors.clear();

        if (data.empty())
        {
            m_errors.add(fmt::format("Cannot create DataArray '{}' with empty data.", name), nullptr, EErrorType::IllegalArgument);
            return nullptr;
        }

        if constexpr (std::is_same_v<T, std::vector<float>>)
        {
            if (m_featureLevel < EFeatureLevel_04)
            {
                m_errors.add(fmt::format("Cannot create DataArray of float arrays, feature level 04 or higher is required, feature level in this runtime set to 0{}.", m_featureLevel), nullptr, EErrorType::Other);
                return nullptr;
            }

            for (const auto& vec : data)
            {
                if (vec.size() != data.front().size())
                {
                    m_errors.add("Failed to create DataArray of float arrays: all arrays must be of same size.", nullptr, EErrorType::Other);
                    return nullptr;
                }
            }
        }

        // NOLINTNEXTLINE(readability-misleading-indentation) for some reason clang is confused about constexpr branch above
        return m_apiObjects->createDataArray(data, name);
    }

    rlogic::AnimationNode* LogicEngineImpl::createAnimationNode(const AnimationNodeConfig& config, std::string_view name)
    {
        m_errors.clear();

        auto containsDataArray = [this](const DataArray* da) {
            const auto& dataArrays = m_apiObjects->getApiObjectContainer<DataArray>();
            const auto it = std::find_if(dataArrays.cbegin(), dataArrays.cend(),
                [da](const auto& d) { return d == da; });
            return it != dataArrays.cend();
        };

        if (config.getChannels().empty())
        {
            m_errors.add(fmt::format("Failed to create AnimationNode '{}': must provide at least one channel.", name), nullptr, EErrorType::IllegalArgument);
            return nullptr;
        }

        for (const auto& channel : config.getChannels())
        {
            if (!containsDataArray(channel.timeStamps) ||
                !containsDataArray(channel.keyframes))
            {
                m_errors.add(fmt::format("Failed to create AnimationNode '{}': timestamps or keyframes were not found in this logic instance.", name), nullptr, EErrorType::IllegalArgument);
                return nullptr;
            }

            if ((channel.tangentsIn && !containsDataArray(channel.tangentsIn)) ||
                (channel.tangentsOut && !containsDataArray(channel.tangentsOut)))
            {
                m_errors.add(fmt::format("Failed to create AnimationNode '{}': tangents were not found in this logic instance.", name), nullptr, EErrorType::IllegalArgument);
                return nullptr;
            }
        }

        return m_apiObjects->createAnimationNode(*config.m_impl, name);
    }

    TimerNode* LogicEngineImpl::createTimerNode(std::string_view name)
    {
        m_errors.clear();
        return m_apiObjects->createTimerNode(name);
    }

    AnchorPoint* LogicEngineImpl::createAnchorPoint(RamsesNodeBinding& nodeBinding, RamsesCameraBinding& cameraBinding, std::string_view name)
    {
        m_errors.clear();
        if (m_featureLevel < EFeatureLevel_02)
        {
            m_errors.add(fmt::format("Cannot create AnchorPoint, feature level 02 or higher is required, feature level in this runtime set to 0{}.", m_featureLevel), nullptr, EErrorType::Other);
            return nullptr;
        }

        const auto& nodeBindings = m_apiObjects->getApiObjectContainer<RamsesNodeBinding>();
        const auto& cameraBindings = m_apiObjects->getApiObjectContainer<RamsesCameraBinding>();
        if (std::find(nodeBindings.cbegin(), nodeBindings.cend(), &nodeBinding) == nodeBindings.cend() ||
            std::find(cameraBindings.cbegin(), cameraBindings.cend(), &cameraBinding) == cameraBindings.cend())
        {
            m_errors.add(fmt::format("Failed to create AnchorPoint '{}': provided Ramses node binding and/or camera binding were not found in this logic instance.", name), nullptr, EErrorType::IllegalArgument);
            return nullptr;
        }

        return m_apiObjects->createAnchorPoint(nodeBinding.m_nodeBinding, cameraBinding.m_cameraBinding, name);
    }

    bool LogicEngineImpl::destroy(LogicObject& object)
    {
        m_errors.clear();
        return m_apiObjects->destroy(object, m_errors);
    }

    bool LogicEngineImpl::isLinked(const LogicNode& logicNode) const
    {
        return m_apiObjects->getLogicNodeDependencies().isLinked(logicNode.m_impl);
    }

    EFeatureLevel LogicEngineImpl::getFeatureLevel() const
    {
        return m_featureLevel;
    }

    size_t LogicEngineImpl::activateLinksRecursive(PropertyImpl& output)
    {
        size_t activatedLinks = 0u;

        const auto childCount = output.getChildCount();
        for (size_t i = 0; i < childCount; ++i)
        {
            PropertyImpl& child = *output.getChild(i)->m_impl;

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
        m_errors.clear();

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
            m_errors.add("Failed to sort logic nodes based on links between their properties. Create a loop-free link graph before calling update()!", nullptr, EErrorType::ContentStateError);
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
                m_errors.add(potentialError->message, m_apiObjects->getApiObject(node), EErrorType::RuntimeError);
                return false;
            }

            Property* outputs = node.getOutputs();
            if (outputs != nullptr)
            {
                const size_t activatedLinks = activateLinksRecursive(*outputs->m_impl);

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
            timerNode->m_impl.setDirty(true);
        // force anchor points dirty because they depend on set of ramses states which cannot be monitored
        for (AnchorPoint* anchorPoint : m_apiObjects->getApiObjectContainer<AnchorPoint>())
            anchorPoint->m_impl.setDirty(true);
        // force skinbindings dirty because they depend on set of ramses states which cannot be monitored
        for (SkinBinding* skinBinding : m_apiObjects->getApiObjectContainer<SkinBinding>())
            skinBinding->m_impl.setDirty(true);
    }

    const std::vector<ErrorData>& LogicEngineImpl::getErrors() const
    {
        return m_errors.getErrors();
    }

    const std::vector<WarningData>& LogicEngineImpl::validate() const
    {
        m_validationResults.clear();

        if (m_apiObjects->bindingsDirty())
            m_validationResults.add("Saving logic engine content with manually updated binding values without calling update() will result in those values being lost!", nullptr, EWarningType::UnsafeDataState);

        m_apiObjects->validateInterfaces(m_validationResults);
        m_apiObjects->validateDanglingNodes(m_validationResults);

        return m_validationResults.getWarnings();
    }

    bool LogicEngineImpl::CheckRamsesVersionFromFile(const rlogic_serialization::Version& ramsesVersion)
    {
        // Only major version changes result in file incompatibilities
        return static_cast<int>(ramsesVersion.v_major()) == ramses::GetRamsesVersion().major;
    }

    bool LogicEngineImpl::loadFromBuffer(const void* rawBuffer, size_t bufferSize, ramses::Scene* scene, bool enableMemoryVerification)
    {
        return loadFromByteData(rawBuffer, bufferSize, scene, enableMemoryVerification, fmt::format("data buffer '{}' (size: {})", rawBuffer, bufferSize));
    }

    bool LogicEngineImpl::loadFromFile(std::string_view filename, ramses::Scene* scene, bool enableMemoryVerification)
    {
        std::optional<std::vector<char>> maybeBytesFromFile = FileUtils::LoadBinary(std::string(filename));
        if (!maybeBytesFromFile)
        {
            m_errors.add(fmt::format("Failed to load file '{}'", filename), nullptr, EErrorType::BinaryDataAccessError);
            return false;
        }

        const size_t fileSize = (*maybeBytesFromFile).size();
        return loadFromByteData((*maybeBytesFromFile).data(), fileSize, scene, enableMemoryVerification, fmt::format("file '{}' (size: {})", filename, fileSize));
    }

    bool LogicEngineImpl::loadFromFileDescriptor(int fd, size_t offset, size_t size, ramses::Scene* scene, bool enableMemoryVerification)
    {
        if (fd <= 0)
        {
            m_errors.add(fmt::format("Invalid file descriptor: {}", fd), nullptr, EErrorType::BinaryDataAccessError);
            return false;
        }
        if (size == 0)
        {
            m_errors.add("Failed to load from file descriptor: size may not be 0", nullptr, EErrorType::BinaryDataAccessError);
            return false;
        }
        std::optional<std::vector<char>> maybeBytesFromFile = FileUtils::LoadBinary(fd, offset, size);
        if (!maybeBytesFromFile)
        {
            m_errors.add(fmt::format("Failed to load from file descriptor: fd: {} offset: {} size: {}", fd, offset, size), nullptr, EErrorType::BinaryDataAccessError);
            return false;
        }
        return loadFromByteData((*maybeBytesFromFile).data(), size, scene, enableMemoryVerification, fmt::format("fd: {} (offset: {}, size: {})", fd, offset, size));
    }

    bool LogicEngineImpl::checkFileIdentifierBytes(const std::string& dataSourceDescription, const std::string& fileIdBytes)
    {
        const std::string expected = getFileIdentifierMatchingFeatureLevel();
        if (expected.substr(0, 2) != fileIdBytes.substr(0, 2))
        {
            m_errors.add(fmt::format("{}: Tried loading a binary data which doesn't store Ramses Logic content! Expected file bytes 4-5 to be '{}', but found '{}' instead",
                dataSourceDescription,
                expected.substr(0, 2),
                fileIdBytes.substr(0, 2)
            ), nullptr, EErrorType::BinaryDataAccessError);
            return false;
        }

        // print dedicated error messages for feature level 1 mismatch
        // feature level 02 introduced a file id change (but future feature levels will not)

        if ((fileIdBytes == fileIdFeatureLevel01) && (m_featureLevel != EFeatureLevel_01))
        {
            m_errors.add(fmt::format("{}: Feature level mismatch! Loaded file with feature level {} but LogicEngine was instantiated with feature level {}",
                dataSourceDescription, EFeatureLevel_01, m_featureLevel), nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        if ((fileIdBytes == fileIdFeatureLevel02orHigher) && (m_featureLevel == EFeatureLevel_01))
        {
            m_errors.add(fmt::format("{}: Feature level mismatch! Loaded file with feature level >={} but LogicEngine was instantiated with feature level {}",
                dataSourceDescription, EFeatureLevel_02, m_featureLevel), nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        if (expected.substr(2, 2) != fileIdBytes.substr(2, 2))
        {
            m_errors.add(fmt::format("{}: Version mismatch while loading binary data! Expected version '{}', but found '{}'",
                dataSourceDescription,
                expected.substr(2, 2),
                fileIdBytes.substr(2, 2)
            ),nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        return true;
    }

    const char* LogicEngineImpl::getFileIdentifierMatchingFeatureLevel() const
    {
        // Solution (workaround) to make sure that previously released rlogic 1.x.x library will fail to load exported binary
        // with any other than the base 01 feature level. Feature level 02 or higher has its own file identifier which can be read
        // only by rlogic version that supports it.
        // TODO remove this with new major release, feature levels are stored in schema and checked when loading,
        // no need to use file identifier for it.

        assert(std::string(rlogic_serialization::LogicEngineIdentifier()) == fileIdFeatureLevel01);
        assert(std::find(AllFeatureLevels.cbegin(), AllFeatureLevels.cend(), m_featureLevel) != AllFeatureLevels.cend());
        return m_featureLevel == EFeatureLevel_01 ? rlogic_serialization::LogicEngineIdentifier() : fileIdFeatureLevel02orHigher;
    }

    bool LogicEngineImpl::loadFromByteData(const void* byteData, size_t byteSize, ramses::Scene* scene, bool enableMemoryVerification, const std::string& dataSourceDescription)
    {
        m_errors.clear();

        if (byteSize < 8)
        {
            m_errors.add(fmt::format("{} contains corrupted data! Data should be at least 8 bytes", dataSourceDescription), nullptr, EErrorType::BinaryDataAccessError);
            return false;
        }

        auto* char8Data(static_cast<const char*>(byteData));
        // file identifier bytes are always placed at bytes 4-7 in the buffer
        const std::string fileIdBytes(&char8Data[4], 4); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic) No better options
        if (!checkFileIdentifierBytes(dataSourceDescription, fileIdBytes))
        {
            return false;
        }

        auto* uint8Data(static_cast<const uint8_t*>(byteData));
        if (enableMemoryVerification)
        {
            flatbuffers::Verifier bufferVerifier(uint8Data, byteSize);
            const bool bufferOK = bufferVerifier.VerifyBuffer<rlogic_serialization::LogicEngine>(getFileIdentifierMatchingFeatureLevel());

            if (!bufferOK)
            {
                m_errors.add(fmt::format("{} contains corrupted data!", dataSourceDescription), nullptr, EErrorType::BinaryDataAccessError);
                return false;
            }
        }

        const auto* logicEngine = rlogic_serialization::GetLogicEngine(byteData);

        if (nullptr == logicEngine)
        {
            m_errors.add(fmt::format("{} doesn't contain logic engine data with readable version specifiers", dataSourceDescription), nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        const auto& ramsesVersion = *logicEngine->ramsesVersion();
        const auto& rlogicVersion = *logicEngine->rlogicVersion();

        LOG_INFO("Loading logic engine content from '{}' which was exported with Ramses {} and Logic Engine {}", dataSourceDescription, ramsesVersion.v_string()->string_view(), rlogicVersion.v_string()->string_view());

        if (!CheckRamsesVersionFromFile(ramsesVersion))
        {
            m_errors.add(fmt::format("Version mismatch while loading {}! Expected Ramses version {}.x.x but found {}",
                dataSourceDescription, ramses::GetRamsesVersion().major,
                ramsesVersion.v_string()->string_view()), nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        const auto featureLevel = static_cast<EFeatureLevel>(logicEngine->featureLevel());
        if (featureLevel != m_featureLevel)
        {
            m_errors.add(fmt::format("Feature level mismatch while loading {}! Loaded file with feature level {} but LogicEngine was instantiated with feature level {}",
                dataSourceDescription, featureLevel, m_featureLevel), nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        if (nullptr == logicEngine->apiObjects())
        {
            m_errors.add(fmt::format("Fatal error while loading {}: doesn't contain API objects!", dataSourceDescription), nullptr, EErrorType::BinaryVersionMismatch);
            return false;
        }

        if (logicEngine->assetMetadata())
        {
            LogAssetMetadata(*logicEngine->assetMetadata());
        }

        std::unique_ptr<IRamsesObjectResolver> ramsesResolver;
        if (scene != nullptr)
            ramsesResolver = std::make_unique<RamsesObjectResolver>(m_errors, *scene);

        std::unique_ptr<ApiObjects> deserializedObjects = ApiObjects::Deserialize(*logicEngine->apiObjects(), ramsesResolver.get(), dataSourceDescription, m_errors, m_featureLevel);

        if (!deserializedObjects)
        {
            return false;
        }

        // No errors -> move data into member
        m_apiObjects = std::move(deserializedObjects);

        return true;
    }

    bool LogicEngineImpl::GetFeatureLevelFromFile(std::string_view filename, EFeatureLevel& detectedFeatureLevel)
    {
        std::optional<std::vector<char>> maybeBytesFromFile = FileUtils::LoadBinary(std::string(filename));
        if (!maybeBytesFromFile)
        {
            LOG_ERROR("Failed to load file '{}'", filename);
            return false;
        }

        return GetFeatureLevelFromBuffer(filename, maybeBytesFromFile->data(), maybeBytesFromFile->size(), detectedFeatureLevel);
    }

    bool LogicEngineImpl::GetFeatureLevelFromBuffer(std::string_view logname, const void* buffer, size_t bufferSize, EFeatureLevel& detectedFeatureLevel)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast) Safe here, not worth transforming whole vector
        flatbuffers::Verifier bufferVerifier(reinterpret_cast<const uint8_t*>(buffer), bufferSize);
        if (!bufferVerifier.VerifyBuffer<rlogic_serialization::LogicEngine>())
        {
            LOG_ERROR("'{}' contains corrupted data", logname);
            return false;
        }

        const auto* logicEngine = rlogic_serialization::GetLogicEngine(buffer);
        if (!logicEngine)
        {
            LOG_ERROR("File '{}' could not be parsed", logname);
            return false;
        }

        const uint32_t featureLevelInt = logicEngine->featureLevel();
        if (std::find(AllFeatureLevels.cbegin(), AllFeatureLevels.cend(), featureLevelInt) == AllFeatureLevels.cend())
        {
            LOG_ERROR("Could not recognize feature level in file '{}'", logname);
            return false;
        }

        detectedFeatureLevel = static_cast<EFeatureLevel>(featureLevelInt);
        return true;
    }

    bool LogicEngineImpl::saveToFile(std::string_view filename, const SaveFileConfigImpl& config)
    {
        m_errors.clear();

        if (!m_apiObjects->checkBindingsReferToSameRamsesScene(m_errors))
        {
            m_errors.add("Can't save a logic engine to file while it has references to more than one Ramses scene!", nullptr, EErrorType::ContentStateError);
            return false;
        }

        if (config.getLuaSavingMode() == ELuaSavingMode::ByteCodeOnly && m_featureLevel == EFeatureLevel_01)
        {
            m_errors.add("Can't save logic content for feature level in binary mode, binary Lua support was introduced with FeatureLevel_02!", nullptr, EErrorType::IllegalArgument);
            return false;
        }

        // Refuse save() if logic graph has loops
        if (!m_apiObjects->getLogicNodeDependencies().getTopologicallySortedNodes())
        {
            m_errors.add("Failed to sort logic nodes based on links between their properties. Create a loop-free link graph before calling saveToFile()!", nullptr, EErrorType::ContentStateError);
            return false;
        }

        if (config.getValidationEnabled())
        {
            const std::vector<WarningData>& warnings = validate();

            if (!warnings.empty())
            {
                m_errors.add(
                    "Failed to saveToFile() because validation warnings were encountered! "
                    "Refer to the documentation of saveToFile() for details how to address these gracefully.", nullptr, EErrorType::ContentStateError);
                return false;
            }
        }

        const auto& scripts = m_apiObjects->getApiObjectContainer<LuaScript>();
        const auto sIt = std::find_if(scripts.cbegin(), scripts.cend(), [](const LuaScript* s) { return s->m_script.hasDebugLogFunctions(); });
        if (sIt != scripts.cend())
        {
            m_errors.add(fmt::format("Cannot save to file, Lua script '{}' has enabled debug log functions, remove this script before saving.", (*sIt)->m_impl.getIdentificationString()), *sIt, EErrorType::ContentStateError);
            return false;
        }
        const auto& modules = m_apiObjects->getApiObjectContainer<LuaModule>();
        const auto mIt = std::find_if(modules.cbegin(), modules.cend(), [](const LuaModule* m) { return m->m_impl.hasDebugLogFunctions(); });
        if (mIt != modules.cend())
        {
            m_errors.add(fmt::format("Cannot save to file, Lua module '{}' has enabled debug log functions, remove this module before saving.", (*mIt)->m_impl.getIdentificationString()), *mIt, EErrorType::ContentStateError);
            return false;
        }

        flatbuffers::FlatBufferBuilder builder;
        ramses::RamsesVersion ramsesVersion = ramses::GetRamsesVersion();

        const auto ramsesVersionOffset = rlogic_serialization::CreateVersion(builder,
            ramsesVersion.major,
            ramsesVersion.minor,
            ramsesVersion.patch,
            builder.CreateString(ramsesVersion.string));
        builder.Finish(ramsesVersionOffset);

        const auto ramsesLogicVersionOffset = rlogic_serialization::CreateVersion(builder,
            ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MAJOR_INT,
            ramses_sdk::RAMSES_SDK_PROJECT_VERSION_MINOR_INT,
            ramses_sdk::RAMSES_SDK_PROJECT_VERSION_PATCH_INT,
            builder.CreateString(ramses_sdk::RAMSES_SDK_RAMSES_VERSION));
        builder.Finish(ramsesLogicVersionOffset);

        const auto exporterVersionOffset = rlogic_serialization::CreateVersion(builder,
            config.getExporterMajorVersion(),
            config.getExporterMinorVersion(),
            config.getExporterPatchVersion(),
            builder.CreateString(""));
        builder.Finish(exporterVersionOffset);

        const auto assetMetadataOffset = rlogic_serialization::CreateMetadata(builder,
            builder.CreateString(config.getMetadataString()),
            exporterVersionOffset,
            config.getExporterFileFormatVersion());
        builder.Finish(assetMetadataOffset);

        const auto logicEngine = rlogic_serialization::CreateLogicEngine(builder,
            ramsesVersionOffset,
            ramsesLogicVersionOffset,
            ApiObjects::Serialize(*m_apiObjects, builder, config.getLuaSavingMode()),
            assetMetadataOffset,
            m_featureLevel);

        builder.Finish(logicEngine, getFileIdentifierMatchingFeatureLevel());

        if (!FileUtils::SaveBinary(std::string(filename), builder.GetBufferPointer(), builder.GetSize()))
        {
            m_errors.add(fmt::format("Failed to save content to path '{}'!", filename), nullptr, EErrorType::BinaryDataAccessError);
            return false;
        }

        LOG_INFO("Saved logic engine to file: '{}'.", filename);

        return true;
    }

    void LogicEngineImpl::LogAssetMetadata(const rlogic_serialization::Metadata& assetMetadata)
    {
        const std::string_view metadataString = assetMetadata.metadataString() ? assetMetadata.metadataString()->string_view() : "none";
        LOG_INFO("Logic Engine content metadata: '{}'", metadataString);
        const std::string exporterVersion = assetMetadata.exporterVersion() ?
            fmt::format("{}.{}.{} (file format version {})",
                assetMetadata.exporterVersion()->v_major(),
                assetMetadata.exporterVersion()->v_minor(),
                assetMetadata.exporterVersion()->v_patch(),
                assetMetadata.exporterFileVersion()) : "undefined";
        LOG_INFO("Exporter version: {}", exporterVersion);
    }

    bool LogicEngineImpl::link(const Property& sourceProperty, const Property& targetProperty)
    {
        m_errors.clear();

        return m_apiObjects->getLogicNodeDependencies().link(*sourceProperty.m_impl, *targetProperty.m_impl, false, m_errors);
    }

    bool LogicEngineImpl::linkWeak(const Property& sourceProperty, const Property& targetProperty)
    {
        m_errors.clear();

        return m_apiObjects->getLogicNodeDependencies().link(*sourceProperty.m_impl, *targetProperty.m_impl, true, m_errors);
    }

    bool LogicEngineImpl::unlink(const Property& sourceProperty, const Property& targetProperty)
    {
        m_errors.clear();

        return m_apiObjects->getLogicNodeDependencies().unlink(*sourceProperty.m_impl, *targetProperty.m_impl, m_errors);
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
        return LogicEngineReport{ std::make_unique<LogicEngineReportImpl>(m_updateReport, *m_apiObjects) };
    }

    void LogicEngineImpl::setStatisticsLoggingRate(size_t loggingRate)
    {
        m_statistics.setLoggingRate(loggingRate);
        m_statisticsEnabled = (loggingRate != 0u);
    }

    void LogicEngineImpl::setStatisticsLogLevel(ELogMessageType logLevel)
    {
        m_statistics.setLogLevel(logLevel);
    }

    size_t LogicEngineImpl::getTotalSerializedSize() const
    {
        return m_apiObjects->getTotalSerializedSize();
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
