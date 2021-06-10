//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererResourceManagerMock.h"
#include "DeviceMock.h"
#include "PlatformAbstraction/FmtBase.h"
#include "MockResourceHash.h"

#include <numeric>

namespace ramses_internal {
using namespace testing;

RendererResourceManagerMock::RendererResourceManagerMock()
{
    // report common fake resources as uploaded by default
    ON_CALL(*this, getResourceDeviceHandle(MockResourceHash::EffectHash)).WillByDefault(Return(DeviceMock::FakeShaderDeviceHandle));
    ON_CALL(*this, getResourceDeviceHandle(MockResourceHash::VertArrayHash)).WillByDefault(Return(DeviceMock::FakeVertexBufferDeviceHandle));
    ON_CALL(*this, getResourceDeviceHandle(MockResourceHash::VertArrayHash2)).WillByDefault(Return(DeviceMock::FakeVertexBufferDeviceHandle));
    ON_CALL(*this, getResourceDeviceHandle(MockResourceHash::IndexArrayHash)).WillByDefault(Return(DeviceMock::FakeIndexBufferDeviceHandle));
    ON_CALL(*this, getResourceDeviceHandle(MockResourceHash::IndexArrayHash2)).WillByDefault(Return(DeviceMock::FakeIndexBufferDeviceHandle));
    ON_CALL(*this, getResourceDeviceHandle(MockResourceHash::IndexArrayHash3)).WillByDefault(Return(DeviceMock::FakeIndexBufferDeviceHandle));
    ON_CALL(*this, getResourceDeviceHandle(MockResourceHash::TextureHash)).WillByDefault(Return(DeviceMock::FakeTextureDeviceHandle));
    ON_CALL(*this, getResourceDeviceHandle(MockResourceHash::TextureHash2)).WillByDefault(Return(DeviceMock::FakeTextureDeviceHandle));

    ON_CALL(*this, getResourceStatus(MockResourceHash::EffectHash)).WillByDefault(Return(EResourceStatus::Uploaded));
    ON_CALL(*this, getResourceStatus(MockResourceHash::VertArrayHash)).WillByDefault(Return(EResourceStatus::Uploaded));
    ON_CALL(*this, getResourceStatus(MockResourceHash::VertArrayHash2)).WillByDefault(Return(EResourceStatus::Uploaded));
    ON_CALL(*this, getResourceStatus(MockResourceHash::IndexArrayHash)).WillByDefault(Return(EResourceStatus::Uploaded));
    ON_CALL(*this, getResourceStatus(MockResourceHash::IndexArrayHash2)).WillByDefault(Return(EResourceStatus::Uploaded));
    ON_CALL(*this, getResourceStatus(MockResourceHash::IndexArrayHash3)).WillByDefault(Return(EResourceStatus::Uploaded));
    ON_CALL(*this, getResourceStatus(MockResourceHash::TextureHash)).WillByDefault(Return(EResourceStatus::Uploaded));
    ON_CALL(*this, getResourceStatus(MockResourceHash::TextureHash2)).WillByDefault(Return(EResourceStatus::Uploaded));

    ON_CALL(*this, getResourceType(MockResourceHash::EffectHash)).WillByDefault(Return(EResourceType_Effect));
    ON_CALL(*this, getResourceType(MockResourceHash::VertArrayHash)).WillByDefault(Return(EResourceType_VertexArray));
    ON_CALL(*this, getResourceType(MockResourceHash::VertArrayHash2)).WillByDefault(Return(EResourceType_VertexArray));
    ON_CALL(*this, getResourceType(MockResourceHash::IndexArrayHash)).WillByDefault(Return(EResourceType_IndexArray));
    ON_CALL(*this, getResourceType(MockResourceHash::IndexArrayHash2)).WillByDefault(Return(EResourceType_IndexArray));
    ON_CALL(*this, getResourceType(MockResourceHash::IndexArrayHash3)).WillByDefault(Return(EResourceType_IndexArray));
    ON_CALL(*this, getResourceType(MockResourceHash::TextureHash)).WillByDefault(Return(EResourceType_Texture2D));
    ON_CALL(*this, getResourceType(MockResourceHash::TextureHash2)).WillByDefault(Return(EResourceType_Texture2D));

    ON_CALL(*this, getRenderTargetDeviceHandle(_, _)).WillByDefault(Return(DeviceMock::FakeRenderTargetDeviceHandle));
    ON_CALL(*this, getRenderTargetBufferDeviceHandle(_, _)).WillByDefault(Return(DeviceMock::FakeRenderBufferDeviceHandle));
    ON_CALL(*this, getOffscreenBufferDeviceHandle(_)).WillByDefault(Return(DeviceMock::FakeRenderTargetDeviceHandle));
    ON_CALL(*this, getOffscreenBufferColorBufferDeviceHandle(_)).WillByDefault(Return(DeviceMock::FakeRenderBufferDeviceHandle));
    ON_CALL(*this, getOffscreenBufferHandle(DeviceResourceHandle::Invalid())).WillByDefault(Return(OffscreenBufferHandle::Invalid()));
    ON_CALL(*this, getStreamBufferDeviceHandle(_)).WillByDefault(Return(DeviceMock::FakeRenderTargetDeviceHandle));
    ON_CALL(*this, getVertexArrayDeviceHandle(_, _)).WillByDefault(Return(DeviceMock::FakeVertexArrayDeviceHandle));

    ON_CALL(*this, getBlitPassRenderTargetsDeviceHandle(_, _, _, _)).WillByDefault(DoAll(SetArgReferee<2>(DeviceMock::FakeBlitPassRenderTargetDeviceHandle), SetArgReferee<3>(DeviceMock::FakeBlitPassRenderTargetDeviceHandle)));

    // no need to strictly test getters
    EXPECT_CALL(*this, getResourceDeviceHandle(_)).Times(AnyNumber());
    EXPECT_CALL(*this, getDataBufferDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getVertexArrayDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getTextureBufferDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getRenderTargetDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getRenderTargetBufferDeviceHandle(_, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getBlitPassRenderTargetsDeviceHandle(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(*this, getOffscreenBufferColorBufferDeviceHandle(_)).Times(AnyNumber());
    EXPECT_CALL(*this, getStreamBufferDeviceHandle(_)).Times(AnyNumber());
    EXPECT_CALL(*this, getResourcesInUseByScene(_)).Times(AnyNumber());
}

RendererResourceManagerRefCountMock::~RendererResourceManagerRefCountMock()
{
    expectNoResourceReferences();
}

void RendererResourceManagerRefCountMock::referenceResourcesForScene(SceneId sceneId, const ResourceContentHashVector& resources)
{
    for (const auto r : resources)
        m_refCounts[sceneId][r]++;
    RendererResourceManagerMock::referenceResourcesForScene(sceneId, resources);
}

void RendererResourceManagerRefCountMock::unreferenceResourcesForScene(SceneId sceneId, const ResourceContentHashVector& resources)
{
    for (const auto r : resources)
    {
        m_refCounts[sceneId][r]--;
        ASSERT_GE(m_refCounts[sceneId][r], 0) << fmt::format("Resource {} from scene {} has {} refs", r, sceneId, m_refCounts[sceneId][r]).c_str();
    }
    RendererResourceManagerMock::unreferenceResourcesForScene(sceneId, resources);
}

void RendererResourceManagerRefCountMock::unreferenceAllResourcesForScene(SceneId sceneId)
{
    m_refCounts.erase(sceneId);
    RendererResourceManagerMock::unreferenceAllResourcesForScene(sceneId);
}

const ResourceContentHashVector* RendererResourceManagerRefCountMock::getResourcesInUseByScene(SceneId sceneId) const
{
    RendererResourceManagerMock::getResourcesInUseByScene(sceneId);

    const auto it = m_refCounts.find(sceneId);
    if (it == m_refCounts.cend())
        return nullptr;

    m_tempUsedResources.clear();
    for (const auto& r : it->second)
        if (r.second > 0)
            m_tempUsedResources.push_back(r.first);

    return &m_tempUsedResources;
}

void RendererResourceManagerRefCountMock::expectNoResourceReferencesForScene(SceneId sceneId) const
{
    const auto it = m_refCounts.find(sceneId);
    if (it != m_refCounts.cend())
    {
        for (const auto& r : it->second)
        {
            EXPECT_EQ(0, r.second) << fmt::format("Resource {} from scene {} has {} refs", r.first, sceneId, r.second).c_str();
        }
    }
}

void RendererResourceManagerRefCountMock::expectNoResourceReferences() const
{
    for (const auto& sr : m_refCounts)
        expectNoResourceReferencesForScene(sr.first);
}

int RendererResourceManagerRefCountMock::getResourceRefCount(ResourceContentHash resource) const
{
    return std::accumulate(m_refCounts.cbegin(), m_refCounts.cend(), 0, [&](int refs, const auto& sceneRefsIt)
    {
        const auto it = sceneRefsIt.second.find(resource);
        if (it != sceneRefsIt.second.cend())
            refs += it->second;
        return refs;
    });
}

}
