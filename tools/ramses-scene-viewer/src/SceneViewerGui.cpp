//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  Copyright (C) 2019 Daniel Werner Lima Souza de Almeida (dwlsalmeida@gmail.com)
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneViewerGui.h"
#include "impl/SceneDumper.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/client/Scene.h"
#include "ramses/client/ramses-utils.h"

#include "impl/SceneImpl.h"
#include "impl/NodeImpl.h"
#include "impl/PickableObjectImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/CameraNodeImpl.h"
#include "impl/MeshNodeImpl.h"
#include "impl/RenderPassImpl.h"
#include "impl/RenderGroupImpl.h"
#include "impl/RenderTargetImpl.h"
#include "impl/RenderBufferImpl.h"
#include "impl/GeometryImpl.h"
#include "impl/ArrayBufferImpl.h"
#include "impl/EffectImpl.h"
#include "impl/Texture2DImpl.h"
#include "impl/Texture3DImpl.h"
#include "impl/Texture2DBufferImpl.h"
#include "impl/TextureCubeImpl.h"
#include "impl/TextureSamplerImpl.h"
#include "impl/DataObjectImpl.h"
#include "impl/RamsesClientImpl.h"
#include "impl/EffectInputImpl.h"
#include "impl/SceneReferenceImpl.h"
#include "impl/BlitPassImpl.h"
#include "impl/SceneObjectRegistryIterator.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "impl/TextureUtils.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/Core/Utils/File.h"
#include "glm/gtc/type_ptr.hpp"
#include "impl/TextureEnumsImpl.h"
namespace ramses::internal
{
    namespace
    {
        void getRotation(NodeImpl* node, glm::vec4& rot, ERotationType& rotationConventionInternal)
        {
            rotationConventionInternal = node->getIScene().getRotationType(node->getTransformHandle());
            rot = node->getIScene().getRotation(node->getTransformHandle());
        }

        void setRotation(NodeImpl* node, const glm::vec4& rot, ERotationType rotationType)
        {
            if (rotationType == ERotationType::Quaternion)
            {
                node->setRotation(ramses::quat(rot.w, rot.x, rot.y, rot.z));
            }
            else
            {
                node->setRotation({rot.x, rot.y, rot.z}, rotationType);
            }
        }

        ramses::EVisibilityMode getEffectiveVisibility(const NodeImpl& obj)
        {
            auto visibility = obj.getVisibility();
            for (auto parent = obj.getParentImpl(); parent != nullptr; parent = parent->getParentImpl())
            {
                visibility = std::min(visibility, parent->getVisibility());
                if (visibility == ramses::EVisibilityMode::Off)
                    break;
            }
            return visibility;
        }

        int32_t getRenderOrder(const RenderGroupImpl& rg, const SceneObjectImpl& child)
        {
            int32_t order = 0;
            switch (child.getType())
            {
            case ramses::ERamsesObjectType::MeshNode:
                rg.getMeshNodeOrder(static_cast<const MeshNodeImpl&>(child), order);
                break;
            case ramses::ERamsesObjectType::RenderGroup:
                rg.getRenderGroupOrder(static_cast<const RenderGroupImpl&>(child), order);
                break;
            default:
                assert(false);
                break;
            }
            return order;
        }

        const char* EnumToString(ramses::EDataType t)
        {
            switch (t)
            {
            case ramses::EDataType::Bool:
                return "Bool";
            case ramses::EDataType::Int32:
                return "Int32";
            case ramses::EDataType::UInt16:
                return "UInt16";
            case ramses::EDataType::UInt32:
                return "UInt32";
            case ramses::EDataType::Float:
                return "Float";
            case ramses::EDataType::Vector2F:
                return "Vector2F";
            case ramses::EDataType::Vector3F:
                return "Vector3F";
            case ramses::EDataType::Vector4F:
                return "Vector4F";
            case ramses::EDataType::Vector2I:
                return "Vector2I";
            case ramses::EDataType::Vector3I:
                return "Vector3I";
            case ramses::EDataType::Vector4I:
                return "Vector4I";
            case ramses::EDataType::Matrix22F:
                return "Matrix22F";
            case ramses::EDataType::Matrix33F:
                return "Matrix33F";
            case ramses::EDataType::Matrix44F:
                return "Matrix44F";
            case ramses::EDataType::ByteBlob:
                return "ByteBlob";
            case ramses::EDataType::TextureSampler2D:
                return "TextureSampler2D";
            case ramses::EDataType::TextureSampler2DMS:
                return "TextureSampler2DMS";
            case ramses::EDataType::TextureSampler3D:
                return "TextureSampler3D";
            case ramses::EDataType::TextureSamplerCube:
                return "TextureSamplerCube";
            case ramses::EDataType::TextureSamplerExternal:
                return "TextureSamplerExternal";
            }
            return nullptr;
        }

        const char* EnumToString(ramses::EVisibilityMode v)
        {
            switch (v)
            {
            case ramses::EVisibilityMode::Off:
                return "Off";
            case ramses::EVisibilityMode::Visible:
                return "Visible";
            case ramses::EVisibilityMode::Invisible:
                return "Invisible";
            }
            return "n.a.";
        }

        const char* shortName(ramses::internal::EDataType t) {
            const char* name = EnumToString(t);
            assert(std::string(name).find("DATATYPE_") == 0);
            return name + 9;
        }

        const char* shortName(ramses::ERamsesObjectType t)
        {
            return RamsesObjectTypeUtils::GetRamsesObjectTypeName(t);
        }

        const char* EnumToString(ramses::EIssueType issueType)
        {
            switch (issueType)
            {
            case ramses::EIssueType::Warning:
                return "Warning";
            case ramses::EIssueType::Error:
                return "Error";
            }
            return "";
        }

    } // namespace

    template<class C>
    bool SceneViewerGui::drawRamsesObject(SceneObjectImpl& obj, const C& drawTreeNode)
    {
        ramses::ValidationReport report;
        obj.getRamsesObject().validate(report);
        const bool hasIssues = report.hasIssue();
        const bool isUnused = !m_usedObjects.contains(&obj);
        if (report.hasError())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 0, 0).Value);
        }
        else if (hasIssues)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 0).Value);
        }
        else if (isUnused)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(127, 127, 127).Value);
        }
        const bool isOpen = drawTreeNode();

        if (ImGui::BeginPopupContextItem(obj.getName().c_str()))
        {
            if (ImGui::MenuItem("Copy name"))
            {
                ImGui::LogToClipboard();
                ImGui::LogText("%s", obj.getName().c_str());
                ImGui::LogFinish();
            }
            ImGui::EndPopup();
        }

        if (hasIssues)
        {
            if (isOpen)
                DrawIssues(obj, report);
            ImGui::PopStyleColor();
        }
        else if (isUnused)
        {
            ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Unnecessary object");
            if (isOpen)
                drawUnusedObject(obj);
        }
        return isOpen;
    }

    bool SceneViewerGui::drawRamsesObject(SceneObjectImpl& obj)
    {
        return drawRamsesObject(obj, [&]() {
            return ImGui::TreeNode(&obj, "%s[%lu]: %s", shortName(obj.getType()), obj.getSceneObjectId().getValue(), obj.getName().c_str());
            });
    }

    void SceneViewerGui::DrawIssues(const SceneObjectImpl& obj, const ramses::ValidationReport& report)
    {
        for (auto& m : report.getIssues())
        {
            if (m.object == &obj.getRamsesObject() && m.type <= ramses::EIssueType::Warning)
            {
                switch (m.type)
                {
                case ramses::EIssueType::Error:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 0, 0).Value);
                    break;
                case ramses::EIssueType::Warning:
                    ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 255, 0).Value);
                    break;
                }
                ImGui::BulletText("%s", m.message.c_str());
                ImGui::PopStyleColor();
            }
        }
    }

    void SceneViewerGui::drawUnusedObject(SceneObjectImpl& obj)
    {
        if (obj.isOfType(ramses::ERamsesObjectType::Resource))
        {
            ImGui::Text("Unused or duplicate resource");
            if (ImGui::TreeNode("Duplicates (same hash):"))
            {
                m_resourceInfo->reloadIfEmpty();
                auto& hlResource = static_cast<ResourceImpl&>(obj);
                auto range = m_resourceInfo->equal_range(hlResource.getLowlevelResourceHash());
                for (auto it = range.first; it != range.second; ++it)
                {
                    draw(it->second->impl());
                }
                ImGui::TreePop();
            }
        }
        else
        {
            switch (obj.getType())
            {
            case ramses::ERamsesObjectType::Node:
                ImGui::Text("Unused node (not a parent of a mesh node or camera node)");
                break;
            default:
                ImGui::Text("Unused object");
                break;
            }
        }
    }

    SceneViewerGui::SceneViewerGui(ramses::Scene& scene, const std::string& filename, ImguiClientHelper& imguiHelper, const ramses::ValidationReport& report)
        : m_scene(scene)
        , m_validationReport(report)
        , m_loadedSceneFile(filename)
        , m_filename(filename)
        , m_imageCache(imguiHelper.getScene())
    {
        ramses::internal::StringOutputStream dummyStream;
        SceneDumper sceneDumper(scene.impl());
        sceneDumper.dumpUnrequiredObjects(dummyStream);
        m_usedObjects = sceneDumper.getRequiredObjects();
        m_resourceInfo = std::make_unique<ResourceList>(scene, m_usedObjects);
        ImGuiIO& io = ImGui::GetIO();
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }

    const ramses::RenderBuffer* SceneViewerGui::findRenderBuffer(ramses::internal::RenderBufferHandle renderBufferHandle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isRenderBufferAllocated(renderBufferHandle);
        if (isAllocated)
        {
            SceneObjectRegistryIterator iter(m_scene.impl().getObjectRegistry(), ramses::ERamsesObjectType::RenderBuffer);
            while (const auto* renderBuffer = iter.getNext<ramses::RenderBuffer>())
            {
                if (renderBufferHandle == renderBuffer->impl().getRenderBufferHandle())
                {
                    return renderBuffer;
                }
            }
        }
        return nullptr;
    }

    const ramses::Texture2DBuffer* SceneViewerGui::findTextureBuffer(ramses::internal::TextureBufferHandle handle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isTextureBufferAllocated(handle);
        if (isAllocated)
        {
            SceneObjectRegistryIterator iter(m_scene.impl().getObjectRegistry(), ramses::ERamsesObjectType::Texture2DBuffer);
            while (const auto* textureBuffer = iter.getNext<ramses::Texture2DBuffer>())
            {
                if (handle == textureBuffer->impl().getTextureBufferHandle())
                {
                    return textureBuffer;
                }
            }
        }
        return nullptr;
    }

    const ramses::TextureSampler* SceneViewerGui::findTextureSampler(ramses::internal::TextureSamplerHandle handle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isTextureSamplerAllocated(handle);
        if (isAllocated)
        {
            SceneObjectRegistryIterator iter(m_scene.impl().getObjectRegistry(), ramses::ERamsesObjectType::TextureSampler);
            while (const auto* textureSampler = iter.getNext<ramses::TextureSampler>())
            {
                if (handle == textureSampler->impl().getTextureSamplerHandle())
                {
                    return textureSampler;
                }
            }
        }
        return nullptr;
    }

    const ramses::ArrayBuffer* SceneViewerGui::findArrayBuffer(ramses::internal::DataBufferHandle handle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isDataBufferAllocated(handle);
        if (isAllocated)
        {
            SceneObjectRegistryIterator iter(m_scene.impl().getObjectRegistry(), ramses::ERamsesObjectType::ArrayBuffer);
            while (const auto* dataBuffer = iter.getNext<ramses::ArrayBuffer>())
            {
                if (handle == dataBuffer->impl().getDataBufferHandle())
                {
                    return dataBuffer;
                }
            }
        }
        return nullptr;
    }

    const ramses::Node* SceneViewerGui::findNode(ramses::internal::NodeHandle handle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isNodeAllocated(handle);
        if (isAllocated)
        {
            SceneObjectRegistryIterator iter(m_scene.impl().getObjectRegistry(), ramses::ERamsesObjectType::Node);
            while (const auto* node = iter.getNext<ramses::Node>())
            {
                if (handle == node->impl().getNodeHandle())
                {
                    return node;
                }
            }
        }
        return nullptr;
    }

    const ramses::DataObject* SceneViewerGui::findDataObject(ramses::internal::DataInstanceHandle handle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isDataInstanceAllocated(handle);
        if (isAllocated)
        {
            SceneObjectVector objects;
            m_scene.impl().getObjectRegistry().getObjectsOfType(objects, ramses::ERamsesObjectType::DataObject);
            for (auto & object : objects)
            {
                if (handle == static_cast<ramses::DataObject*>(object)->impl().getDataReference())
                {
                    return static_cast<ramses::DataObject*>(object);
                }
            }
        }
        return nullptr;
    }

    const ramses::Texture2D* SceneViewerGui::findTexture2D(ramses::internal::ResourceContentHash hash) const
    {
        if (hash.isValid())
        {
            SceneObjectRegistryIterator iter(m_scene.impl().getObjectRegistry(), ramses::ERamsesObjectType::Texture2D);
            while (const auto* texture = iter.getNext<ramses::Texture2D>())
            {
                if (texture->impl().getLowlevelResourceHash() == hash)
                {
                    return texture;
                }
            }
        }
        return nullptr;
    }

    const ramses::internal::DataSlot* SceneViewerGui::findDataSlot(ramses::internal::NodeHandle handle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isNodeAllocated(handle);
        if (isAllocated)
        {
            const auto& slots = m_scene.impl().getIScene().getDataSlots();
            for (auto it : slots)
            {
                if (it.second->attachedNode == handle)
                {
                    return it.second;
                }
            }
        }
        return nullptr;
    }

    const ramses::internal::DataSlot* SceneViewerGui::findDataSlot(ramses::internal::DataInstanceHandle handle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isDataInstanceAllocated(handle);
        if (isAllocated)
        {
            const auto& slots = m_scene.impl().getIScene().getDataSlots();
            for (auto it : slots)
            {
                if (it.second->attachedDataReference == handle)
                {
                    return it.second;
                }
            }
        }
        return nullptr;
    }

    const ramses::internal::DataSlot* SceneViewerGui::findDataSlot(ramses::internal::TextureSamplerHandle handle) const
    {
        const bool isAllocated = m_scene.impl().getIScene().isTextureSamplerAllocated(handle);
        if (isAllocated)
        {
            const auto& slots = m_scene.impl().getIScene().getDataSlots();
            for (auto it : slots)
            {
                if (it.second->attachedTextureSampler == handle)
                {
                    return it.second;
                }
            }
        }
        return nullptr;
    }

    const ramses::internal::DataSlot* SceneViewerGui::findDataSlot(ramses::internal::ResourceContentHash hash) const
    {
        if (hash.isValid())
        {
            const auto& slots = m_scene.impl().getIScene().getDataSlots();
            for (auto it : slots)
            {
                if (it.second->attachedTexture == hash)
                {
                    return it.second;
                }
            }
        }
        return nullptr;
    }

    template<class T, class Filter>
    void SceneViewerGui::drawRefs(const char* headline, const SceneObjectImpl& target, Filter filter)
    {
        const RefKey key = {&target, headline};
        auto   result = m_refs.insert({ key, SceneObjectVector{} });

        SceneObjectVector& filteredList = result.first->second;
        if (result.second)
        {
            // fill the list initially
            const auto type = ramses::TYPE_ID_OF_RAMSES_OBJECT<T>::ID;
            SceneObjectVector objects;
            m_scene.impl().getObjectRegistry().getObjectsOfType(objects, type);
            for (auto* obj : objects)
            {
                const T* tObj = static_cast<const T*>(obj);
                if (filter(tObj))
                    filteredList.push_back(obj);
            }
        }

        if (!filteredList.empty())
        {
            if (ImGui::TreeNode(headline, "%s (%zu):", headline, filteredList.size()))
            {
                for (auto* obj : filteredList)
                {
                    draw(obj->impl());
                }
                ImGui::TreePop();
            }
        }
    }

    void SceneViewerGui::draw(SceneObjectImpl& obj)
    {
        if (drawRamsesObject(obj))
        {
            switch (obj.getType())
            {
            case ramses::ERamsesObjectType::Node:
                drawNode(static_cast<NodeImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::PickableObject:
                drawPickableObject(static_cast <PickableObjectImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::MeshNode:
                drawMeshNode(static_cast<MeshNodeImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::PerspectiveCamera:
            case ramses::ERamsesObjectType::OrthographicCamera:
                drawCameraNode(static_cast<CameraNodeImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::Effect:
                drawEffect(static_cast<EffectImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::RenderPass:
                drawRenderPass(static_cast<RenderPassImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::RenderGroup:
                drawRenderGroup(static_cast<RenderGroupImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::Appearance:
                drawAppearance(static_cast<ramses::Appearance&>(obj.getRamsesObject()));
                break;
            case ramses::ERamsesObjectType::Geometry:
                drawGeometry(static_cast<GeometryImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::Texture2D:
                drawTexture2D(static_cast<Texture2DImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::Texture3D:
                drawTexture3D(static_cast<Texture3DImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::Texture2DBuffer:
                drawTexture2DBuffer(static_cast<Texture2DBufferImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::TextureCube:
                drawTextureCube(static_cast<TextureCubeImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::TextureSampler:
            case ramses::ERamsesObjectType::TextureSamplerMS:
            case ramses::ERamsesObjectType::TextureSamplerExternal:
                drawTextureSampler(static_cast<TextureSamplerImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::ArrayResource:
                drawArrayResource(static_cast<ArrayResourceImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::RenderTarget:
                drawRenderTarget(static_cast<RenderTargetImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::RenderBuffer:
                drawRenderBuffer(static_cast<RenderBufferImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::SceneReference:
                DrawSceneReference(static_cast<SceneReferenceImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::DataObject:
                drawDataObject(static_cast<ramses::DataObject&>(obj.getRamsesObject()));
                break;
            case ramses::ERamsesObjectType::ArrayBuffer:
                drawArrayBuffer(static_cast<ArrayBufferImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::BlitPass:
                drawBlitPass(static_cast<BlitPassImpl&>(obj));
                break;
            case ramses::ERamsesObjectType::Invalid:
            case ramses::ERamsesObjectType::ClientObject:
            case ramses::ERamsesObjectType::RamsesObject:
            case ramses::ERamsesObjectType::SceneObject:
            case ramses::ERamsesObjectType::Client:
            case ramses::ERamsesObjectType::Scene:
            case ramses::ERamsesObjectType::Camera:
            case ramses::ERamsesObjectType::Resource:
            case ramses::ERamsesObjectType::LogicEngine:
            case ramses::ERamsesObjectType::LogicObject:
                ImGui::Text("tbd.");
            }
            ImGui::TreePop();
        }
    }

    void SceneViewerGui::drawNode(NodeImpl& obj)
    {
        int vis = static_cast<int>(obj.getVisibility());
        if (ImGui::RadioButton("Visible", &vis, static_cast<int>(ramses::EVisibilityMode::Visible)))
            setVisibility(obj, ramses::EVisibilityMode::Visible);
        ImGui::SameLine();
        if (ImGui::RadioButton("Invisible", &vis, static_cast<int>(ramses::EVisibilityMode::Invisible)))
            setVisibility(obj, ramses::EVisibilityMode::Invisible);
        ImGui::SameLine();
        if (ImGui::RadioButton("Off", &vis, static_cast<int>(ramses::EVisibilityMode::Off)))
            setVisibility(obj, ramses::EVisibilityMode::Off);
        const auto effectiveVisibility = getEffectiveVisibility(obj);
        if (obj.getVisibility() != effectiveVisibility)
        {
            ImGui::SameLine();
            ImGui::Text("(-> %s)", EnumToString(effectiveVisibility));
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Parent(s) limit visibility");
        }
        const auto* slot = findDataSlot(obj.getNodeHandle());
        if (slot)
            ImGui::Text("DataSlot: %u %s", slot->id.getValue(), EnumToString(slot->type));
        if (ImGui::TreeNode("Transformation"))
        {
            glm::vec4 rot;
            ERotationType rotationType;
            getRotation(&obj, rot, rotationType);
            if (rotationType == ERotationType::Quaternion)
            {
                if (ImGui::DragFloat4("Rotation (x,y,z,w)", glm::value_ptr(rot), .01f, -1.f, 1.f, "%.6f"))
                    setRotation(&obj, rot, rotationType);
            }
            else
            {
                if (ImGui::DragFloat3("Rotation (x,y,z)", glm::value_ptr(rot), 1.0f, -360.f, 360.f, "%.1f"))
                    setRotation(&obj, rot, rotationType);
            }
            int rotationConventionInt = static_cast<int>(rotationType);
            if (ImGui::Combo("RotationConvention", &rotationConventionInt, ERotationTypeNames.data(), static_cast<int>(ERotationTypeNames.size()), -1))
                setRotation(&obj, rot, static_cast<ERotationType>(rotationConventionInt));
            ramses::vec3f xyz;
            obj.getTranslation(xyz);
            if (ImGui::DragFloat3("Translation (x,y,z)", glm::value_ptr(xyz), 0.01f, 0.f, 0.f, "%.3f"))
                obj.setTranslation(xyz);
            obj.getScaling(xyz);
            if (ImGui::DragFloat3("Scaling (x,y,z)", glm::value_ptr(xyz), 0.01f, 0.f, 0.f, "%.3f"))
                obj.setScaling(xyz);
            ImGui::TreePop();
        }
        if (ramses::ERamsesObjectType::Node == obj.getType())
        {
            drawNodeChildrenParent(obj);
        }
    }

    void SceneViewerGui::drawNodeChildrenParent(NodeImpl& obj)
    {
        for (uint32_t i = 0; i < obj.getChildCount(); ++i)
        {
            draw(obj.getChild(i)->impl());
        }
        if (obj.getParentImpl() != nullptr)
        {
            ImGui::Text("Parent:");
            ImGui::SameLine();
            draw(*obj.getParentImpl());
        }
    }

    void SceneViewerGui::drawMeshNode(MeshNodeImpl& obj)
    {
        drawNode(obj);
        if (obj.getAppearance())
            draw(obj.getAppearance()->impl());
        if (obj.getGeometry())
            draw(obj.getGeometry()->impl());
        drawNodeChildrenParent(obj);
        drawRefs<ramses::RenderGroup>("Used by RenderGroup", obj, [&](const ramses::RenderGroup* ref) { return ref->impl().contains(obj); });
    }

    void SceneViewerGui::drawPickableObject(PickableObjectImpl& obj)
    {
        drawNode(obj);
        draw(const_cast<ramses::ArrayBuffer&>(obj.getGeometryBuffer()).impl());
        auto camera = const_cast<ramses::Camera*>(obj.getCamera());
        if (camera != nullptr)
            draw(camera->impl());
        drawNodeChildrenParent(obj);
    }

    void SceneViewerGui::drawCameraNode(CameraNodeImpl& obj)
    {
        drawNode(obj);
        std::array<int32_t, 2> xy{};
        std::array<int32_t, 2> wh{};
        xy[0] = obj.getViewportX();
        xy[1] = obj.getViewportY();
        wh[0] = static_cast<int32_t>(obj.getViewportWidth());
        wh[1] = static_cast<int32_t>(obj.getViewportHeight());
        if (obj.isViewportOffsetBound())
        {
            auto dataObject = const_cast<ramses::DataObject*>(findDataObject(obj.getViewportOffsetHandle()));
            ImGui::Text("Viewport offset:");
            if (dataObject != nullptr)
            {
                ImGui::SameLine();
                draw(dataObject->impl());
            }
        }
        else
        {
            if (ImGui::DragInt2("Viewport Offset(x, y)", xy.data()))
                obj.setViewport(xy[0], xy[1], wh[0], wh[1]);
        }

        if (obj.isViewportSizeBound())
        {
            auto dataObject = const_cast<ramses::DataObject*>(findDataObject(obj.getViewportSizeHandle()));
            ImGui::Text("Viewport size:");
            if (dataObject != nullptr)
            {
                ImGui::SameLine();
                draw(dataObject->impl());
            }
        }
        else
        {
            if (ImGui::DragInt2("Viewport Size(w, h)", wh.data()))
                obj.setViewport(xy[0], xy[1], wh[0], wh[1]);
        }

        if (obj.isFrustumPlanesBound())
        {
            ImGui::Text("Frustum (l,r,b,t):");
            auto frustum = const_cast<ramses::DataObject*>(findDataObject(obj.getFrustrumPlanesHandle()));
            if (frustum != nullptr)
            {
                ImGui::SameLine();
                draw(frustum->impl());
            }

            ImGui::Text("Frustum (n,f):");
            auto nearFar = const_cast<ramses::DataObject*>(findDataObject(obj.getFrustrumNearFarPlanesHandle()));
            if (nearFar != nullptr)
            {
                ImGui::SameLine();
                draw(nearFar->impl());
            }
        }
        else
        {
            std::array<float, 4> lrbp{};
            lrbp[0] = obj.getLeftPlane();
            lrbp[1] = obj.getRightPlane();
            lrbp[2] = obj.getBottomPlane();
            lrbp[3] = obj.getTopPlane();
            std::array<float, 2> nf{};
            nf[0] = obj.getNearPlane();
            nf[1] = obj.getFarPlane();
            if (ImGui::DragFloat4("Frustum (l,r,b,t)", lrbp.data(), 0.001f))
                obj.setFrustum(lrbp[0], lrbp[1], lrbp[2], lrbp[3], nf[0], nf[1]);
            if (ImGui::DragFloat2("Frustum (n,f)", nf.data(), 0.1f))
                obj.setFrustum(lrbp[0], lrbp[1], lrbp[2], lrbp[3], nf[0], nf[1]);
            if (obj.getType() == ramses::ERamsesObjectType::PerspectiveCamera)
            {
                std::array<float, 2> va{};
                va[0] = obj.getVerticalFieldOfView();
                va[1] = obj.getAspectRatio();
                if (ImGui::DragFloat2("VerticalFoV, AspectRatio", va.data(), 0.1f))
                    obj.setPerspectiveFrustum(va[0], va[1], nf[0], nf[1]);
            }
        }
        drawNodeChildrenParent(obj);
        drawRefs<ramses::RenderPass>("Used by RenderPass", obj, [&](const ramses::RenderPass* ref) {
            return ref->impl().getCamera() == &obj.getRamsesObject();
        });
    }

    void SceneViewerGui::drawResource(ResourceImpl& obj)
    {
        const auto hash = obj.getLowlevelResourceHash();
        auto resource = m_scene.getRamsesClient().impl().getResource(hash);
        const auto hashString = fmt::format("{}", hash);
        ImGui::Text("Hash: %s", hashString.c_str());
        if (ImGui::BeginPopupContextItem(hashString.c_str()))
        {
            if (ImGui::MenuItem("Copy hash"))
            {
                ImGui::LogToClipboard();
                ImGui::LogText("%s", hashString.c_str());
                ImGui::LogFinish();
            }
            ImGui::EndPopup();
        }

        if (resource)
        {
            double      size       = resource->getDecompressedDataSize();
            const auto  compressedSize = resource->getCompressedDataSize();
            double      compressed     = compressedSize;
            const char* unit       = "Bytes";
            if (size > 1024)
            {
                unit = "kB";
                size /= 1024.0;
                compressed /= 1024.0;
            }
            if (compressedSize != 0)
            {
                ImGui::Text("Resource size (%s): %.1f (compressed %.1f)", unit, size, compressed);
            }
            else
            {
                ImGui::Text("Resource size (%s): %.1f (uncompressed)", unit, size);
            }
        }
        else
        {
            ImGui::Text("Resource not loaded");
        }
    }

    void SceneViewerGui::drawEffect(EffectImpl& obj)
    {
        drawResource(obj);
        auto resource = m_scene.getRamsesClient().impl().getResource(obj.getLowlevelResourceHash());
        if (resource)
        {
            if (ImGui::Button("Shader Sources..."))
            {
                ImGui::OpenPopup("shader_src");
            }
            ImGui::SameLine();
            if (ImGui::Button("Save to file(s)"))
            {
                m_lastErrorMessage = saveShaderSources(obj);
            }
            if (ImGui::BeginPopup("shader_src"))
            {
                const auto effectRes = resource->convertTo<ramses::internal::EffectResource>();
                if (ImGui::BeginPopupContextWindow())
                {
                    if (ImGui::MenuItem("Copy vertex shader"))
                    {
                        ImGui::LogToClipboard();
                        ImGui::LogText("%s", effectRes->getVertexShader());
                        ImGui::LogFinish();
                    }
                    if (ImGui::MenuItem("Copy fragment shader"))
                    {
                        ImGui::LogToClipboard();
                        ImGui::LogText("%s", effectRes->getFragmentShader());
                        ImGui::LogFinish();
                    }
                    if (ImGui::MenuItem("Copy geometry shader"))
                    {
                        ImGui::LogToClipboard();
                        ImGui::LogText("%s", effectRes->getGeometryShader());
                        ImGui::LogFinish();
                    }
                    ImGui::EndPopup();
                }
                if (ImGui::CollapsingHeader("Vertex Shader", ImGuiTreeNodeFlags_DefaultOpen))
                    ImGui::TextUnformatted(effectRes->getVertexShader());
                if (ImGui::CollapsingHeader("Fragment Shader", ImGuiTreeNodeFlags_DefaultOpen))
                    ImGui::TextUnformatted(effectRes->getFragmentShader());
                if (ImGui::CollapsingHeader("Geometry Shader", ImGuiTreeNodeFlags_DefaultOpen))
                    ImGui::TextUnformatted(effectRes->getGeometryShader());
                ImGui::EndPopup();
            }
        }

        ImGui::Text("UNIFORMS");
        const auto& uniforms = obj.getUniformInputInformation();
        for (const auto& u : uniforms)
        {
            const char* const format = (u.semantics == EFixedSemantics::Invalid) ? "%s %s[%d]" : "%s %s[%d] (%s)";
            ImGui::BulletText(format, shortName(u.dataType), u.inputName.c_str(), u.elementCount, EFixedSemanticsNames[static_cast<int>(u.semantics)]);
        }
        ImGui::Text("ATTRIBUTES");
        const auto& attributes = obj.getAttributeInputInformation();
        for (const auto& a : attributes)
        {
            const char* const format = (a.semantics == EFixedSemantics::Invalid) ? "%s %s[%d]" : "%s %s[%d] (%s)";
            ImGui::BulletText(format, shortName(a.dataType), a.inputName.c_str(), a.elementCount, EFixedSemanticsNames[static_cast<int>(a.semantics)]);
        }

        drawRefs<ramses::Appearance>("Used by Appearance", obj, [&](const ramses::Appearance* ref) { return ref->impl().getEffectImpl()== &obj; });
        drawRefs<ramses::Geometry>("Used by Geometry", obj, [&](const ramses::Geometry* ref) { return &ref->impl().getEffect().impl() == &obj; });
    }

    void SceneViewerGui::drawRenderPass(RenderPassImpl& obj)
    {
        int32_t renderOrder = obj.getRenderOrder();
        if (ImGui::DragInt("RenderOrder", &renderOrder))
            obj.setRenderOrder(renderOrder);
        auto clearFlags = obj.getClearFlags().value();
        if (ImGui::TreeNode("Clear"))
        {
            if (ImGui::CheckboxFlags("Color", &clearFlags, static_cast<uint32_t>(ramses::EClearFlag::Color)))
                obj.setClearFlags(ClearFlags(clearFlags));
            if (ImGui::CheckboxFlags("Depth", &clearFlags, static_cast<uint32_t>(ramses::EClearFlag::Depth)))
                obj.setClearFlags(ClearFlags(clearFlags));
            if (ImGui::CheckboxFlags("Stencil", &clearFlags, static_cast<uint32_t>(ramses::EClearFlag::Stencil)))
                obj.setClearFlags(EClearFlag(clearFlags));
            if ((clearFlags & static_cast<uint32_t>(ramses::EClearFlag::Color)) != 0u)
            {
                std::array<float, 4> rgba{};
                const auto& color = obj.getClearColor();
                rgba[0] = color.r;
                rgba[1] = color.g;
                rgba[2] = color.b;
                rgba[3] = color.a;
                if (ImGui::ColorEdit4("ClearColor", rgba.data()))
                    obj.setClearColor(glm::vec4(rgba[0], rgba[1], rgba[2], rgba[3]));
            }
            ImGui::TreePop();
        }
        bool enabled = obj.isEnabled();
        if (ImGui::Checkbox("Enabled", &enabled))
            obj.setEnabled(enabled);
        bool renderOnce = obj.isRenderOnce();
        if (ImGui::Checkbox("RenderOnce", &renderOnce))
            obj.setRenderOnce(renderOnce);
        if (renderOnce)
        {
            ImGui::SameLine();
            if (ImGui::Button("Refresh"))
                obj.retriggerRenderOnce();
        }

        if (obj.getCamera())
            draw(obj.getCamera()->impl());

        auto rt = const_cast<ramses::RenderTarget*>(obj.getRenderTarget());
        if (rt != nullptr)
        {
            draw(rt->impl());
        }
        else
        {
            ImGui::Text("No render target");
        }

        ImGui::Text("RenderGroups:");
        auto& renderGroups = m_renderInfo.renderGroupMap[&obj];
        if (renderGroups.empty())
        {
            renderGroups = obj.getAllRenderGroups();
            std::sort(renderGroups.begin(), renderGroups.end(), [&](const auto* a, const auto* b) {
                int32_t orderA = 0;
                int32_t orderB = 0;
                obj.getRenderGroupOrder(*a, orderA);
                obj.getRenderGroupOrder(*b, orderB);
                return (orderA < orderB);
                });
        }

        for (const auto* it : renderGroups)
        {
            int32_t order = 0;
            obj.getRenderGroupOrder(*it, order);
            ImGui::SetNextItemWidth(60);
            ImGui::PushID(it);
            if (ImGui::DragInt("##order", &order))
            {
                obj.removeIfContained(*it);
                obj.addRenderGroup(*it, order);
            }
            ImGui::PopID();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Render group order");
            ImGui::SameLine();
            draw(*const_cast<RenderGroupImpl*>(it));
        }
    }

    void SceneViewerGui::drawRenderGroup(RenderGroupImpl& obj)
    {
        // sort meshes / render groups by drawing order
        auto& renderables = m_renderInfo.renderableMap[&obj];
        if (renderables.empty())
        {
            const auto& meshes = obj.getAllMeshes();
            const auto& renderGroups = obj.getAllRenderGroups();
            renderables.reserve(meshes.size() + renderGroups.size());
            for (const auto* it : renderGroups)
                renderables.push_back(it);
            for (const auto* it : meshes)
                renderables.push_back(it);
            std::sort(renderables.begin(), renderables.end(), [&](const auto* a, const auto* b) {
                const int32_t orderA = getRenderOrder(obj, *a);
                const int32_t orderB = getRenderOrder(obj, *b);
                return (orderA < orderB);
                });
        }

        for (auto it : renderables)
        {
            int32_t order = getRenderOrder(obj, *it);
            ImGui::SetNextItemWidth(60);
            ImGui::PushID(it);
            if (ImGui::DragInt("##order", &order))
            {
                if (it->getType() == ramses::ERamsesObjectType::MeshNode)
                {
                    const auto* meshNode = static_cast<const MeshNodeImpl*>(it);
                    obj.removeIfContained(*meshNode);
                    obj.addMeshNode(*meshNode, order);
                }
                else
                {
                    const auto* renderGroup = static_cast<const RenderGroupImpl*>(it);
                    obj.removeIfContained(*renderGroup);
                    obj.addRenderGroup(*renderGroup, order);
                }
            }
            ImGui::PopID();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Render order");
            ImGui::SameLine();
            draw(const_cast<SceneObjectImpl&>(*it));
        }

        drawRefs<ramses::RenderPass>("Used by RenderPass", obj, [&](const ramses::RenderPass* ref) {
            const auto& groups = ref->impl().getAllRenderGroups();
            return std::find(groups.begin(), groups.end(), &obj) != groups.end();
        });
    }

    void SceneViewerGui::drawRenderTarget(RenderTargetImpl& obj)
    {
        const auto rtHandle = obj.getRenderTargetHandle();
        const uint32_t numBuffers = m_scene.impl().getIScene().getRenderTargetRenderBufferCount(rtHandle);
        for (uint32_t i = 0; i < numBuffers; ++i)
        {
            const auto rbHandle = m_scene.impl().getIScene().getRenderTargetRenderBuffer(rtHandle, i);
            auto* rb = const_cast<ramses::RenderBuffer*>(findRenderBuffer(rbHandle));
            if (rb != nullptr)
            {
                draw(rb->impl());
            }
            else
            {
                ImGui::Text("RenderBuffer not found");
            }
        }
        drawRefs<ramses::RenderPass>("Used by RenderPass", obj, [&](const ramses::RenderPass* ref) {
            return (ref->impl().getRenderTarget() == &obj.getRamsesObject());
        });
    }

    void SceneViewerGui::drawRenderBuffer(RenderBufferImpl& obj)
    {
        const auto& rb = m_scene.impl().getIScene().getRenderBuffer(obj.getRenderBufferHandle());
        ImGui::Text("Width:%u Height:%u", rb.width, rb.height);
        ImGui::Text("BufferFormat: %s", EnumToString(rb.format));
        ImGui::Text("AccessMode: %s", EnumToString(rb.accessMode));
        ImGui::Text("SampleCount: %u", rb.sampleCount);
        drawRefs<ramses::RenderTarget>("Used by RenderTarget", obj, [&](const ramses::RenderTarget* ref) {
            const auto rtHandle   = ref->impl().getRenderTargetHandle();
            const uint32_t numBuffers = m_scene.impl().getIScene().getRenderTargetRenderBufferCount(rtHandle);
            for (uint32_t i = 0; i < numBuffers; ++i)
            {
                const auto rbHandle = m_scene.impl().getIScene().getRenderTargetRenderBuffer(rtHandle, i);
                if (rbHandle == obj.getRenderBufferHandle())
                    return true;
            }
            return false;
        });
        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const ramses::internal::TextureSampler& sampler = obj.getIScene().getTextureSampler(ref->impl().getTextureSamplerHandle());
            return (sampler.isRenderBuffer()) && (sampler.contentHandle == obj.getRenderBufferHandle().asMemoryHandle());
        });
        drawRefs<ramses::TextureSamplerMS>("Used by TextureSamplerMS", obj, [&](const ramses::TextureSamplerMS* ref) {
            const ramses::internal::TextureSampler& sampler = obj.getIScene().getTextureSampler(ref->impl().getTextureSamplerHandle());
            return (sampler.isRenderBuffer()) && (sampler.contentHandle == obj.getRenderBufferHandle().asMemoryHandle());
        });
        drawRefs<ramses::BlitPass>("Used by BlitPass", obj, [&](const ramses::BlitPass* ref) {
            const auto& bp = m_scene.impl().getIScene().getBlitPass(ref->impl().getBlitPassHandle());
            return (bp.sourceRenderBuffer == obj.getRenderBufferHandle()) || (bp.destinationRenderBuffer == obj.getRenderBufferHandle());
        });
    }

    void SceneViewerGui::drawBlitPass(BlitPassImpl& obj)
    {
        const auto& bp = m_scene.impl().getIScene().getBlitPass(obj.getBlitPassHandle());
        std::array src = {static_cast<int>(bp.sourceRegion.x), static_cast<int>(bp.sourceRegion.y), bp.sourceRegion.width, bp.sourceRegion.height};
        std::array dst = {static_cast<int>(bp.destinationRegion.x), static_cast<int>(bp.destinationRegion.y)};
        bool isEnabled = bp.isEnabled;
        int renderOrder = obj.getRenderOrder();
        if (ImGui::Checkbox("Enabled", &isEnabled))
            obj.setEnabled(isEnabled);
        if (ImGui::DragInt("RenderOrder", &renderOrder))
            obj.setRenderOrder(renderOrder);
        if (ImGui::DragInt4("SourceRegion (x,y,w,h)", src.data()))
            obj.setBlittingRegion(src[0], src[1], dst[0], dst[1], src[2], src[3]);
        if (ImGui::DragInt2("Destination (x,y)", dst.data()))
            obj.setBlittingRegion(src[0], src[1], dst[0], dst[1], src[2], src[3]);
        ImGui::Text("src:");
        ImGui::SameLine();
        draw(const_cast<ramses::RenderBuffer&>(obj.getSourceRenderBuffer()).impl());
        ImGui::Text("dst:");
        ImGui::SameLine();
        draw(const_cast<ramses::RenderBuffer&>(obj.getDestinationRenderBuffer()).impl());
    }

    void SceneViewerGui::drawAppearance(ramses::Appearance& appearance)
    {
        auto& obj = appearance.impl();
        const ramses::Effect& effect = obj.getEffect();
        ramses::internal::ClientScene& iscene   = m_scene.impl().getIScene();

        imgui::RenderState(iscene, obj.getRenderStateHandle());

        if (ImGui::TreeNode("Uniform input"))
        {
            for (uint32_t i = 0; i < effect.getUniformInputCount(); ++i)
            {
                const std::optional<ramses::UniformInput> uniform = effect.getUniformInput(i);
                assert(uniform.has_value());
                auto* boundObj = const_cast<ramses::DataObject*>(obj.getBoundDataObject(uniform->impl()));
                if (uniform->getSemantics() != ramses::EEffectUniformSemantic::Invalid)
                {
                    ImGui::BulletText("%s %s[%lu] (%s)", shortName(uniform->impl().getInternalDataType()), uniform->getName(), uniform->getElementCount(), EFixedSemanticsNames[static_cast<int>(uniform->impl().getSemantics())]);
                }
                else
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                    if (ImGui::TreeNode(uniform->getName(), "%s %s[%lu]:", shortName(uniform->impl().getInternalDataType()), uniform->getName(), uniform->getElementCount()))
                    {
                        if (boundObj != nullptr)
                        {
                            draw(boundObj->impl());
                        }
                        else
                        {
                            drawUniformValue(appearance, *uniform);
                        }
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::TreePop();
        }
        draw(const_cast<ramses::Effect&>(effect).impl());
        drawRefs<ramses::MeshNode>("Used by MeshNode", obj, [&](const ramses::MeshNode* ref) {
            return ref->impl().getAppearanceImpl() == &obj;
        });
    }

    void SceneViewerGui::drawUniformValue(ramses::Appearance& appearance, const ramses::UniformInput& uniform)
    {
        std::vector<ramses::vec4i> vec4i;
        std::vector<ramses::vec3i> vec3i;
        std::vector<ramses::vec2i> vec2i;
        std::vector<int32_t>       vec1i;
        std::vector<ramses::vec4f> vec4;
        std::vector<ramses::vec3f> vec3;
        std::vector<ramses::vec2f> vec2;
        std::vector<float>         vec1;
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        std::unique_ptr<bool[]>    uniformBool;
        const ramses::TextureSampler*          textureSampler = nullptr;
        const ramses::TextureSamplerMS*        textureSamplerMS = nullptr;
        const ramses::TextureSamplerExternal*  textureSamplerExternal = nullptr;
        bool status = false;

        switch (uniform.getDataType())
        {
        case ramses::EDataType::Float:
            vec1.resize(uniform.getElementCount());
            status = appearance.getInputValue(uniform, uniform.getElementCount(), vec1.data());
            if (status)
            {
                for (auto it = vec1.begin(); it != vec1.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec1.begin());
                    if (ImGui::DragFloat(label.c_str(), &*it, 0.1f))
                        appearance.setInputValue(uniform, uniform.getElementCount(), vec1.data());
                }
            }
            break;
        case ramses::EDataType::Vector2F:
            vec2.resize(uniform.getElementCount());
            status = appearance.getInputValue(uniform, uniform.getElementCount(), vec2.data());
            if (status)
            {
                for (auto it = vec2.begin(); it != vec2.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec2.begin());
                    if (ImGui::DragFloat2(label.c_str(), glm::value_ptr(*it), 0.1f))
                        appearance.setInputValue(uniform, uniform.getElementCount(), vec2.data());
                }
            }
            break;
        case ramses::EDataType::Vector3F:
            vec3.resize(uniform.getElementCount());
            status = appearance.getInputValue(uniform, uniform.getElementCount(), vec3.data());
            if (status)
            {
                for (auto it = vec3.begin(); it != vec3.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec3.begin());
                    if (ImGui::DragFloat3(label.c_str(), glm::value_ptr(*it), 0.1f))
                        appearance.setInputValue(uniform, uniform.getElementCount(), vec3.data());
                }
            }
            break;
        case ramses::EDataType::Vector4F:
            vec4.resize(uniform.getElementCount());
            status = appearance.getInputValue(uniform, uniform.getElementCount(), vec4.data());
            if (status)
            {
                for (auto it = vec4.begin(); it != vec4.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec4.begin());
                    if (ImGui::DragFloat4(label.c_str(), glm::value_ptr(*it), 0.1f))
                        appearance.setInputValue(uniform, uniform.getElementCount(), vec4.data());
                }
            }
            break;
        case ramses::EDataType::Bool:
            uniformBool.reset(new bool[uniform.getElementCount()]);
            status = appearance.getInputValue(uniform, uniform.getElementCount(), uniformBool.get());
            if (status)
            {
                for (size_t i = 0u; i < uniform.getElementCount(); ++i)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), i);
                    if (ImGui::Checkbox(label.c_str(), &uniformBool[i]))
                        appearance.setInputValue(uniform, uniform.getElementCount(), uniformBool.get());
                }
            }
            break;
        case ramses::EDataType::Int32:
            vec1i.resize(uniform.getElementCount());
            status = appearance.getInputValue(uniform, uniform.getElementCount(), vec1i.data());
            if (status)
            {
                for (auto it = vec1i.begin(); it != vec1i.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec1i.begin());
                    if (ImGui::DragInt(label.c_str(), &*it, 0.1f))
                        appearance.setInputValue(uniform, uniform.getElementCount(), vec1i.data());
                }
            }
            break;
        case ramses::EDataType::Vector2I:
            vec2i.resize(uniform.getElementCount());
            status = appearance.getInputValue(uniform, uniform.getElementCount(), vec2i.data());
            if (status)
            {
                for (auto it = vec2i.begin(); it != vec2i.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec2i.begin());
                    if (ImGui::DragInt2(label.c_str(), glm::value_ptr(*it), 0.1f))
                        appearance.setInputValue(uniform, uniform.getElementCount(), vec2i.data());
                }
            }
            break;
        case ramses::EDataType::Vector3I:
            vec3i.resize(uniform.getElementCount());
            status = appearance.getInputValue(uniform, uniform.getElementCount(), vec3i.data());
            if (status)
            {
                for (auto it = vec3i.begin(); it != vec3i.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec3i.begin());
                    if (ImGui::DragInt3(label.c_str(), glm::value_ptr(*it), 0.1f))
                        appearance.setInputValue(uniform, uniform.getElementCount(), vec3i.data());
                }
            }
            break;
        case ramses::EDataType::Vector4I:
            vec4i.resize(uniform.getElementCount());
            status = appearance.getInputValue(uniform, uniform.getElementCount(), vec4i.data());
            if (status)
            {
                for (auto it = vec4i.begin(); it != vec4i.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec4i.begin());
                    if (ImGui::DragInt4(label.c_str(), glm::value_ptr(*it), 0.1f))
                        appearance.setInputValue(uniform, uniform.getElementCount(), vec4i.data());
                }
            }
            break;
        case ramses::EDataType::TextureSampler2D:
        case ramses::EDataType::TextureSampler3D:
        case ramses::EDataType::TextureSamplerCube:
            status = appearance.getInputTexture(uniform, textureSampler);
            if (status)
            {
                draw(const_cast<ramses::TextureSampler*>(textureSampler)->impl());
            }
            break;
        case ramses::EDataType::TextureSampler2DMS:
            status = appearance.getInputTextureMS(uniform, textureSamplerMS);
            if (status)
            {
                draw(const_cast<ramses::TextureSamplerMS*>(textureSamplerMS)->impl());
            }
            break;
        case ramses::EDataType::TextureSamplerExternal:
            status = appearance.getInputTextureExternal(uniform, textureSamplerExternal);
            if (status)
            {
                draw(const_cast<ramses::TextureSamplerExternal*>(textureSamplerExternal)->impl());
            }
            break;
        case ramses::EDataType::UInt16:
        case ramses::EDataType::UInt32:
        case ramses::EDataType::Matrix22F:
        case ramses::EDataType::Matrix33F:
        case ramses::EDataType::Matrix44F:
        case ramses::EDataType::ByteBlob:
            ImGui::Text("tbd. %s", EnumToString(uniform.impl().getInternalDataType()));
            break;
        }

        if (!status)
            ImGui::Text("Error occurred: %s", m_scene.getRamsesClient().getRamsesFramework().getLastError().value_or(Issue{}).message.c_str());
    }

    void SceneViewerGui::drawGeometry(GeometryImpl& obj)
    {
        const ramses::Effect& effect = obj.getEffect();
        if (ImGui::TreeNode("Attribute input"))
        {
            auto& iScene = m_scene.impl().getIScene();
            const ramses::internal::DataLayout& layout = iScene.getDataLayout(obj.getAttributeDataLayout());
            const uint32_t dataLayoutFieldCount = layout.getFieldCount();
            for (uint32_t i = 0U; i < dataLayoutFieldCount; ++i)
            {
                if (0 == i)
                {
                    ImGui::Text("Indices:");
                }
                else
                {
                    ImGui::Text("%s:", effect.getAttributeInput(i - 1)->getName());
                }
                const ramses::internal::DataFieldHandle fieldIndex(i);
                const ramses::internal::ResourceField&  dataResource = iScene.getDataResource(obj.getAttributeDataInstance(), fieldIndex);

                if (dataResource.dataBuffer.isValid())
                {
                    auto buf = const_cast<ramses::ArrayBuffer*>(findArrayBuffer(dataResource.dataBuffer));
                    assert(buf != nullptr);
                    draw(buf->impl());
                }
                else if (dataResource.hash.isValid())
                {
                    ramses::Resource* resource = m_scene.impl().scanForResourceWithHash(dataResource.hash);
                    if (resource != nullptr)
                    {
                        draw(resource->impl());
                    }
                    else
                    {
                        ImGui::Text("Resource missing");
                    }
                }
            }
            ImGui::TreePop();
        }

        draw(const_cast<ramses::Effect&>(effect).impl());
        drawRefs<ramses::MeshNode>("Used by MeshNode", obj, [&](const ramses::MeshNode* ref) {
            return ref->impl().getGeometryImpl() == &obj;
        });
    }

    void SceneViewerGui::drawTexture2D(Texture2DImpl& obj)
    {
        drawResource(obj);
        ImGui::Text("Width:%u Height:%u Format:%s", obj.getWidth(), obj.getHeight(), ramses::toString(obj.getTextureFormat()));
        const auto& swizzle = obj.getTextureSwizzle();
        ImGui::Text("Swizzle: r:%s g:%s b:%s a:%s",
            ramses::EnumToString(swizzle.channelRed),
            ramses::EnumToString(swizzle.channelGreen),
            ramses::EnumToString(swizzle.channelBlue),
            ramses::EnumToString(swizzle.channelAlpha));
        const auto* slot = findDataSlot(obj.getLowlevelResourceHash());
        if (slot)
            ImGui::Text("DataSlot: %u %s", slot->id.getValue(), EnumToString(slot->type));
        auto resource = m_scene.getRamsesClient().impl().getResource(obj.getLowlevelResourceHash());
        if (resource)
        {
            const auto* textureResource = resource->convertTo<ramses::internal::TextureResource>();
            if (textureResource != nullptr)
            {
                ImGui::TextUnformatted(fmt::format("GenerateMipChain: {}", textureResource->getGenerateMipChainFlag()).c_str());

                if (ImGui::Button("Save png"))
                {
                    m_lastErrorMessage = saveTexture2D(obj);
                }
                imgui::PreviewImage(m_imageCache.get(textureResource), ImVec2(128, 128));
            }
        }

        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl().getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::ClientTexture) && (sampler.textureResource == obj.getLowlevelResourceHash());
        });
    }

    void SceneViewerGui::drawTexture3D(Texture3DImpl& obj)
    {
        drawResource(obj);
        ImGui::Text("Width:%u Height:%u Format:%s", obj.getWidth(), obj.getHeight(), ramses::toString(obj.getTextureFormat()));
        ImGui::Text("Depth:%u", obj.getDepth());

        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl().getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::ClientTexture) && (sampler.textureResource == obj.getLowlevelResourceHash());
        });
    }

    void SceneViewerGui::drawTexture2DBuffer(Texture2DBufferImpl& obj)
    {
        const auto& tb = m_scene.impl().getIScene().getTextureBuffer(obj.getTextureBufferHandle());
        ImGui::Text("Format:%s", EnumToString(tb.textureFormat));
        for (auto it = tb.mipMaps.begin(); it != tb.mipMaps.end(); ++it)
        {
            ImGui::Text("MipMap:");
            ImGui::BulletText("width:%u height:%u", it->width, it->height);
            ImGui::BulletText("area: x:%d y:%d w:%d h:%d", it->usedRegion.x, it->usedRegion.y, it->usedRegion.width, it->usedRegion.height);
            ImGui::BulletText("size (kB): %" PRIu32, static_cast<uint32_t>(it->data.size() / 1024));
            if (ImGui::Button("Save png"))
            {
                const auto filename = fmt::format("{:04}_Texture2DBuffer.png", obj.getObjectRegistryHandle().asMemoryHandle());
                m_lastErrorMessage = imgui::SaveToPng(reinterpret_cast<const uint8_t*>(it->data.data()), it->data.size(), tb.textureFormat, it->width, it->height, filename);
            }
            imgui::PreviewImage(m_imageCache.get(*it, tb.textureFormat), ImVec2(128, 128));
        }
        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl().getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::TextureBuffer) && (sampler.contentHandle == obj.getTextureBufferHandle());
        });
    }

    void SceneViewerGui::drawTextureCube(TextureCubeImpl& obj)
    {
        drawResource(obj);
        ImGui::Text("Size:%u Format:%s", obj.getSize(), ramses::toString(obj.getTextureFormat()));
        const auto& swizzle = obj.getTextureSwizzle();
        ImGui::Text("Swizzle: r:%s g:%s b:%s a:%s",
            ramses::EnumToString(swizzle.channelRed),
            ramses::EnumToString(swizzle.channelGreen),
            ramses::EnumToString(swizzle.channelBlue),
            ramses::EnumToString(swizzle.channelAlpha));

        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl().getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::ClientTexture) && (sampler.textureResource == obj.getLowlevelResourceHash());
        });
    }

    void SceneViewerGui::drawTextureSampler(TextureSamplerImpl& obj)
    {
        const auto* slot = findDataSlot(obj.getTextureSamplerHandle());
        if (slot)
            ImGui::Text("DataSlot: %u %s", slot->id.getValue(), EnumToString(slot->type));
        ImGui::Text("Wrap:");
        ImGui::BulletText("u:%s", ramses::toString(obj.getWrapUMode()));
        ImGui::BulletText("v:%s", ramses::toString(obj.getWrapVMode()));
        ImGui::BulletText("r:%s", ramses::toString(obj.getWrapRMode()));
        ImGui::Text("Sampling:");
        ImGui::BulletText("min:%s", ramses::toString(obj.getMinSamplingMethod()));
        ImGui::BulletText("mag:%s", ramses::toString(obj.getMagSamplingMethod()));
        ImGui::Text("AnisotropyLevel: %u", obj.getAnisotropyLevel());

        ramses::Resource* resource = nullptr;
        const ramses::internal::TextureSampler& sampler = obj.getIScene().getTextureSampler(obj.getTextureSamplerHandle());
        ramses::RenderBuffer* rb = nullptr;
        ramses::Texture2DBuffer* textureBuffer = nullptr;
        switch (sampler.contentType)
        {
        case TextureSampler::ContentType::ClientTexture:
            resource = obj.getSceneImpl().scanForResourceWithHash(sampler.textureResource);
            if (resource != nullptr)
            {
                draw(resource->impl());
            }
            else
            {
                ImGui::Text("Resource missing");
            }
            break;
        case TextureSampler::ContentType::RenderBuffer:
        case TextureSampler::ContentType::RenderBufferMS:
            rb = const_cast<ramses::RenderBuffer*>(findRenderBuffer(RenderBufferHandle(sampler.contentHandle)));
            if (rb != nullptr)
            {
                draw(rb->impl());
            }
            else
            {
                ImGui::Text("RenderBuffer missing");
            }
            break;
        case TextureSampler::ContentType::TextureBuffer:
            textureBuffer = const_cast<ramses::Texture2DBuffer*>(findTextureBuffer(TextureBufferHandle(sampler.contentHandle)));
            if (textureBuffer != nullptr)
            {
                draw(textureBuffer->impl());
            }
            else
            {
                ImGui::Text("TextureBuffer missing");
            }
            break;
        case TextureSampler::ContentType::ExternalTexture:
            // no details
            break;
        case TextureSampler::ContentType::OffscreenBuffer:
        case TextureSampler::ContentType::StreamBuffer:
        case TextureSampler::ContentType::None:
            ImGui::Text("Type: %s (tbd.)", RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj.getTextureType()));
            break;
        }

        drawRefs<ramses::Appearance>("Used by Appearance", obj, [&](const ramses::Appearance* ref) {
            const ramses::Effect& effect = ref->getEffect();
            auto* ref_nonconst = const_cast<ramses::Appearance*>(ref);
            for (uint32_t i = 0; i < effect.getUniformInputCount(); ++i)
            {
                std::optional<ramses::UniformInput> uniform = effect.getUniformInput(i);
                assert(uniform.has_value());
                const ramses::TextureSampler* textureSampler = nullptr;
                const ramses::TextureSamplerMS* textureSamplerMS = nullptr;
                const ramses::TextureSamplerExternal* textureSamplerExternal = nullptr;
                switch (uniform->getDataType())
                {
                case ramses::EDataType::TextureSampler2D:
                case ramses::EDataType::TextureSampler3D:
                case ramses::EDataType::TextureSamplerCube:
                    if (ref_nonconst->impl().getInputTexture(uniform->impl(), textureSampler))
                    {
                        if (textureSampler == &obj.getRamsesObject())
                            return true;
                    }
                    break;
                case ramses::EDataType::TextureSampler2DMS:
                    if (ref_nonconst->impl().getInputTextureMS(uniform->impl(), textureSamplerMS))
                    {
                        if (textureSamplerMS == &obj.getRamsesObject())
                            return true;
                    }
                    break;
                case ramses::EDataType::TextureSamplerExternal:
                    if (ref_nonconst->impl().getInputTextureExternal(uniform->impl(), textureSamplerExternal))
                    {
                        if (textureSamplerExternal == &obj.getRamsesObject())
                            return true;
                    }
                    break;
                default:
                    break;
                }
            }
            return false;
        });
    }

    void SceneViewerGui::drawArrayResource(ArrayResourceImpl& obj)
    {
        drawResource(obj);
        ImGui::Text("%s[%u]", EnumToString(obj.getElementType()), obj.getElementCount());
        drawRefs<ramses::Geometry>("Used by Geometry", obj, [&](const ramses::Geometry* ref) {
            auto&          iScene     = m_scene.impl().getIScene();
            const auto&    layout     = iScene.getDataLayout(ref->impl().getAttributeDataLayout());
            const uint32_t fieldCount = layout.getFieldCount();
            for (uint32_t i = 0U; i < fieldCount; ++i)
            {
                const ramses::internal::DataFieldHandle fieldIndex(i);
                const ramses::internal::ResourceField&  dataResource = iScene.getDataResource(ref->impl().getAttributeDataInstance(), fieldIndex);
                if (dataResource.hash == obj.getLowlevelResourceHash())
                    return true;
            }
            return false;
        });
    }

    void SceneViewerGui::drawArrayBuffer(ArrayBufferImpl& obj)
    {
        ImGui::Text("%s[%u]", EnumToString(obj.getDataType()), obj.getElementCount());
        drawRefs<ramses::Geometry>("Used by Geometry", obj, [&](const ramses::Geometry* ref) {
            auto&          iScene     = m_scene.impl().getIScene();
            const auto&    layout     = iScene.getDataLayout(ref->impl().getAttributeDataLayout());
            const uint32_t fieldCount = layout.getFieldCount();
            for (uint32_t i = 0U; i < fieldCount; ++i)
            {
                const ramses::internal::DataFieldHandle fieldIndex(i);
                const ramses::internal::ResourceField&  dataResource = iScene.getDataResource(ref->impl().getAttributeDataInstance(), fieldIndex);
                if (dataResource.dataBuffer == obj.getDataBufferHandle())
                    return true;
            }
            return false;
        });
    }

    void SceneViewerGui::drawDataObject(ramses::DataObject& obj)
    {
        float valueF = NAN;
        int32_t valueI = 0;
        ramses::vec2f value2F;
        ramses::vec3f value3F;
        ramses::vec4f value4F;
        ramses::vec2i value2I;
        ramses::vec3i value3I;
        ramses::vec4i value4I;
        const auto* slot = findDataSlot(obj.impl().getDataReference());
        if (slot)
            ImGui::Text("DataSlot: %u %s", slot->id.getValue(), EnumToString(slot->type));
        switch (obj.getDataType())
        {
        case ramses::EDataType::Float:
            obj.getValue(valueF);
            if (ImGui::DragFloat("Value", &valueF, 0.1f))
                obj.setValue(valueF);
            break;
        case ramses::EDataType::Vector2F:
            obj.getValue(value2F);
            if (ImGui::DragFloat2("Value", glm::value_ptr(value2F), 0.1f))
                obj.setValue(value2F);
            break;
        case ramses::EDataType::Vector3F:
            obj.getValue(value3F);
            if (ImGui::DragFloat3("Value", glm::value_ptr(value3F), 0.1f))
                obj.setValue(value3F);
            break;
        case ramses::EDataType::Vector4F:
            obj.getValue(value4F);
            if (ImGui::DragFloat4("Value", glm::value_ptr(value4F), 0.1f))
                obj.setValue(value4F);
            break;
        case ramses::EDataType::Int32:
            obj.getValue(valueI);
            if (ImGui::DragInt("Value", &valueI))
                obj.setValue(valueI);
            break;
        case ramses::EDataType::Vector2I:
            obj.getValue(value2I);
            if (ImGui::DragInt2("Value", glm::value_ptr(value2I)))
                obj.setValue(value2I);
            break;
        case ramses::EDataType::Vector3I:
            obj.getValue(value3I);
            if (ImGui::DragInt3("Value", glm::value_ptr(value3I)))
                obj.setValue(value3I);
            break;
        case ramses::EDataType::Vector4I:
            obj.getValue(value4I);
            if (ImGui::DragInt4("Value", glm::value_ptr(value4I)))
                obj.setValue(value4I);
            break;
        default:
            ImGui::Text("tbd. %s", EnumToString(obj.getDataType()));
        }

        drawRefs<ramses::Appearance>("Used by Appearance", obj.impl(), [&](const ramses::Appearance* ref) {
            const ramses::Effect& effect = ref->getEffect();
            for (uint32_t i = 0; i < effect.getUniformInputCount(); ++i)
            {
                const ramses::DataObject* boundObj = ref->impl().getBoundDataObject(effect.getUniformInput(i)->impl());
                if (boundObj == &obj)
                    return true;
            }
            return false;
        });

        drawRefs<ramses::Camera>("Used by Camera", obj.impl(), [&](const ramses::Camera* ref) {
            auto fp = findDataObject(ref->impl().getFrustrumPlanesHandle());
            auto nf = findDataObject(ref->impl().getFrustrumNearFarPlanesHandle());
            auto pos = findDataObject(ref->impl().getViewportOffsetHandle());
            auto size = findDataObject(ref->impl().getViewportSizeHandle());
            const auto objPtr = &obj;
            return (objPtr == fp || objPtr == nf || objPtr == pos || objPtr == size);
        });
    }

    void SceneViewerGui::DrawSceneReference(SceneReferenceImpl& obj)
    {
        ImGui::TextUnformatted(fmt::format("ReferencedScene: {}", obj.getReferencedSceneId().getValue()).c_str());
        ImGui::TextUnformatted(fmt::format("RequestedState: {}", EnumToString(obj.getRequestedState())).c_str());
        ImGui::TextUnformatted(fmt::format("ReportedState: {}", EnumToString(obj.getReportedState())).c_str());
    }

    void SceneViewerGui::drawDataSlot(const ramses::internal::DataSlot& obj)
    {
        if (ImGui::TreeNode(&obj, "Slot: %3u %s", obj.id.getValue(), EnumToString(obj.type)))
        {
            if (obj.attachedTexture.isValid())
            {
                auto tex = const_cast<ramses::Texture2D*>(findTexture2D(obj.attachedTexture));
                if (tex != nullptr)
                {
                    draw(tex->impl());
                }
                else
                {
                    ImGui::Text("Texture not found: %" PRIx64 ":%" PRIx64, obj.attachedTexture.highPart, obj.attachedTexture.lowPart);
                }
            }
            else if (obj.attachedNode.isValid())
            {
                auto node = const_cast<ramses::Node*>(findNode(obj.attachedNode));
                if (node != nullptr)
                {
                    draw(node->impl());
                }
                else
                {
                    ImGui::Text("Node not found: %u", obj.attachedNode.asMemoryHandle());
                }
            }
            else if (obj.attachedDataReference.isValid())
            {
                auto ref = const_cast<ramses::DataObject*>(findDataObject(obj.attachedDataReference));
                if (ref != nullptr)
                {
                    draw(ref->impl());
                }
                else
                {
                    ImGui::Text("DataReference not found: %u", obj.attachedDataReference.asMemoryHandle());
                }
            }
            else if (obj.attachedTextureSampler.isValid())
            {
                auto ref = const_cast<ramses::TextureSampler*>(findTextureSampler(obj.attachedTextureSampler));
                if (ref != nullptr)
                {
                    draw(ref->impl());
                }
                else
                {
                    ImGui::Text("TextureSampler not found: %u", obj.attachedTextureSampler.asMemoryHandle());
                }
            }
            else
            {
                ImGui::Text("<not connected>");
            }
            ImGui::TreePop();
        }
    }

    void SceneViewerGui::drawSceneObjectsFilter()
    {
        ImGui::Text("Name filter:");
        bool filterModified = m_filter.Draw();
        if (ImGui::TreeNode("More filters..."))
        {
            ImGui::Text("Show nodes with visibility:");
            filterModified = ImGui::Checkbox("Off", &m_nodeFilter.showOff) || filterModified;
            filterModified = ImGui::Checkbox("Invisible", &m_nodeFilter.showInvisible) || filterModified;
            filterModified = ImGui::Checkbox("Visible", &m_nodeFilter.showVisible) || filterModified;
            ImGui::TreePop();
        }

        if (filterModified || m_sceneObjects.empty())
        {
            const auto& reg = m_scene.impl().getObjectRegistry();
            for (uint32_t i = 0u; i < static_cast<uint32_t>(ramses::RamsesObjectTypeCount); ++i)
            {
                const auto type = static_cast<ramses::ERamsesObjectType>(i);

                if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ramses::ERamsesObjectType::SceneObject)
                    && RamsesObjectTypeUtils::IsConcreteType(type))
                {
                    auto& objects = m_sceneObjects[type];
                    const auto numberOfObjects = reg.getNumberOfObjects(type);
                    objects.reserve(numberOfObjects);
                    objects.clear();
                    if (numberOfObjects > 0u)
                    {
                        SceneObjectRegistryIterator iter(reg, ramses::ERamsesObjectType(i));
                        while (auto* obj = iter.getNextNonConst())
                        {
                            if (passFilter(obj->impl()))
                                objects.push_back(obj);
                        }
                    }
                }
            }
        }
    }

    bool SceneViewerGui::passFilter(const SceneObjectImpl& obj) const
    {
        bool pass = true;
        if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(obj.getType(), ramses::ERamsesObjectType::Resource))
        {
            auto resource = static_cast<const ResourceImpl&>(obj);
            const auto hashString = fmt::format("{}", resource.getLowlevelResourceHash());
            return m_filter.PassFilter(hashString.c_str()) || m_filter.PassFilter(obj.getName().c_str());
        }
        if (RamsesObjectTypeUtils::IsTypeMatchingBaseType(obj.getType(), ramses::ERamsesObjectType::Node))
        {
            auto node = static_cast<const NodeImpl&>(obj);
            switch (node.getVisibility())
            {
            case ramses::EVisibilityMode::Off:
                pass = m_nodeFilter.showOff;
                break;
            case ramses::EVisibilityMode::Invisible:
                pass = m_nodeFilter.showInvisible;
                break;
            case ramses::EVisibilityMode::Visible:
                pass = m_nodeFilter.showVisible;
                break;
            }
        }
        return pass && m_filter.PassFilter(obj.getName().c_str());
    }

    void SceneViewerGui::drawSceneObjects()
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Scene objects"))
        {
            drawSceneObjectsFilter();

            for (auto& it : m_sceneObjects)
            {
                if (!it.second.empty())
                {
                    const char* typeName = RamsesObjectTypeUtils::GetRamsesObjectTypeName(it.first);
                    if (ImGui::TreeNode(typeName, "%s (%zu)", typeName, it.second.size()))
                    {
                        for (auto* obj : it.second)
                        {
                            draw(obj->impl());
                        }
                        ImGui::TreePop();
                    }
                }
            }

            const auto& slots = m_scene.impl().getIScene().getDataSlots();
            if (slots.getTotalCount() != 0)
            {
                if (ImGui::TreeNode("DataSlots", "Data Slots (%u)", slots.getTotalCount()))
                {
                    for (const auto& it : slots)
                    {
                        drawDataSlot(*(it.second));
                    }
                    ImGui::TreePop();
                }
            }
        }
    }

    void SceneViewerGui::drawNodeHierarchy()
    {
        if (ImGui::CollapsingHeader("Node hierarchy"))
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            ramses::SceneObjectIterator it(m_scene, ramses::ERamsesObjectType::Node);
            while (auto* node = static_cast<ramses::Node*>(it.getNext()))
            {
                if (node->getParent() == nullptr)
                    draw(node->impl());
            }
        }
    }

    void SceneViewerGui::drawResources()
    {
        const bool isOpen = (ImGui::CollapsingHeader("Resources"));

        if (ImGui::BeginPopupContextItem("ResourcesContextMenu"))
        {
            drawMenuItemCopyTexture2D();
            drawMenuItemStorePng();
            drawMenuItemExportShaderSources();
            ImGui::EndPopup();
        }

        if (isOpen)
        {
            m_resourceInfo->reloadIfEmpty();
            ImGui::TextUnformatted(fmt::format("Total: {} resources", m_resourceInfo->totalResources()).c_str());
            const auto used = m_resourceInfo->totalResources() - m_resourceInfo->unavailable();
            ImGui::TextUnformatted(fmt::format("In use: {} resources with {} kB (compressed: {} kB)",
                used, m_resourceInfo->decompressedSize() / 1024U, m_resourceInfo->compressedSize() / 1024U).c_str());
            ImGui::TextUnformatted(fmt::format("Not loaded: {} resources", m_resourceInfo->unavailable()).c_str());
            ImGui::Separator();
            auto displayLimit = m_resourceInfo->getDisplayLimit();
            ImGui::TextUnformatted(fmt::format("Size of {} biggest resources: {} kB", displayLimit, m_resourceInfo->getDisplayedSize() / 1024U).c_str());

            auto orderCriteria = m_resourceInfo->getOrderCriteriaIndex();
            if (ImGui::Combo("Order Criteria", &orderCriteria, m_resourceInfo->orderCriteriaItems.data(), static_cast<int>(m_resourceInfo->orderCriteriaItems.size())))
            {
                m_resourceInfo->setOrderCriteriaIndex(orderCriteria);
            }
            if (ImGui::InputInt("Display limit", &displayLimit))
            {
                m_resourceInfo->setDisplayLimit(displayLimit);
            }
            ImGui::TextUnformatted(fmt::format("Resources sorted by {} (descending):", m_resourceInfo->orderCriteriaItems[m_resourceInfo->getOrderCriteriaIndex()]).c_str());
            for (auto it : *m_resourceInfo)
            {
                draw(it->impl());
            }
        }
    }

    void SceneViewerGui::drawRenderHierarchy()
    {
        if (ImGui::CollapsingHeader("Render hierarchy"))
        {
            ImGui::Text("Sorted by render order");
            ImGui::SameLine();
            if (ImGui::SmallButton("refresh"))
            {
                m_renderInfo.renderableMap.clear();
                m_renderInfo.renderGroupMap.clear();
                m_renderInfo.renderPassVector.clear();
            }

            if (m_renderInfo.renderPassVector.empty())
            {
                const auto& reg = m_scene.impl().getObjectRegistry();
                reg.getObjectsOfType(m_renderInfo.renderPassVector, ramses::ERamsesObjectType::RenderPass);
                std::sort(m_renderInfo.renderPassVector.begin(), m_renderInfo.renderPassVector.end(), [](const auto* a, const auto* b) {
                    return static_cast<const ramses::RenderPass*>(a)->getRenderOrder() < static_cast<const ramses::RenderPass*>(b)->getRenderOrder();
                });
            }
            for (auto pObj : m_renderInfo.renderPassVector)
            {
                auto& renderPass = static_cast<RenderPassImpl&>(pObj->impl());
                const ramses::RenderTarget* rt = renderPass.getRenderTarget();
                const auto handle = renderPass.getObjectRegistryHandle().asMemoryHandle();
                const char* name = renderPass.getName().c_str();

                auto treeNode = [&]() { return ImGui::TreeNode(&renderPass, "RenderPass[%u]: %s", handle, name); };
                auto treeNodeRT = [&]() { return ImGui::TreeNode(&renderPass, "RenderPass[%u]: %s <RT:%ux%u>", handle, name, rt->getWidth(), rt->getHeight()); };
                const bool isOpen = (rt == nullptr) ? drawRamsesObject(renderPass, treeNode) : drawRamsesObject(renderPass, treeNodeRT);
                if (isOpen)
                {
                    drawRenderPass(renderPass);
                    ImGui::TreePop();
                }
            }
        }
    }

    void SceneViewerGui::drawErrors()
    {
        if (m_validationReport.hasIssue())
        {
            if (ImGui::CollapsingHeader("Objects with warnings/errors"))
            {
                if (ImGui::Button("Copy to clipboard"))
                {
                    ImGui::LogToClipboard();
                    ImGui::LogText("IssueType, Message, ObjectType, Name, Id\n");
                    for (auto& issue : m_validationReport.getIssues())
                    {
                        if (issue.object)
                        {
                            auto *sceneObject = issue.object->as<SceneObject>();
                            const auto id = sceneObject ? sceneObject->getSceneObjectId() : sceneObjectId_t();

                            ImGui::LogText("%s\n", fmt::format(R"("{}", "{}", "{}", "{}", {})",
                                EnumToString(issue.type), issue.message, shortName(issue.object->getType()), issue.object->getName(), id.getValue()).c_str());
                        }
                        else
                        {
                            ImGui::LogText("%s\n", fmt::format(R"("{}", "{}")", EnumToString(issue.type), issue.message).c_str());
                        }

                    }
                    ImGui::LogFinish();
                }
                ImGui::Separator();
                const ramses::RamsesObject* lastVisited = nullptr;
                for (auto& msg : m_validationReport.getIssues())
                {
                    if (msg.type <= ramses::EIssueType::Warning)
                    {
                        // draw each object only once
                        if (lastVisited != msg.object && msg.object->isOfType(ERamsesObjectType::SceneObject))
                            draw(const_cast<SceneObjectImpl&>(msg.object->as<SceneObject>()->impl()));
                        lastVisited = msg.object;
                    }
                }
            }
        }
    }

    void SceneViewerGui::drawFile()
    {
        if (ImGui::CollapsingHeader("File"))
        {
            ImGui::Checkbox("Compress scene file", &m_compressFile);
            ImGui::InputText("##filename", &m_filename);
            ImGui::SameLine();
            if (ImGui::Button("Save"))
            {
                File file = File(m_filename);
                if (m_loadedSceneFile == m_filename)
                {
                    m_lastErrorMessage = "Cannot save to the same file that is currently open.";
                }
                else if (file.exists() && !m_alwaysOverwrite)
                {
                    ImGui::OpenPopup("Overwrite?");
                }
                else
                {
                    saveSceneToFile();
                }
            }

            if (ImGui::BeginPopupModal("Overwrite?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("File exists:\n%s\nOverwrite?", m_filename.c_str());
                ImGui::Separator();

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::Checkbox("Don't ask me next time", &m_alwaysOverwrite);
                ImGui::PopStyleVar();

                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    saveSceneToFile();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
    }

    void SceneViewerGui::draw()
    {
        if (ImGui::IsKeyPressed(ramses::EKeyCode_F11))
        {
            m_settings.showWindow = !m_settings.showWindow;
        }

        if (ImGui::IsKeyPressed(ramses::EKeyCode_F10))
        {
            m_settings.showPreview = !m_settings.showPreview;
        }

        if (ImGui::BeginPopupContextVoid("GlobalContextMenu"))
        {
            drawMenuItemShowWindow();
            drawMenuItemShowPreview();
            ImGui::Separator();
            drawMenuItemCopyTexture2D();
            drawMenuItemStorePng();
            drawMenuItemExportShaderSources();
            ImGui::EndPopup();
        }

        if (m_progress.isRunning() && !m_progress.canceled)
        {
            ImGui::OpenPopup("Progress");
        }

        if (ImGui::BeginPopupModal("Progress", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const uint32_t current = m_progress.current;
            ImGui::TextUnformatted(m_progress.getDescription().c_str());
            ImGui::TextUnformatted(fmt::format("{} of {}", current, m_progress.getTotal()).c_str());
            ImGui::Separator();
            if (ImGui::Button("Cancel", ImVec2(120,0)))
            {
                m_progress.canceled = true;
                ImGui::CloseCurrentPopup();
            }
            if (!m_progress.isRunning())
            {
                ImGui::CloseCurrentPopup();
                auto result = m_progress.getResult();
                if (!result.empty())
                {
                    m_lastErrorMessage = fmt::format("{}", fmt::join(result, "\n"));
                }
            }
            ImGui::EndPopup();
        }

        if (!m_lastErrorMessage.empty())
        {
            ImGui::OpenPopup("Error");
        }

        if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted(m_lastErrorMessage.c_str());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                m_lastErrorMessage.clear();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Copy Message", ImVec2(120, 0)))
            {
                ImGui::LogToClipboard();
                ImGui::LogText("%s", m_lastErrorMessage.c_str());
                ImGui::LogFinish();
            }
            ImGui::EndPopup();
        }

        drawSceneTexture();
        drawInspectionWindow();

        if (m_nodeVisibilityChanged)
        {
            // refresh resource information in next iteration
            m_nodeVisibilityChanged = false;
            m_resourceInfo->clear();
        }
    }

    void SceneViewerGui::setSceneTexture(ramses::TextureSampler* sampler, uint32_t width, uint32_t height)
    {
        m_sceneTexture       = sampler;
        m_sceneTextureSize.x = static_cast<float>(width);
        m_sceneTextureSize.y = static_cast<float>(height);
    }

    void SceneViewerGui::zoomIn()
    {
        if (m_settings.zoomIx < (static_cast<int>(m_settings.zoomLevels.size()) - 1))
        {
            ++m_settings.zoomIx;
        }
    }

    void SceneViewerGui::zoomOut()
    {
        if (m_settings.zoomIx > 0)
        {
            --m_settings.zoomIx;
        }
    }

    void SceneViewerGui::drawSceneTexture()
    {
        if (m_sceneTexture)
        {
            if (m_settings.showPreview)
            {
                if (ImGui::Begin("Preview", &m_settings.showPreview, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    if (ImGui::GetIO().KeyCtrl)
                    {
                        if (ImGui::GetIO().MouseWheel >= 1.f)
                        {
                            zoomIn();
                        }
                        if (ImGui::GetIO().MouseWheel <= -1.f)
                        {
                            zoomOut();
                        }
                    }
                    const auto f = m_settings.zoomLevels[m_settings.zoomIx];
                    if (ImGui::SmallButton("-"))
                        zoomOut();
                    ImGui::SameLine();
                    if (ImGui::SmallButton("+"))
                        zoomIn();
                    ImGui::SameLine();
                    ImGui::Text("Zoom %d%%", static_cast<int>(f * 100));
                    ImVec2 size(m_sceneTextureSize.x * f, m_sceneTextureSize.y * f);
                    ImGui::Image(m_sceneTexture, size, ImVec2(0, 1), ImVec2(1, 0));
                    ImGui::End();
                }
                else
                {
                    ImGui::End();
                }
            }
            else
            {
                ImGui::GetBackgroundDrawList()->AddImage(m_sceneTexture, ImVec2(0, 0), m_sceneTextureSize, ImVec2(0, 1), ImVec2(1, 0));
            }
        }
    }

    void SceneViewerGui::drawInspectionWindow()
    {
        if (m_settings.showWindow)
        {
            const std::string windowTitle = fmt::format("[Scene] id:{} name:{}", m_scene.getSceneId().getValue(), m_scene.getName());
            if (!ImGui::Begin(windowTitle.c_str(), &m_settings.showWindow, ImGuiWindowFlags_MenuBar))
            {
                ImGui::End();
                return;
            }
            drawMenuBar();
            drawFile();
            drawSceneObjects();
            drawNodeHierarchy();
            drawResources();
            drawRenderHierarchy();
            drawErrors();
            ImGui::End();
        }
    }

    void SceneViewerGui::drawMenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Tools"))
            {
                drawMenuItemCopyTexture2D();
                drawMenuItemStorePng();
                drawMenuItemExportShaderSources();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings"))
            {
                drawMenuItemShowWindow();
                drawMenuItemShowPreview();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void SceneViewerGui::drawMenuItemShowWindow()
    {
        ImGui::MenuItem("Show Inspection Window", "F11", &m_settings.showWindow);
    }

    void SceneViewerGui::drawMenuItemShowPreview()
    {
        if (m_sceneTexture)
        {
            ImGui::MenuItem("Show Preview Window", "F10", &m_settings.showPreview);
        }
    }

    void SceneViewerGui::drawMenuItemCopyTexture2D()
    {
        if (ImGui::MenuItem("Copy Texture2D list (CSV)"))
        {
            SceneObjectRegistryIterator iter(m_scene.impl().getObjectRegistry(), ramses::ERamsesObjectType::Texture2D);
            ImGui::LogToClipboard();
            ImGui::LogText("Id, Name, Type, Hash, Loaded, Size, CompressedSize, Width, Height, Format, Swizzle, GenerateMipChain\n");
            while (auto* obj = iter.getNextNonConst<ramses::Texture2D>())
            {
                logTexture2D(obj->impl());
            }
            ImGui::LogFinish();
        }
    }

    template <class F>
    void SceneViewerGui::processObjectsAsync(F&& func, ramses::ERamsesObjectType objType, const char* message)
    {
        SceneObjectVector objects;
        m_scene.impl().getObjectRegistry().getObjectsOfType(objects, objType);
        m_progress.stop();
        ProgressMonitor::FutureList futures;
        const size_t tasks     = objects.size() > 16u ? 4u : 1u;
        const auto   chunkSize = static_cast<ptrdiff_t>(objects.size() / tasks);
        for (size_t i = 0; i < tasks; ++i)
        {
            const auto begin = objects.begin() + static_cast<ptrdiff_t>(i) * chunkSize;
            if (i + 1 == tasks)
            {
                futures.push_back(std::async(std::launch::async, func, SceneObjectVector(begin, objects.end())));
            }
            else
            {
                futures.push_back(std::async(std::launch::async, func, SceneObjectVector(begin, begin + chunkSize)));
            }
        }
        m_progress.start(std::move(futures), static_cast<uint32_t>(objects.size()), message);
    }

    void SceneViewerGui::drawMenuItemStorePng()
    {
        if (ImGui::MenuItem("Export all 2D textures to png"))
        {
            auto storeAllTextures = [&](SceneObjectVector objects) {
                std::vector<std::string> errorList;
                for (auto it = objects.begin(); it != objects.end() && !m_progress.canceled; ++it)
                {
                    const ramses::Texture2D* obj = static_cast<ramses::Texture2D*>(*it);
                    ++m_progress.current;
                    const auto error = saveTexture2D(obj->impl());
                    if (!error.empty())
                    {
                        errorList.push_back(error);
                    }
                }
                return errorList;
            };

            processObjectsAsync(storeAllTextures, ramses::ERamsesObjectType::Texture2D, "Saving Texture2D to png");
        }
    }

    void SceneViewerGui::drawMenuItemExportShaderSources()
    {
        if (ImGui::MenuItem("Export all shader sources"))
        {
            auto exportShaders = [&](SceneObjectVector objects) {
                std::vector<std::string> errorList;
                for (auto it = objects.begin(); it != objects.end() && !m_progress.canceled; ++it)
                {
                    const ramses::Effect* obj = static_cast<ramses::Effect*>(*it);
                    ++m_progress.current;
                    const auto error = saveShaderSources(obj->impl());
                    if (!error.empty())
                    {
                        errorList.push_back(error);
                    }
                }
                return errorList;
            };

            processObjectsAsync(exportShaders, ramses::ERamsesObjectType::Effect, "Exporting shader sources");
        }
    }

    void SceneViewerGui::saveSceneToFile()
    {
        SaveFileConfig config;
        config.setCompressionEnabled(m_compressFile);
        config.setValidationEnabled(false);
        if (!m_scene.saveToFile(m_filename.c_str(), config))
        {
            m_lastErrorMessage = m_scene.getRamsesClient().getRamsesFramework().getLastError().value_or(Issue{}).message;
        }
    }

    std::string SceneViewerGui::saveTexture2D(const Texture2DImpl& obj) const
    {
        auto resource = m_scene.getRamsesClient().impl().getResource(obj.getLowlevelResourceHash());
        std::string errorMsg;
        if (resource)
        {
            const auto* textureResource = resource->convertTo<ramses::internal::TextureResource>();
            const auto filename = fmt::format("{:04}_{}.png", obj.getObjectRegistryHandle().asMemoryHandle(), obj.getName());
            errorMsg = imgui::SaveTextureToPng(textureResource, filename);
        }
        return errorMsg;
    }

    std::string SceneViewerGui::saveShaderSources(const EffectImpl& obj) const
    {
        auto resource = m_scene.getRamsesClient().impl().getResource(obj.getLowlevelResourceHash());
        std::string errorMsg;
        if (resource)
        {
            auto save = [&obj, &errorMsg](const char* const format, const char* text) {
                std::string strText = text != nullptr ? text : "";
                if (!strText.empty())
                {
                    const auto fileName = fmt::format(format, obj.getLowlevelResourceHash());

                    ramses::internal::File outputFile(fileName.c_str());

                    if (!outputFile.open(File::Mode::WriteNewBinary))
                    {
                        errorMsg = fmt::format("Could not open file for writing: {}", fileName);
                        return false;
                    }
                    if (!outputFile.write(strText.c_str(), strText.size()))
                    {
                        errorMsg = fmt::format("Write to file failed: {}", fileName);
                        return false;
                    }
                    if (!outputFile.close())
                    {
                        errorMsg = fmt::format("Close file failed: {}", fileName);
                        return false;
                    }
                }
                return true;
            };

            const auto effectRes = resource->convertTo<ramses::internal::EffectResource>();

            save("{}.frag", effectRes->getFragmentShader())
                && save("{}.vert", effectRes->getVertexShader())
                && save("{}.geom", effectRes->getGeometryShader());
        }
        return errorMsg;
    }

    void SceneViewerGui::setVisibility(NodeImpl& node, ramses::EVisibilityMode visibility)
    {
        node.setVisibility(visibility);
        // can't clear the resource cache immediately, because it might be currently iterated
        m_nodeVisibilityChanged = true;
    }

    void SceneViewerGui::LogRamsesObject(SceneObjectImpl& obj)
    {
        ImGui::LogText(R"(%lu,"%s","%s",)", obj.getSceneObjectId().getValue(), obj.getName().c_str(), shortName(obj.getType()));
    }

    void SceneViewerGui::logResource(ResourceImpl& obj)
    {
        LogRamsesObject(obj);
        const auto hash = obj.getLowlevelResourceHash();
        auto resource = m_scene.getRamsesClient().impl().getResource(hash);
        ImGui::LogText("%s", fmt::format("{},", hash).c_str());
        if (resource)
        {
            ImGui::LogText("true, %u, %u,", resource->getDecompressedDataSize(), resource->getCompressedDataSize());
        }
        else
        {
            ImGui::LogText("false, 0, 0,");
        }
    }

    void SceneViewerGui::logTexture2D(Texture2DImpl& obj)
    {
        logResource(obj);
        ImGui::LogText("%u,%u,%s,", obj.getWidth(), obj.getHeight(), ramses::toString(obj.getTextureFormat()));
        const auto& swizzle = obj.getTextureSwizzle();
        ImGui::LogText("r:%s g:%s b:%s a:%s,",
            EnumToString(swizzle.channelRed),
            EnumToString(swizzle.channelGreen),
            EnumToString(swizzle.channelBlue),
            EnumToString(swizzle.channelAlpha));
        auto resource = m_scene.getRamsesClient().impl().getResource(obj.getLowlevelResourceHash());
        if (resource)
        {
            const auto* textureResource = resource->convertTo<ramses::internal::TextureResource>();
            ImGui::LogText("%s,", textureResource->getGenerateMipChainFlag() ? "true" : "false");
        }
        else
        {
            ImGui::LogText(",");
        }
        const auto* slot = findDataSlot(obj.getLowlevelResourceHash());
        if (slot)
            ImGui::LogText("DataSlot: %u %s,", slot->id.getValue(), EnumToString(slot->type));
        ImGui::LogText("\n");
    }
}
