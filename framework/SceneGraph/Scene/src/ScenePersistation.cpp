//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/ScenePersistation.h"
#include "SceneAPI/SceneCreationInformation.h"
#include "Scene/SceneActionApplier.h"
#include "Scene/SceneDescriber.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "Utils/File.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "Utils/LogMacros.h"
#include "Collections/String.h"
#include <array>
#include "Scene/ClientScene.h"

namespace ramses_internal
{
    static const UInt32 gSceneMarker = 0x534d4152;  // {'R', 'A', 'M', 'S'}

    void ScenePersistation::ReadSceneMetadataFromStream(IInputStream& inStream, SceneCreationInformation& createInfo)
    {
        SceneId::BaseType sceneIdBaseType;
        inStream >> sceneIdBaseType;
        createInfo.m_id = SceneId(sceneIdBaseType);

        SceneSizeInformation sizeInfo;
        inStream >> sizeInfo.nodeCount;
        inStream >> sizeInfo.cameraCount;
        inStream >> sizeInfo.transformCount;
        inStream >> sizeInfo.renderableCount;
        inStream >> sizeInfo.renderStateCount;
        inStream >> sizeInfo.datalayoutCount;
        inStream >> sizeInfo.datainstanceCount;
        inStream >> sizeInfo.renderGroupCount;
        inStream >> sizeInfo.renderPassCount;
        inStream >> sizeInfo.renderTargetCount;
        inStream >> sizeInfo.renderBufferCount;
        inStream >> sizeInfo.textureSamplerCount;
        inStream >> sizeInfo.dataBufferCount;
        inStream >> sizeInfo.animationSystemCount;
        inStream >> sizeInfo.textureBufferCount;
        createInfo.m_sizeInfo = sizeInfo;

        inStream >> createInfo.m_name;
    }

    void ScenePersistation::WriteSceneMetadataToStream(IOutputStream& outStream, const IScene& scene)
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
        outStream << sizeInfo.renderGroupCount;
        outStream << sizeInfo.renderPassCount;
        outStream << sizeInfo.renderTargetCount;
        outStream << sizeInfo.renderBufferCount;
        outStream << sizeInfo.textureSamplerCount;
        outStream << sizeInfo.dataBufferCount;
        outStream << sizeInfo.animationSystemCount;
        outStream << sizeInfo.textureBufferCount;

        outStream << scene.getName();
    }

    void ScenePersistation::WriteSceneToStream(IOutputStream& outStream, const ClientScene& scene)
    {
        SceneActionCollection collection;
        SceneActionCollectionCreator creator(collection);
        creator.preallocateSceneSize(scene.getSceneSizeInformation());
        SceneDescriber::describeScene<ClientScene>(scene, creator);

        const std::vector<Byte>& actionData = collection.collectionData();

        outStream << static_cast<UInt32>(gSceneMarker);
        outStream << static_cast<UInt32>(collection.numberOfActions());
        outStream << static_cast<UInt32>(actionData.size());

        // write data
        outStream.write(actionData.data(), static_cast<UInt32>(actionData.size()));

        // write types and offsets
        for (const auto& reader : collection)
        {
            outStream << static_cast<UInt32>(reader.type());
            outStream << static_cast<UInt32>(reader.offsetInCollection());
        }
    }

    void ScenePersistation::WriteSceneToFile(const String& filename, const ClientScene& scene)
    {
        File f(filename);
        BinaryFileOutputStream stream(f);

        if (stream.getState() == EStatus_RAMSES_OK)
        {
            ScenePersistation::WriteSceneToStream(stream, scene);
        }
        else
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ScenePersistation::WriteSceneToFile: failed to open stream");
        }
    }

    void ScenePersistation::ReadSceneFromStream(IInputStream& inStream, IScene& scene, AnimationSystemFactory* animSystemFactory)
    {
        UInt32 sceneMarker = 0;
        inStream >> sceneMarker;
        if (sceneMarker != gSceneMarker)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "ScenePersistation::ReadSceneFromStream:  could not load scene from file, its not marked as a scene");
            return;
        }

        UInt32 numberOfSceneActionsToRead = 0;
        inStream >> numberOfSceneActionsToRead;
        UInt32 sizeOfAllSceneActions = 0;
        inStream >> sizeOfAllSceneActions;

        SceneActionCollection actions(0, numberOfSceneActionsToRead);

        // read data
        std::vector<Byte>& rawActionData = actions.getRawDataForDirectWriting();
        rawActionData.resize(sizeOfAllSceneActions);
        inStream.read(reinterpret_cast<char*>(rawActionData.data()), static_cast<UInt32>(rawActionData.size()));

        std::array<uint32_t, ESceneActionId_NUMBER_OF_TYPES> objectCounts = {};

        // read types and offsets
        for (UInt32 i = 0; i < numberOfSceneActionsToRead; ++i)
        {
            UInt32 actionType = 0;
            inStream >> actionType;
            UInt32 offsetInCollection = 0;
            inStream >> offsetInCollection;
            actions.addRawSceneActionInformation(static_cast<ESceneActionId>(actionType), offsetInCollection);
            ++objectCounts[actionType];
        }

        LOG_DEBUG_F(ramses_internal::CONTEXT_PROFILING, ([&](ramses_internal::StringOutputStream& sos) {
                    sos << "ScenePersistation::ReadSceneFromStream: SceneAction type counts for SceneID " << scene.getSceneId().getValue() << " (total: " << numberOfSceneActionsToRead << ")\n";
                    for (uint32_t i = 0; i < ESceneActionId_NUMBER_OF_TYPES; i++)
                    {
                        if (objectCounts[i] > 0)
                        {
                            sos << "  " << GetNameForSceneActionId(static_cast<ESceneActionId>(i)) << " count: " << objectCounts[i] << "\n";
                        }
                    }
                }));

        SceneActionApplier::ApplyActionsOnScene(scene, actions, animSystemFactory);
    }

    void ScenePersistation::ReadSceneFromFile(const String& filename, IScene& scene, AnimationSystemFactory* animSystemFactory)
    {
        File f(filename);
        if (!f.exists())
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "ScenePersistation::ReadSceneFromFile:  could not find file to load scene from");
        }

        BinaryFileInputStream stream(f);

        const EStatus state = stream.getState();
        if (EStatus_RAMSES_OK == state)
        {
            ScenePersistation::ReadSceneFromStream(stream, scene, animSystemFactory);
        }
        else
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "ScenePersistation::ReadSceneFromFile:  could not read scene from file");
        }
    }
}
