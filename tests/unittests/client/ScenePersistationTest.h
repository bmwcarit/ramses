//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gtest/gtest.h"
#include "ramses/client/ramses-utils.h"
#include "ClientTestUtils.h"
#include "impl/SceneImpl.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/RamsesClientImpl.h"
#include "impl/RamsesFrameworkConfigImpl.h"
#include "impl/SaveFileConfigImpl.h"

#include <string_view>

namespace ramses::internal
{
    class ASceneLoadedFromFile : public LocalTestClientWithScene, public ::testing::Test
    {
    public:
        ASceneLoadedFromFile()
            : m_clientForLoading(*m_frameworkForLoader.createClient("client"))
        {
            m_frameworkForLoader.impl().getScenegraphComponent().setSceneRendererHandler(&sceneActionsCollector);
        }

        using ObjectTypeHistogram = ramses::internal::HashMap< ERamsesObjectType, uint32_t >;

        static void FillObjectTypeHistorgramFromObjectVector(ObjectTypeHistogram& counter, const SceneObjectVector& objects)
        {
            for (const auto obj : objects)
            {
                ERamsesObjectType type = obj->getType();
                uint32_t* pCount = counter.get(type);
                if( pCount )
                {
                    ++(*pCount);
                }
                else
                {
                    counter.put(type, 1u);
                }
            }
        }


        static void FillObjectTypeHistogramFromScene( ObjectTypeHistogram& counter, const ramses::Scene& scene )
        {
            SceneObjectVector objects;
            scene.impl().getObjectRegistry().getObjectsOfType(objects, ERamsesObjectType::SceneObject);
            FillObjectTypeHistorgramFromObjectVector( counter, objects );
        }

        static ::testing::AssertionResult AssertHistogramEqual( std::string_view m_expr, std::string_view n_expr, const ObjectTypeHistogram& m, const ObjectTypeHistogram& n )
        {
            bool isEqual = true;
            ::testing::AssertionResult wrongHistogramCount = ::testing::AssertionFailure();
            wrongHistogramCount << "Histogram differs\n";

            if( m.size() != n.size() )
            {
                wrongHistogramCount << m_expr << " and " << n_expr
                                    << " have different sizes " << m.size() << " and " << n.size() << "\n";

                wrongHistogramCount << m_expr << ":\n";
                for(auto iter : m)
                {
                        wrongHistogramCount << "Type " << RamsesObjectTypeUtils::GetRamsesObjectTypeName( iter.key ) << ":\t"
                                            << iter.value << "\n";
                }

                wrongHistogramCount << n_expr << ":\n";
                for(auto iter : n)
                {
                        wrongHistogramCount << "Type " << RamsesObjectTypeUtils::GetRamsesObjectTypeName( iter.key ) << ":\t"
                                            << iter.value << "\n";
                }

                isEqual = false;
            }
            else
            {
                for(auto iter = m.begin(); iter != m.end(); ++iter)
                {
                    if(*m.get(iter->key) != *n.get(iter->key))
                    {
                        wrongHistogramCount << "Type " << RamsesObjectTypeUtils::GetRamsesObjectTypeName(iter->key) << ":\t"
                                            << m_expr << " = " << *m.get(iter->key) << " while "
                                            << n_expr << " = " << *n.get(iter->key) << "\n";
                        isEqual = false;
                    }
                }
            }

            if (isEqual)
            {
                return ::testing::AssertionSuccess();
            }
            return wrongHistogramCount;
        }

        void checkSceneFile(const char* filename)
        {
            std::vector<std::byte> buffer;
            SaveFileConfigImpl config;
            config.setValidationEnabled(false);
            EXPECT_TRUE(m_scene.impl().serialize(buffer, config));
            EXPECT_FALSE(buffer.empty());

            ramses::internal::File f(filename);
            EXPECT_TRUE(f.open(ramses::internal::File::Mode::ReadOnlyBinary));
            size_t numBytes = 0u;
            EXPECT_TRUE(f.getSizeInBytes(numBytes));
            EXPECT_EQ(numBytes, buffer.size());
            std::vector<std::byte> fileBuffer;
            fileBuffer.resize(numBytes);
            EXPECT_EQ(ramses::internal::EStatus::Ok, f.read(fileBuffer.data(), numBytes, numBytes));
            EXPECT_TRUE(f.close());
            EXPECT_EQ(buffer, fileBuffer);
        }

        void doWriteReadCycle(bool expectSameSceneSizeInfo = true, bool expectSameTypeHistogram = true, bool withCompression = false)
        {
            SaveFileConfig config;
            config.setCompressionEnabled(withCompression);
            config.setValidationEnabled(false);
            ASSERT_TRUE(m_scene.saveToFile("someTemporaryFile.ram", config));

            checkSceneFile("someTemporaryFile.ram");

            m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram");
            ASSERT_TRUE(nullptr != m_sceneLoaded);

            if (expectSameTypeHistogram)
            {
                ObjectTypeHistogram origSceneNumbers;
                ObjectTypeHistogram loadedSceneNumbers;
                FillObjectTypeHistogramFromScene(origSceneNumbers, m_scene);
                FillObjectTypeHistogramFromScene(loadedSceneNumbers, *m_sceneLoaded);
                EXPECT_PRED_FORMAT2(AssertHistogramEqual, origSceneNumbers, loadedSceneNumbers);
            }

            // this can not be enabled for text serialization, because those are destroying/creating meshes
            if (expectSameSceneSizeInfo)
            {
                ramses::internal::SceneSizeInformation origSceneSizeInfo = m_scene.impl().getIScene().getSceneSizeInformation();
                ramses::internal::SceneSizeInformation loadedSceneSizeInfo = m_sceneLoaded->impl().getIScene().getSceneSizeInformation();

                EXPECT_EQ(origSceneSizeInfo, loadedSceneSizeInfo);
            }
        }

        static void SaveSceneWithFeatureLevelToFile(EFeatureLevel featureLevel, std::string_view fileName)
        {
            RamsesFrameworkConfig config{EFeatureLevel_Latest};
            config.impl().setFeatureLevelNoCheck(featureLevel);
            RamsesFramework someFramework{ config };
            EXPECT_EQ(featureLevel, someFramework.getFeatureLevel());
            RamsesClient* someClient = someFramework.createClient("someClient");

            ramses::Scene* scene = someClient->createScene(sceneId_t{ 123u });
            EXPECT_EQ(featureLevel, scene->getRamsesClient().getRamsesFramework().getFeatureLevel());
            EXPECT_TRUE(scene->saveToFile(fileName, {}));
        }

        template<typename T>
        T* getObjectForTesting(std::string_view name)
        {
            RamsesObject* objectPerName = this->m_sceneLoaded->findObject(name);
            EXPECT_TRUE(objectPerName != nullptr);
            if (!objectPerName)
                return nullptr;
            EXPECT_EQ(name, objectPerName->getName());

            T* specificObject = objectPerName->as<T>();
            EXPECT_TRUE(nullptr != specificObject);
            return specificObject;
        }

        ramses::RamsesFramework m_frameworkForLoader{ RamsesFrameworkConfig{EFeatureLevel_Latest} };
        ramses::RamsesClient& m_clientForLoading;
        ramses::Scene* m_sceneLoaded{nullptr};
    };

    class ASceneLoadedFromFileWithDefaultRenderPass : public ASceneLoadedFromFile
    {
    };

    template <typename T>
    class ASceneLoadedFromFileTemplated : public ASceneLoadedFromFile
    {
    };

}
