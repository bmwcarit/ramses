//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/SceneGraph/Scene/ScenePersistation.h"
#include "internal/SceneGraph/SceneAPI/SceneCreationInformation.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"
#include "internal/SceneGraph/Scene/SceneDescriber.h"
#include "internal/SceneGraph/Scene/SceneActionCollectionCreator.h"
#include "internal/SceneGraph/Scene/SceneMergeHandleMapping.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/Scene/MergeScene.h"

#include <array>

namespace ramses::internal
{
    static const uint32_t gSceneMarker = 0x534d4152;  // {'R', 'A', 'M', 'S'}

    void ScenePersistation::ReadSceneMetadataFromStream(IInputStream& inStream, SceneCreationInformation& createInfo, EFeatureLevel featureLevel)
    {
        SceneId::BaseType sceneIdBaseType{};
        inStream >> sceneIdBaseType;
        createInfo.m_sceneInfo.sceneID = SceneId(sceneIdBaseType);

        SceneSizeInformation sizeInfo;
        inStream >> sizeInfo.nodeCount;
        inStream >> sizeInfo.cameraCount;
        inStream >> sizeInfo.transformCount;
        inStream >> sizeInfo.renderableCount;
        inStream >> sizeInfo.renderStateCount;
        inStream >> sizeInfo.datalayoutCount;
        inStream >> sizeInfo.datainstanceCount;
        if (featureLevel >= EFeatureLevel_02)
            inStream >> sizeInfo.uniformBufferCount;
        inStream >> sizeInfo.renderGroupCount;
        inStream >> sizeInfo.renderPassCount;
        inStream >> sizeInfo.renderTargetCount;
        inStream >> sizeInfo.renderBufferCount;
        inStream >> sizeInfo.textureSamplerCount;
        inStream >> sizeInfo.dataBufferCount;
        inStream >> sizeInfo.textureBufferCount;
        createInfo.m_sizeInfo = sizeInfo;

        inStream >> createInfo.m_sceneInfo.friendlyName;

        if (featureLevel >= EFeatureLevel_02)
        {
            inStream >> createInfo.m_sceneInfo.renderBackendCompatibility;
            inStream >> createInfo.m_sceneInfo.vulkanAPIVersion;
            inStream >> createInfo.m_sceneInfo.spirvVersion;
        }
    }

    void ScenePersistation::WriteSceneMetadataToStream(IOutputStream& outStream, const IScene& scene, EFeatureLevel featureLevel)
    {
        outStream << scene.getSceneId().getValue();

        const SceneSizeInformation& sizeInfo = scene.getSceneSizeInformation();
        outStream << sizeInfo.nodeCount;
        outStream << sizeInfo.cameraCount;
        outStream << sizeInfo.transformCount;
        outStream << sizeInfo.renderableCount;
        outStream << sizeInfo.renderStateCount;
        outStream << sizeInfo.datalayoutCount;
        outStream << sizeInfo.datainstanceCount;
        if (featureLevel >= EFeatureLevel_02)
            outStream << sizeInfo.uniformBufferCount;
        outStream << sizeInfo.renderGroupCount;
        outStream << sizeInfo.renderPassCount;
        outStream << sizeInfo.renderTargetCount;
        outStream << sizeInfo.renderBufferCount;
        outStream << sizeInfo.textureSamplerCount;
        outStream << sizeInfo.dataBufferCount;
        outStream << sizeInfo.textureBufferCount;

        outStream << scene.getName();

        if (featureLevel >= EFeatureLevel_02)
        {
            outStream << scene.getRenderBackendCompatibility();
            outStream << scene.getVulkanAPIVersion();
            outStream << scene.getSPIRVVersion();
        }
    }

    void ScenePersistation::WriteSceneToStream(IOutputStream& outStream, const ClientScene& scene, EFeatureLevel featureLevel)
    {
        SceneActionCollection collection;
        SceneActionCollectionCreator creator(collection, featureLevel);
        creator.preallocateSceneSize(scene.getSceneSizeInformation());
        SceneDescriber::describeScene<ClientScene>(scene, creator);

        const std::vector<std::byte>& actionData = collection.collectionData();

        outStream << static_cast<uint32_t>(gSceneMarker);
        outStream << static_cast<uint32_t>(collection.numberOfActions());
        outStream << static_cast<uint32_t>(actionData.size());

        // write data
        outStream.write(actionData.data(), static_cast<uint32_t>(actionData.size()));

        // write types and offsets
        for (const auto& reader : collection)
        {
            outStream << static_cast<uint32_t>(reader.type());
            outStream << static_cast<uint32_t>(reader.offsetInCollection());
        }
    }

    void ScenePersistation::WriteSceneToFile(std::string_view filename, const ClientScene& scene, EFeatureLevel featureLevel)
    {
        File f(filename);
        BinaryFileOutputStream stream(f);

        if (stream.getState() == EStatus::Ok)
        {
            ScenePersistation::WriteSceneToStream(stream, scene, featureLevel);
        }
        else
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ScenePersistation::WriteSceneToFile: failed to open stream");
        }
    }

    void ScenePersistation::ReadSceneFromStream(IInputStream& inStream, IScene& scene, EFeatureLevel featureLevel, SceneMergeHandleMapping* mapping)
    {
        uint32_t sceneMarker = 0;
        inStream >> sceneMarker;
        if (sceneMarker != gSceneMarker)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "ScenePersistation::ReadSceneFromStream:  could not load scene from file, its not marked as a scene");
            return;
        }

        uint32_t numberOfSceneActionsToRead = 0;
        inStream >> numberOfSceneActionsToRead;
        uint32_t sizeOfAllSceneActions = 0;
        inStream >> sizeOfAllSceneActions;

        SceneActionCollection actions(0, numberOfSceneActionsToRead);

        // read data
        std::vector<std::byte>& rawActionData = actions.getRawDataForDirectWriting();
        rawActionData.resize(sizeOfAllSceneActions);
        inStream.read(rawActionData.data(), rawActionData.size());

        std::array<uint32_t, NumOfSceneActionTypes> objectCounts = {};

        // read types and offsets
        for (uint32_t i = 0; i < numberOfSceneActionsToRead; ++i)
        {
            uint32_t actionType = 0;
            inStream >> actionType;
            uint32_t offsetInCollection = 0;
            inStream >> offsetInCollection;
            actions.addRawSceneActionInformation(static_cast<ESceneActionId>(actionType), offsetInCollection);
            ++objectCounts[actionType];
        }

        LOG_DEBUG_F(CONTEXT_PROFILING, ([&](StringOutputStream& sos) {
                    sos << "ScenePersistation::ReadSceneFromStream: SceneAction type counts for SceneID " << scene.getSceneId() << " (total: " << numberOfSceneActionsToRead << ")\n";
                    for (uint32_t i = 0; i < NumOfSceneActionTypes; i++)
                    {
                        if (objectCounts[i] > 0)
                        {
                            sos << "  " << GetNameForSceneActionId(static_cast<ESceneActionId>(i)) << " count: " << objectCounts[i] << "\n";
                        }
                    }
                }));

        if (mapping)
        {
            MergeScene mergeScene(scene, *mapping);
            SceneActionApplier::ApplyActionsOnScene(mergeScene, actions, featureLevel);
        }
        else
        {
            SceneActionApplier::ApplyActionsOnScene(scene, actions, featureLevel);
        }
    }

    void ScenePersistation::ReadSceneFromFile(std::string_view filename, IScene& scene, EFeatureLevel featureLevel, SceneMergeHandleMapping* mapping)
    {
        File f(filename);
        if (!f.exists())
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "ScenePersistation::ReadSceneFromFile:  could not find file to load scene from");
        }

        BinaryFileInputStream stream(f);

        const EStatus state = stream.getState();
        if (EStatus::Ok == state)
        {
            ScenePersistation::ReadSceneFromStream(stream, scene, featureLevel, mapping);
        }
        else
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "ScenePersistation::ReadSceneFromFile:  could not read scene from file");
        }
    }
}
