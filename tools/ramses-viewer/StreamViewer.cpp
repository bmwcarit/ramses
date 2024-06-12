//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StreamViewer.h"
#include "ImguiWrapper.h"
#include "fmt/format.h"

constexpr const char* const vertexShader = R"##(
#version 300 es

in vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
}
)##";

constexpr const char* const fragmentShader = R"##(
#version 300 es

uniform sampler2D textureSampler;
out highp vec4 fragColor;
void main(void)
{
    highp vec2 ts = vec2(textureSize(textureSampler, 0));
    if(gl_FragCoord.x < ts.x && gl_FragCoord.y < ts.y)
    {
        fragColor = texelFetch(textureSampler,
#ifdef FLIP_Y
        ivec2(gl_FragCoord.xy),
#else
        ivec2(gl_FragCoord.x, ts.y-gl_FragCoord.y),
#endif
        0);
    }
    else
    {
        fragColor = vec4(0.2);
    }
}
)##";


namespace ramses::internal {

    StreamViewer::StreamViewer(ImguiClientHelper& imguiHelper, RamsesRenderer& renderer, displayId_t displayId)
        : m_imguiHelper(imguiHelper)
        , m_renderer(renderer)
        , m_displayId(displayId)
    {
        auto* scene = imguiHelper.getScene();
        // prepare triangle geometry: vertex position array and index array
        const std::array<ramses::vec2f, 4u> vertexPositionsArray{ ramses::vec2f{-1.0f, -1.0f}, ramses::vec2f{1.0f, -1.0f}, ramses::vec2f{-1.0f, 1.0f}, ramses::vec2f{1.0f, 1.0f} };
        auto* vertexPositions = scene->createArrayResource(4u, vertexPositionsArray.data());

        std::array<uint16_t, 6> indicesArray = {0, 1, 2, 2, 1, 3};
        auto* indices = scene->createArrayResource(6u, indicesArray.data());

        EffectDescription effectDesc;
        effectDesc.setVertexShader(vertexShader);
        effectDesc.setFragmentShader(fragmentShader);
        m_effect = scene->createEffect(effectDesc, "streambuf shader");

        m_appearance = scene->createAppearance(*m_effect);
        auto* geometry = scene->createGeometry(*m_effect);
        geometry->setInputBuffer(*m_effect->findAttributeInput("a_position"), *vertexPositions);
        geometry->setIndices(*indices);
        m_meshNode = scene->createMeshNode();
        m_meshNode->setAppearance(*m_appearance);
        m_meshNode->setGeometry(*geometry);
        m_meshNode->setVisibility(EVisibilityMode::Invisible);

        m_imguiHelper.getBackgroundGroup()->addMeshNode(*m_meshNode);
        scene->flush();
    }

    void StreamViewer::draw(bool defaultOpen)
    {
        if( m_streamEntries.empty())
            return;
        if (ImGui::CollapsingHeader("Stream Surfaces", defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0))
        {
            for (auto& it : m_streamEntries)
            {
                if (it.second.streamBuffer.isValid())
                {
                    bool isActive = m_activeStream == it.second.sampler;
                    bool update = ImGui::Checkbox(fmt::format("Ivi Surface: {}", it.first.getValue()).c_str(), &isActive);
                    ImVec2 textureSize(128, 128); // TODO: there seems to be no API to get the size
                    if (ImGui::ImageButton(it.second.sampler, textureSize, ImVec2(0, 0), ImVec2(1, 1)))
                    {
                        isActive = !isActive;
                        update = true;
                    }
                    if (update)
                        setActiveSurface(isActive ? it.first : waylandIviSurfaceId_t());
                }
            }
        }
    }

    void StreamViewer::streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available)
    {
        if (available)
        {
            auto& entry = findOrCreateStreamEntry(streamId);
            if (m_dataConsumers.find(entry.consumerId) != m_dataConsumers.end())
            {
                createAndLinkStreamBuffer(streamId, entry);
            }
        }
        else
        {
            auto it = m_streamEntries.find(streamId);
            if (it != m_streamEntries.end())
            {
                unlinkAndDestroyStreamBuffer(it->second);
            }
        }
    }

    void StreamViewer::dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId)
    {
        if (sceneId != m_imguiHelper.getScene()->getSceneId())
            return;
        auto it = std::find_if(m_streamEntries.begin(), m_streamEntries.end(), [&dataConsumerId](const auto& entry){ return entry.second.consumerId == dataConsumerId;});
        if (it == m_streamEntries.end())
            return; // unrelated data consumer

        m_dataConsumers.insert(dataConsumerId);
        assert(it->second.streamBuffer == streamBufferId_t());
        createAndLinkStreamBuffer(it->first, it->second);
    }

    void StreamViewer::dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId)
    {
        if (sceneId != m_imguiHelper.getScene()->getSceneId())
            return;
        m_dataConsumers.erase(dataConsumerId);
    }

    StreamViewer::StreamEntry& StreamViewer::findOrCreateStreamEntry(waylandIviSurfaceId_t surfaceId)
    {
        auto& entry = m_streamEntries[surfaceId];
        if (!entry.sampler)
        {
            auto consumer = m_imguiHelper.createTextureConsumer();
            entry.consumerId = consumer.first;
            entry.sampler = consumer.second;
        }
        return entry;
    }

    void StreamViewer::setActiveSurface(waylandIviSurfaceId_t surfaceId)
    {
        if (surfaceId.isValid())
        {
            m_activeStream = m_streamEntries[surfaceId].sampler;
            m_appearance->setInputTexture(*m_effect->findUniformInput("textureSampler"), *m_activeStream);
            m_meshNode->setVisibility(EVisibilityMode::Visible);
        }
        else
        {
            m_activeStream = nullptr;
            m_meshNode->setVisibility(EVisibilityMode::Invisible);
        }
    }

    void StreamViewer::createAndLinkStreamBuffer(waylandIviSurfaceId_t surfaceId, StreamEntry& entry)
    {
        if (m_autoShow)
            setActiveSurface(surfaceId);
        entry.streamBuffer = m_renderer.createStreamBuffer(m_displayId, surfaceId);
        m_renderer.getSceneControlAPI()->linkStreamBuffer(entry.streamBuffer, m_imguiHelper.getScene()->getSceneId(), entry.consumerId);
        m_renderer.flush();
        m_renderer.getSceneControlAPI()->flush();
    }

    void StreamViewer::unlinkAndDestroyStreamBuffer(StreamEntry &entry)
    {
        m_renderer.getSceneControlAPI()->unlinkData(m_imguiHelper.getScene()->getSceneId(), entry.consumerId);
        m_renderer.getSceneControlAPI()->flush();
        m_renderer.destroyStreamBuffer(m_displayId, entry.streamBuffer);
        entry.streamBuffer = streamBufferId_t();
        if (m_activeStream == entry.sampler)
        {
            auto it = std::find_if(m_streamEntries.begin(), m_streamEntries.end(), [](const auto& e){ return e.second.streamBuffer.isValid();});
            if (it != m_streamEntries.end())
            {
                setActiveSurface(it->first);
            }
            else
            {
                setActiveSurface({});
            }
        }
        m_renderer.flush();
    }
}
