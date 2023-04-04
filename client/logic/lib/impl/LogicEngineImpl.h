//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses-logic/EFeatureLevel.h"
#include "ramses-logic/ERotationType.h"
#include "ramses-logic/AnimationTypes.h"
#include "ramses-logic/LogicEngineReport.h"
#include "ramses-logic/DataTypes.h"
#include "internals/ApiObjects.h"
#include "internals/LogicNodeDependencies.h"
#include "internals/ErrorReporting.h"
#include "internals/ValidationResults.h"
#include "internals/UpdateReport.h"
#include "internals/LogicNodeUpdateStatistics.h"

#include "ramses-framework-api/RamsesFrameworkTypes.h"

#include <memory>
#include <vector>
#include <string>
#include <string_view>

namespace ramses
{
    class Scene;
    class SceneObject;
    class Node;
    class Appearance;
    class Camera;
    class RenderPass;
    class RenderGroup;
    class UniformInput;
    class MeshNode;
}

namespace rlogic
{
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
    class AnimationNodeConfig;
    class TimerNode;
    class AnchorPoint;
    class LuaScript;
    class LuaInterface;
    class LuaModule;
    class LogicNode;
    class Property;
    enum class ELogMessageType;
}

namespace rlogic_serialization
{
    struct Version;
    struct Metadata;
}

namespace rlogic::internal
{
    class LuaConfigImpl;
    class SaveFileConfigImpl;
    class LogicNodeImpl;
    class RamsesBindingImpl;
    class ApiObjects;

    class LogicEngineImpl
    {
    public:
        // Move-able (noexcept); Not copy-able

        // can't be noexcept anymore because constructor of std::unordered_map can throw
        explicit LogicEngineImpl(EFeatureLevel featureLevel);
        ~LogicEngineImpl() noexcept;

        LogicEngineImpl(LogicEngineImpl&& other) noexcept = default;
        LogicEngineImpl& operator=(LogicEngineImpl&& other) noexcept = default;
        LogicEngineImpl(const LogicEngineImpl& other)                = delete;
        LogicEngineImpl& operator=(const LogicEngineImpl& other) = delete;

        // Public API
        LuaScript* createLuaScript(std::string_view source, const LuaConfigImpl& config, std::string_view scriptName);
        LuaInterface* createLuaInterface(std::string_view source, const LuaConfigImpl& config, std::string_view interfaceName, bool verifyModules);
        LuaModule* createLuaModule(std::string_view source, const LuaConfigImpl& config, std::string_view moduleName);
        bool extractLuaDependencies(std::string_view source, const std::function<void(const std::string&)>& callbackFunc);
        RamsesNodeBinding* createRamsesNodeBinding(ramses::Node& ramsesNode, ERotationType rotationType, std::string_view name);
        RamsesAppearanceBinding* createRamsesAppearanceBinding(ramses::Appearance& ramsesAppearance, std::string_view name);
        RamsesCameraBinding* createRamsesCameraBinding(ramses::Camera& ramsesCamera, std::string_view name);
        RamsesCameraBinding* createRamsesCameraBindingWithFrustumPlanes(ramses::Camera& ramsesCamera, std::string_view name);
        RamsesRenderPassBinding* createRamsesRenderPassBinding(ramses::RenderPass& ramsesRenderPass, std::string_view name);
        RamsesRenderGroupBinding* createRamsesRenderGroupBinding(ramses::RenderGroup& ramsesRenderGroup, const RamsesRenderGroupBindingElements& elements, std::string_view name);
        RamsesMeshNodeBinding* createRamsesMeshNodeBinding(ramses::MeshNode& ramsesMeshNode, std::string_view name);
        SkinBinding* createSkinBinding(
            const std::vector<const RamsesNodeBinding*>& joints,
            const std::vector<matrix44f>& inverseBindMatrices,
            RamsesAppearanceBinding& appearanceBinding,
            const ramses::UniformInput& jointMatInput,
            std::string_view name);
        template <typename T>
        DataArray* createDataArray(const std::vector<T>& data, std::string_view name);
        AnimationNode* createAnimationNode(const AnimationNodeConfig& config, std::string_view name);
        TimerNode* createTimerNode(std::string_view name);
        AnchorPoint* createAnchorPoint(RamsesNodeBinding& nodeBinding, RamsesCameraBinding& cameraBinding, std::string_view name);

        bool destroy(LogicObject& object);

        bool update();

        [[nodiscard]] const std::vector<ErrorData>& getErrors() const;
        const std::vector<WarningData>& validate() const;

        bool loadFromFile(std::string_view filename, ramses::Scene* scene, bool enableMemoryVerification);
        bool loadFromFileDescriptor(int fd, size_t offset, size_t size, ramses::Scene* scene, bool enableMemoryVerification);
        bool loadFromBuffer(const void* rawBuffer, size_t bufferSize, ramses::Scene* scene, bool enableMemoryVerification);
        bool saveToFile(std::string_view filename, const SaveFileConfigImpl& config);
        [[nodiscard]] static bool GetFeatureLevelFromFile(std::string_view filename, EFeatureLevel& detectedFeatureLevel);
        [[nodiscard]] static bool GetFeatureLevelFromBuffer(std::string_view logname, const void* buffer, size_t bufferSize, EFeatureLevel& detectedFeatureLevel);

        bool link(const Property& sourceProperty, const Property& targetProperty);
        bool linkWeak(const Property& sourceProperty, const Property& targetProperty);
        bool unlink(const Property& sourceProperty, const Property& targetProperty);

        [[nodiscard]] bool isLinked(const LogicNode& logicNode) const;

        [[nodiscard]] EFeatureLevel getFeatureLevel() const;

        [[nodiscard]] ApiObjects& getApiObjects();

        // for benchmarking purposes only
        void disableTrackingDirtyNodes();

        void enableUpdateReport(bool enable);
        [[nodiscard]] LogicEngineReport getLastUpdateReport() const;

        void setStatisticsLoggingRate(size_t loggingRate);
        void setStatisticsLogLevel(ELogMessageType logLevel);

        [[nodiscard]] size_t getTotalSerializedSize() const;

        template<typename T>
        [[nodiscard]] size_t getSerializedSize() const;

    private:
        size_t activateLinksRecursive(PropertyImpl& output);
        void setNodeToBeAlwaysUpdatedDirty();

        static bool CheckRamsesVersionFromFile(const rlogic_serialization::Version& ramsesVersion);

        static void LogAssetMetadata(const rlogic_serialization::Metadata& assetMetadata);

        [[nodiscard]] bool updateNodes(const NodeVector& nodes);

        [[nodiscard]] bool loadFromByteData(const void* byteData, size_t byteSize, ramses::Scene* scene, bool enableMemoryVerification, const std::string& dataSourceDescription);
        [[nodiscard]] bool checkFileIdentifierBytes(const std::string& dataSourceDescription, const std::string& fileIdBytes);
        [[nodiscard]] const char* getFileIdentifierMatchingFeatureLevel() const;

        std::unique_ptr<ApiObjects> m_apiObjects;
        ErrorReporting m_errors;
        mutable ValidationResults m_validationResults;
        bool m_nodeDirtyMechanismEnabled = true;

        bool m_updateReportEnabled = false;
        bool m_statisticsEnabled   = true;
        UpdateReport m_updateReport;
        LogicNodeUpdateStatistics m_statistics;

        EFeatureLevel m_featureLevel;
    };

    template<typename T>
    size_t LogicEngineImpl::getSerializedSize() const
    {
        return m_apiObjects->getSerializedSize<T>();
    }
}
