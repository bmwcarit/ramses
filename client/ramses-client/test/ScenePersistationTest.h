//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEPERSISTATIONTEST_H
#define RAMSES_SCENEPERSISTATIONTEST_H

#include "gtest/gtest.h"
#include "ramses-client-api/ResourceFileDescriptionSet.h"
#include "ramses-utils.h"
#include "ClientTestUtils.h"
#include "SceneImpl.h"
#include "RamsesObjectTypeUtils.h"
#include "RamsesClientImpl.h"

namespace ramses
{
    class ASceneAndAnimationSystemLoadedFromFile : public LocalTestClientWithSceneAndAnimationSystem, public ::testing::Test
    {
    public:
        explicit ASceneAndAnimationSystemLoadedFromFile(uint32_t animationSystemCreationFlags = EAnimationSystemFlags_Default)
            : LocalTestClientWithSceneAndAnimationSystem(animationSystemCreationFlags)
            , m_clientForLoading("client", m_frameworkForLoader)
            , m_sceneLoaded(0)
            , m_animationSystemLoaded(0)
            , m_resources("someTemporaryResources.ramres")
        {
            m_frameworkForLoader.impl.getScenegraphComponent().setSceneRendererServiceHandler(&sceneActionsCollector);
        }

        ~ASceneAndAnimationSystemLoadedFromFile()
        {
        }

        typedef ramses_internal::HashMap< ERamsesObjectType, uint32_t > ObjectTypeHistogram;


        void fillObjectTypeHistorgramFromObjectVector(ObjectTypeHistogram& counter, const RamsesObjectVector& objects)
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


        void fillObjectTypeHistogramFromScene( ObjectTypeHistogram& counter, const ramses::Scene& scene )
        {
            RamsesObjectVector objects;
            scene.impl.getObjectRegistry().getObjectsOfType(objects, ERamsesObjectType_SceneObject);
            fillObjectTypeHistorgramFromObjectVector( counter, objects );
        }

        void fillObjectTypeHistogramFromClient( ObjectTypeHistogram& counter, const ramses::RamsesClient& cl )
        {
            fillObjectTypeHistorgramFromObjectVector( counter, cl.impl.getListOfResourceObjects() );
        }

        ::testing::AssertionResult AssertHistogramEqual( const char* m_expr, const char* n_expr, const ObjectTypeHistogram& m, const ObjectTypeHistogram& n )
        {
            bool isEqual = true;
            ::testing::AssertionResult wrongHistogramCount = ::testing::AssertionFailure();
            wrongHistogramCount << "Histogram differs\n";

            if( m.count() != n.count() )
            {
                wrongHistogramCount << m_expr << " and " << n_expr
                                    << " have different sizes " << m.count() << " and " << n.count() << "\n";

                wrongHistogramCount << m_expr << ":\n";
                for(ObjectTypeHistogram::ConstIterator iter = m.begin(); iter != m.end(); ++iter)
                {
                        wrongHistogramCount << "Type " << RamsesObjectTypeUtils::GetRamsesObjectTypeName( iter->key ) << ":\t"
                                            << iter->value << "\n";
                }

                wrongHistogramCount << n_expr << ":\n";
                for(ObjectTypeHistogram::ConstIterator iter = n.begin(); iter != n.end(); ++iter)
                {
                        wrongHistogramCount << "Type " << RamsesObjectTypeUtils::GetRamsesObjectTypeName( iter->key ) << ":\t"
                                            << iter->value << "\n";
                }

                isEqual = false;
            }
            else
            {
                for(ObjectTypeHistogram::ConstIterator iter = m.begin(); iter != m.end(); ++iter)
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

            if( isEqual )
            {
                return ::testing::AssertionSuccess();
            }
            else
            {
                return wrongHistogramCount;
            }
        }

        void doWriteReadCycle(bool expectSameSceneSizeInfo = true, bool expectSameTypeHistogram = true)
        {
            doWriteReadCycle(m_resources, expectSameSceneSizeInfo, expectSameTypeHistogram);
        }

        void doWriteReadCycle(const ResourceFileDescription& resources, bool expectSameSceneSizeInfo, bool expectSameTypeHistogram)
        {
            m_resourceVector.add(resources);
            doWriteReadCycle(m_resourceVector, expectSameSceneSizeInfo, expectSameTypeHistogram);
        }

        void doWriteReadCycle(const ResourceFileDescriptionSet& resourceVector, bool expectSameSceneSizeInfo, bool expectSameTypeHistogram)
        {
            const status_t status = client.saveSceneToFile(m_scene, "someTemporaryFile.ram", resourceVector, false);
            EXPECT_EQ(StatusOK, status);

            m_sceneLoaded = m_clientForLoading.loadSceneFromFile("someTemporaryFile.ram", resourceVector);
            ASSERT_TRUE(0 != m_sceneLoaded);

            if (expectSameTypeHistogram)
            {
                ObjectTypeHistogram origSceneNumbers;
                ObjectTypeHistogram loadedSceneNumbers;
                fillObjectTypeHistogramFromScene(origSceneNumbers, m_scene);
                fillObjectTypeHistogramFromClient(origSceneNumbers, client);

                fillObjectTypeHistogramFromScene(loadedSceneNumbers, *m_sceneLoaded);
                fillObjectTypeHistogramFromClient(loadedSceneNumbers, this->m_clientForLoading);
                EXPECT_PRED_FORMAT2(AssertHistogramEqual, origSceneNumbers, loadedSceneNumbers);
            }

            // this can not be enabled for text serialization, because those are destroying/creating meshes
            if (expectSameSceneSizeInfo)
            {
                ramses_internal::SceneSizeInformation origSceneSizeInfo = m_scene.impl.getIScene().getSceneSizeInformation();
                ramses_internal::SceneSizeInformation loadedSceneSizeInfo = m_sceneLoaded->impl.getIScene().getSceneSizeInformation();

                EXPECT_EQ(origSceneSizeInfo, loadedSceneSizeInfo);
            }

            m_animationSystemLoaded = RamsesUtils::TryConvert<AnimationSystem>(*m_sceneLoaded->findObjectByName("animation system"));
            ASSERT_TRUE(0 != m_animationSystemLoaded);
        }

        template<typename T>
        T* getObjectForTesting(const char* name)
        {
            RamsesObject* objectPerName = this->m_sceneLoaded->findObjectByName(name);
            EXPECT_TRUE(objectPerName != NULL);
            if (!objectPerName)
                return nullptr;
            EXPECT_STREQ(name, objectPerName->getName());

            T* specificObject = RamsesUtils::TryConvert<T>(*objectPerName);
            EXPECT_TRUE(0 != specificObject);
            return specificObject;
        }

        template<typename T>
        T* getAnimationObjectForTesting(const char* name)
        {
            RamsesObject* objectPerName = this->m_animationSystemLoaded->findObjectByName(name);
            EXPECT_TRUE(objectPerName != NULL);
            if (!objectPerName)
                return nullptr;
            EXPECT_STREQ(name, objectPerName->getName());

            T* specificObject = RamsesUtils::TryConvert<T>(*objectPerName);
            EXPECT_TRUE(0 != specificObject);
            return specificObject;
        }

        ramses::RamsesFramework m_frameworkForLoader;
        ramses::RamsesClient m_clientForLoading;
        ramses::Scene* m_sceneLoaded;
        ramses::AnimationSystem* m_animationSystemLoaded;
        ramses::ResourceFileDescription m_resources;
        ramses::ResourceFileDescriptionSet m_resourceVector;
    };

    class ASceneAndAnimationSystemLoadedFromFileWithDefaultRenderPass : public ASceneAndAnimationSystemLoadedFromFile
    {
    public:
        ASceneAndAnimationSystemLoadedFromFileWithDefaultRenderPass()
            : ASceneAndAnimationSystemLoadedFromFile(0)
        {
        }
    };

    template <typename T>
    class ASceneAndAnimationSystemLoadedFromFileTemplated : public ASceneAndAnimationSystemLoadedFromFile
    {
    public:
        ASceneAndAnimationSystemLoadedFromFileTemplated()
            : ASceneAndAnimationSystemLoadedFromFile(0)
        {
        }
    };

}

#endif
