//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/SceneLinksManager.h"
#include "RendererLib/RendererScenes.h"
#include "RendererEventCollector.h"
#include "SceneUtils/ISceneDataArrayAccessor.h"
#include "SceneAllocateHelper.h"
#include "Utils/ThreadLocalLog.h"

using namespace testing;
using namespace ramses_internal;

class ADataReferenceLinkManager : public ::testing::Test
{
public:
    explicit ADataReferenceLinkManager(bool createSlots = true)
        : rendererScenes(rendererEventCollector)
        , sceneLinksManager(rendererScenes.getSceneLinksManager())
        , dataReferenceLinkManager(sceneLinksManager.getDataReferenceLinkManager())
        , providerSceneId(3u)
        , consumerSceneId(4u)
        , providerScene(rendererScenes.createScene(SceneInfo(providerSceneId)))
        , consumerScene(rendererScenes.createScene(SceneInfo(consumerSceneId)))
        , providerSceneAllocator(providerScene)
        , consumerSceneAllocator(consumerScene)
        , providerSlotHandle(55u)
        , consumerSlotHandle(66u)
        , providerId(33u)
        , consumerId(44u)
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);

        if (createSlots)
        {
            const DataLayoutHandle providerLayout = providerSceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
            providerDataRef = providerSceneAllocator.allocateDataInstance(providerLayout, DataInstanceHandle(3u));

            const DataLayoutHandle consumerLayout = consumerSceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
            consumerDataRef = consumerSceneAllocator.allocateDataInstance(consumerLayout, DataInstanceHandle(5u));

            providerSceneAllocator.allocateDataSlot({ EDataSlotType_DataProvider, providerId, NodeHandle(), providerDataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, providerSlotHandle);
            expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, providerSceneId, providerId, SceneId(0u), DataSlotId(0u));
            consumerSceneAllocator.allocateDataSlot({ EDataSlotType_DataConsumer, consumerId, NodeHandle(), consumerDataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, consumerSlotHandle);
            expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId);
        }
    }

protected:
    void expectRendererEvent(ERendererEventType event, SceneId providerSId, DataSlotId pId, SceneId consumerSId, DataSlotId cId)
    {
        RendererEventVector events;
        RendererEventVector dummy;
        rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(event, events.front().eventType);
        EXPECT_EQ(providerSId, events.front().providerSceneId);
        EXPECT_EQ(consumerSId, events.front().consumerSceneId);
        EXPECT_EQ(pId, events.front().providerdataId);
        EXPECT_EQ(cId, events.front().consumerdataId);
    }

    void expectRendererEvent(ERendererEventType event, SceneId consumerSId, DataSlotId cId, SceneId providerSId)
    {
        RendererEventVector events;
        RendererEventVector dummy;
        rendererEventCollector.appendAndConsumePendingEvents(dummy, events);
        ASSERT_EQ(1u, events.size());
        EXPECT_EQ(event, events.front().eventType);
        EXPECT_EQ(consumerSId, events.front().consumerSceneId);
        EXPECT_EQ(cId, events.front().consumerdataId);
        EXPECT_EQ(providerSId, events.front().providerSceneId);
    }

    void setDataValue(DataInstanceHandle dataRef, IScene& scene, float value)
    {
        scene.setDataSingleFloat(dataRef, DataFieldHandle(0u), value);
    }

    void expectDataValue(DataInstanceHandle dataRef, const IScene& scene, float value)
    {
        const float actualValue = scene.getDataSingleFloat(dataRef, DataFieldHandle(0u));
        EXPECT_EQ(value, actualValue);
    }

    RendererEventCollector rendererEventCollector;
    RendererScenes rendererScenes;
    SceneLinksManager& sceneLinksManager;
    const DataReferenceLinkManager& dataReferenceLinkManager;
    const SceneId providerSceneId;
    const SceneId consumerSceneId;
    IScene& providerScene;
    DataReferenceLinkCachedScene& consumerScene;
    SceneAllocateHelper providerSceneAllocator;
    SceneAllocateHelper consumerSceneAllocator;

    DataInstanceHandle providerDataRef;
    DataInstanceHandle consumerDataRef;

    const DataSlotHandle providerSlotHandle;
    const DataSlotHandle consumerSlotHandle;
    const DataSlotId providerId;
    const DataSlotId consumerId;
};

TEST_F(ADataReferenceLinkManager, failsToCreateLinkOfMismatchingDataTypes)
{
    const DataLayoutHandle providerLayout = providerSceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType::Int32) }, ResourceContentHash::Invalid());
    const DataInstanceHandle providerDataRef2 = providerSceneAllocator.allocateDataInstance(providerLayout, DataInstanceHandle(9u));

    const DataSlotId providerId2(999u);
    const DataSlotHandle slotHandle(43u);
    providerSceneAllocator.allocateDataSlot({ EDataSlotType_DataProvider, providerId2, NodeHandle(), providerDataRef2, ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
    expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, providerSceneId, providerId2, SceneId(0u), DataSlotId(0u));

    sceneLinksManager.createDataLink(providerSceneId, providerId2, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinkFailed, providerSceneId, providerId2, consumerSceneId, consumerId);
}

TEST_F(ADataReferenceLinkManager, canResolveLinkedData)
{
    setDataValue(providerDataRef, providerScene, 666.f);
    setDataValue(consumerDataRef, consumerScene, -1.f);

    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);

    expectDataValue(consumerDataRef, consumerScene, 666.f);
    expectDataValue(providerDataRef, providerScene, 666.f);
}

TEST_F(ADataReferenceLinkManager, canResolveLinkedDataToMultipleConsumers)
{
    const DataLayoutHandle consumerLayout = consumerSceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
    const DataInstanceHandle consumerDataRef2 = consumerSceneAllocator.allocateDataInstance(consumerLayout, DataInstanceHandle(9u));

    const DataSlotId consumerId2(999u);
    const DataSlotHandle slotHandle(43u);
    consumerSceneAllocator.allocateDataSlot({ EDataSlotType_DataConsumer, consumerId2, NodeHandle(), consumerDataRef2, ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
    expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId2);

    setDataValue(providerDataRef, providerScene, 666.f);
    setDataValue(consumerDataRef, consumerScene, -1.f);
    setDataValue(consumerDataRef2, consumerScene, -1.f);

    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId2);

    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);

    expectDataValue(consumerDataRef, consumerScene, 666.f);
    expectDataValue(consumerDataRef2, consumerScene, 666.f);
    expectDataValue(providerDataRef, providerScene, 666.f);
}

TEST_F(ADataReferenceLinkManager, doesNotResolveLinkedDataIfConsumerUnlinked)
{
    const DataLayoutHandle consumerLayout = consumerSceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
    const DataInstanceHandle consumerDataRef2 = consumerSceneAllocator.allocateDataInstance(consumerLayout, DataInstanceHandle(9u));

    const DataSlotId consumerId2(999u);
    const DataSlotHandle slotHandle(43u);
    consumerSceneAllocator.allocateDataSlot({ EDataSlotType_DataConsumer, consumerId2, NodeHandle(), consumerDataRef2, ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
    expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId2);

    setDataValue(providerDataRef, providerScene, 666.f);
    setDataValue(consumerDataRef, consumerScene, -1.f);
    setDataValue(consumerDataRef2, consumerScene, -1.f);

    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId2);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId, providerSceneId);

    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);

    expectDataValue(consumerDataRef, consumerScene, -1.f);
    expectDataValue(consumerDataRef2, consumerScene, 666.f);
    expectDataValue(providerDataRef, providerScene, 666.f);
}

TEST_F(ADataReferenceLinkManager, confidenceTest_canResolveLinkedWithThreeScenesAndTwoLinks)
{
    const SceneId middleSceneId(145u);
    DataReferenceLinkCachedScene& middleScene = rendererScenes.createScene(SceneInfo(middleSceneId));
    SceneAllocateHelper middleSceneAllocator(middleScene);

    DataInstanceHandle middleProviderDataRef;
    DataInstanceHandle middleConsumerDataRef;

    const DataLayoutHandle layout = middleSceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
    middleProviderDataRef = middleSceneAllocator.allocateDataInstance(layout, DataInstanceHandle(3u));
    middleConsumerDataRef = middleSceneAllocator.allocateDataInstance(layout, DataInstanceHandle(5u));

    const DataSlotHandle middleProviderSlotHandle(18u);
    const DataSlotHandle middleConsumerSlotHandle(19u);
    const DataSlotId middleProviderId(18u);
    const DataSlotId middleConsumerId(19u);
    middleSceneAllocator.allocateDataSlot({ EDataSlotType_DataProvider, middleProviderId, NodeHandle(), middleProviderDataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, middleProviderSlotHandle);
    expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, middleSceneId, middleProviderId, SceneId(0u), DataSlotId(0u));
    middleSceneAllocator.allocateDataSlot({ EDataSlotType_DataConsumer, middleConsumerId, NodeHandle(), middleConsumerDataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, middleConsumerSlotHandle);
    expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), middleSceneId, middleConsumerId);

    setDataValue(providerDataRef, providerScene, 666.f);
    setDataValue(middleConsumerDataRef, middleScene, -1.f);

    setDataValue(middleProviderDataRef, middleScene, 333.f);
    setDataValue(consumerDataRef, consumerScene, -1.f);

    sceneLinksManager.createDataLink(providerSceneId, providerId, middleSceneId, middleConsumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, middleSceneId, middleConsumerId);
    sceneLinksManager.createDataLink(middleSceneId, middleProviderId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, middleSceneId, middleProviderId, consumerSceneId, consumerId);

    dataReferenceLinkManager.resolveLinksForConsumerScene(middleScene);
    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);

    expectDataValue(providerDataRef, providerScene, 666.f);
    expectDataValue(middleConsumerDataRef, middleScene, 666.f);
    expectDataValue(middleProviderDataRef, middleScene, 333.f);
    expectDataValue(consumerDataRef, consumerScene, 333.f);
}

TEST_F(ADataReferenceLinkManager, unlinkedDataFallsBackToPreviousValue)
{
    setDataValue(providerDataRef, providerScene, 666.f);
    setDataValue(consumerDataRef, consumerScene, -1.f);

    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);

    expectDataValue(providerDataRef, providerScene, 666.f);
    expectDataValue(consumerDataRef, consumerScene, 666.f);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId, providerSceneId);

    expectDataValue(providerDataRef, providerScene, 666.f);
    expectDataValue(consumerDataRef, consumerScene, -1.f);
}

TEST_F(ADataReferenceLinkManager, dataFallsBackToPreviouslySetValueIfProviderSceneDestroyed)
{
    setDataValue(providerDataRef, providerScene, 666.f);
    setDataValue(consumerDataRef, consumerScene, -1.f);

    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);

    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);
    expectDataValue(providerDataRef, providerScene, 666.f);
    expectDataValue(consumerDataRef, consumerScene, 666.f);

    setDataValue(consumerDataRef, consumerScene, -333.f);

    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);
    expectDataValue(providerDataRef, providerScene, 666.f);
    expectDataValue(consumerDataRef, consumerScene, 666.f);

    rendererScenes.destroyScene(providerSceneId);

    expectDataValue(consumerDataRef, consumerScene, -333.f);
}

TEST_F(ADataReferenceLinkManager, confidenceTest_createTwoLinksChangeValueAndUnlink)
{
    const DataLayoutHandle consumerLayout = consumerSceneAllocator.allocateDataLayout({ DataFieldInfo(EDataType::Float) }, ResourceContentHash::Invalid());
    const DataInstanceHandle consumerDataRef2 = consumerSceneAllocator.allocateDataInstance(consumerLayout, DataInstanceHandle(9u));

    const DataSlotId consumerId2(999u);
    const DataSlotHandle slotHandle(43u);
    consumerSceneAllocator.allocateDataSlot({ EDataSlotType_DataConsumer, consumerId2, NodeHandle(), consumerDataRef2, ResourceContentHash::Invalid(), TextureSamplerHandle() }, slotHandle);
    expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), consumerSceneId, consumerId2);

    setDataValue(providerDataRef, providerScene, 666.f);
    setDataValue(consumerDataRef, consumerScene, -1.f);
    setDataValue(consumerDataRef2, consumerScene, -1.f);

    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId);
    sceneLinksManager.createDataLink(providerSceneId, providerId, consumerSceneId, consumerId2);
    expectRendererEvent(ERendererEventType::SceneDataLinked, providerSceneId, providerId, consumerSceneId, consumerId2);

    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);

    expectDataValue(consumerDataRef, consumerScene, 666.f);
    expectDataValue(consumerDataRef2, consumerScene, 666.f);
    expectDataValue(providerDataRef, providerScene, 666.f);

    setDataValue(providerDataRef, providerScene, 123.f);
    dataReferenceLinkManager.resolveLinksForConsumerScene(consumerScene);

    expectDataValue(consumerDataRef, consumerScene, 123.f);
    expectDataValue(consumerDataRef2, consumerScene, 123.f);
    expectDataValue(providerDataRef, providerScene, 123.f);

    sceneLinksManager.removeDataLink(consumerSceneId, consumerId);
    expectRendererEvent(ERendererEventType::SceneDataUnlinked, consumerSceneId, consumerId, providerSceneId);

    expectDataValue(consumerDataRef, consumerScene, -1.f);
    expectDataValue(consumerDataRef2, consumerScene, 123.f);
    expectDataValue(providerDataRef, providerScene, 123.f);
}

template <typename T>
class ADataReferenceLinkManagerTyped : public ADataReferenceLinkManager
{
public:
    ADataReferenceLinkManagerTyped()
        : ADataReferenceLinkManager(false)
    {
    }
};

using ItemTypes = ::testing::Types <
    Int32,
    Float,
    Vector2,
    Vector3,
    Vector4,
    Vector2i,
    Vector3i,
    Vector4i,
    Matrix22f,
    Matrix33f,
    Matrix44f
>;

TYPED_TEST_SUITE(ADataReferenceLinkManagerTyped, ItemTypes);

template <typename T>
T GetSomeValue()
{
    return T(5);
}

template <>
Matrix22f GetSomeValue<Matrix22f>()
{
    return Matrix22f(1,2,3,4);
}

template <>
Matrix33f GetSomeValue<Matrix33f>()
{
    return Matrix33f(1, 2, 3, 4, 5, 6, 7, 8, 9);
}

template <>
Matrix44f GetSomeValue<Matrix44f>()
{
    return Matrix44f(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
}

template <typename T>
T GetSomeValue2()
{
    return T(13);
}

template <>
Matrix22f GetSomeValue2<Matrix22f>()
{
    return Matrix22f(-1, -2, -3, -4);
}

template <>
Matrix33f GetSomeValue2<Matrix33f>()
{
    return Matrix33f(-1, -2, -3, -4, -5, -6, -7, -8, -9);
}

template <>
Matrix44f GetSomeValue2<Matrix44f>()
{
    return Matrix44f(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16);
}

template <typename T>
T GetSomeValue3()
{
    return T(-1513);
}

template <>
Matrix22f GetSomeValue3<Matrix22f>()
{
    return Matrix22f(-11, -22, -33, -44);
}

template <>
Matrix33f GetSomeValue3<Matrix33f>()
{
    return Matrix33f(-11, -21, -31, -41, -51, -61, -71, -81, -91);
}

template <>
Matrix44f GetSomeValue3<Matrix44f>()
{
    return Matrix44f(-12, -22, -32, -42, -52, -62, -72, -82, -92, -102, -112, -122, -132, -142, -152, -162);
}

TYPED_TEST(ADataReferenceLinkManagerTyped, confidenceTest_linkAndChangeAndUnlink)
{
    const DataLayoutHandle providerLayout = this->providerSceneAllocator.allocateDataLayout({ DataFieldInfo(TypeToEDataTypeTraits<TypeParam>::DataType) }, ResourceContentHash::Invalid());
    this->providerDataRef = this->providerSceneAllocator.allocateDataInstance(providerLayout, DataInstanceHandle(23u));

    const DataLayoutHandle consumerLayout = this->consumerSceneAllocator.allocateDataLayout({ DataFieldInfo(TypeToEDataTypeTraits<TypeParam>::DataType) }, ResourceContentHash::Invalid());
    this->consumerDataRef = this->consumerSceneAllocator.allocateDataInstance(consumerLayout, DataInstanceHandle(24u));

    this->providerSceneAllocator.allocateDataSlot({ EDataSlotType_DataProvider, this->providerId, NodeHandle(), this->providerDataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, this->providerSlotHandle);
    this->expectRendererEvent(ERendererEventType::SceneDataSlotProviderCreated, this->providerSceneId, this->providerId, SceneId(0u), DataSlotId(0u));
    this->consumerSceneAllocator.allocateDataSlot({ EDataSlotType_DataConsumer, this->consumerId, NodeHandle(), this->consumerDataRef, ResourceContentHash::Invalid(), TextureSamplerHandle() }, this->consumerSlotHandle);
    this->expectRendererEvent(ERendererEventType::SceneDataSlotConsumerCreated, SceneId(0u), DataSlotId(0u), this->consumerSceneId, this->consumerId);

    const TypeParam providerValue = GetSomeValue<TypeParam>();
    const TypeParam consumerValueInitial = GetSomeValue2<TypeParam>();
    const TypeParam consumerValueNew = GetSomeValue3<TypeParam>();

    // create link
    ISceneDataArrayAccessor::SetDataArray<TypeParam>(&this->providerScene, this->providerDataRef, DataFieldHandle(0u), 1u, &providerValue);
    ISceneDataArrayAccessor::SetDataArray<TypeParam>(&this->consumerScene, this->consumerDataRef, DataFieldHandle(0u), 1u, &consumerValueInitial);

    this->sceneLinksManager.createDataLink(this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType::SceneDataLinked, this->providerSceneId, this->providerId, this->consumerSceneId, this->consumerId);

    this->dataReferenceLinkManager.resolveLinksForConsumerScene(this->consumerScene);
    TypeParam actualProviderValue = ISceneDataArrayAccessor::GetDataArray<TypeParam>(&this->providerScene, this->providerDataRef, DataFieldHandle(0u))[0];
    TypeParam actualConsumerValue = ISceneDataArrayAccessor::GetDataArray<TypeParam>(&this->consumerScene, this->consumerDataRef, DataFieldHandle(0u))[0];
    EXPECT_EQ(providerValue, actualProviderValue);
    EXPECT_EQ(providerValue, actualConsumerValue);

    // change value on consumer while linked
    ISceneDataArrayAccessor::SetDataArray(&this->consumerScene, this->consumerDataRef, DataFieldHandle(0u), 1u, &consumerValueNew);
    this->dataReferenceLinkManager.resolveLinksForConsumerScene(this->consumerScene);
    actualProviderValue = ISceneDataArrayAccessor::GetDataArray<TypeParam>(&this->providerScene, this->providerDataRef, DataFieldHandle(0u))[0];
    actualConsumerValue = ISceneDataArrayAccessor::GetDataArray<TypeParam>(&this->consumerScene, this->consumerDataRef, DataFieldHandle(0u))[0];
    EXPECT_EQ(providerValue, actualProviderValue);
    EXPECT_EQ(providerValue, actualConsumerValue);

    // remove link
    this->sceneLinksManager.removeDataLink(this->consumerSceneId, this->consumerId);
    this->expectRendererEvent(ERendererEventType::SceneDataUnlinked, this->consumerSceneId, this->consumerId, this->providerSceneId);

    actualProviderValue = ISceneDataArrayAccessor::GetDataArray<TypeParam>(&this->providerScene, this->providerDataRef, DataFieldHandle(0u))[0];
    actualConsumerValue = ISceneDataArrayAccessor::GetDataArray<TypeParam>(&this->consumerScene, this->consumerDataRef, DataFieldHandle(0u))[0];
    EXPECT_EQ(providerValue, actualProviderValue);
    EXPECT_EQ(consumerValueNew, actualConsumerValue);
}
