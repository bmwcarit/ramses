//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  Copyright (C) 2019 Daniel Werner Lima Souza de Almeida (dwlsalmeida@gmail.com)
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SceneViewerGui.h"
#include "SceneDumper.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-client-api/Scene.h"
#include "ramses-utils.h"

#include "SceneImpl.h"
#include "NodeImpl.h"
#include "PickableObjectImpl.h"
#include "MeshNodeImpl.h"
#include "CameraNodeImpl.h"
#include "MeshNodeImpl.h"
#include "RenderPassImpl.h"
#include "RenderGroupImpl.h"
#include "RenderTargetImpl.h"
#include "RenderBufferImpl.h"
#include "GeometryBindingImpl.h"
#include "ArrayBufferImpl.h"
#include "EffectImpl.h"
#include "Texture2DImpl.h"
#include "Texture3DImpl.h"
#include "Texture2DBufferImpl.h"
#include "TextureCubeImpl.h"
#include "StreamTextureImpl.h"
#include "TextureSamplerImpl.h"
#include "DataObjectImpl.h"
#include "RamsesClientImpl.h"
#include "EffectInputImpl.h"
#include "SceneReferenceImpl.h"
#include "AnimationSystemImpl.h"
#include "BlitPassImpl.h"
#include "RamsesObjectRegistryIterator.h"
#include "Resource/EffectResource.h"
#include "Resource/TextureResource.h"
#include "RotationConventionUtils.h"
#include "TextureUtils.h"
#include "Scene/ClientScene.h"
#include "Utils/File.h"

namespace ramses_internal
{
    namespace
    {
        void getRotation(ramses::NodeImpl* node, float& x, float& y, float& z, ramses_internal::ERotationConvention& rotationConventionInternal)
        {
            auto handle = node->getTransformHandle();
            if (handle.isValid() && ramses_internal::ERotationConvention::Legacy_ZYX == node->getIScene().getRotationConvention(handle))
            {
                rotationConventionInternal = ramses_internal::ERotationConvention::Legacy_ZYX;
                node->getRotation(x, y, z);
            }
            else
            {
                ramses::ERotationConvention rotationConvention;
                node->getRotation(x, y, z, rotationConvention);
                rotationConventionInternal = ramses::RotationConventionUtils::ConvertRotationConventionToInternal(rotationConvention);
            }
        }

        void setRotation(ramses::NodeImpl* node, float x, float y, float z, ramses_internal::ERotationConvention rotationConventionInternal)
        {
            if (rotationConventionInternal == ramses_internal::ERotationConvention::Legacy_ZYX)
                node->setRotation(x, y, z);
            else
            {
                const auto rotationConvention = ramses::RotationConventionUtils::ConvertDataTypeFromInternal(rotationConventionInternal);
                node->setRotation(x, y, z, rotationConvention);
            }
        }

        ramses::EVisibilityMode getEffectiveVisibility(const ramses::NodeImpl& obj)
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

        int32_t getRenderOrder(const ramses::RenderGroupImpl& rg, const ramses::RamsesObjectImpl& child)
        {
            int32_t order = 0;
            switch (child.getType())
            {
            case ramses::ERamsesObjectType_MeshNode:
                rg.getMeshNodeOrder(static_cast<const ramses::MeshNodeImpl&>(child), order);
                break;
            case ramses::ERamsesObjectType_RenderGroup:
                rg.getRenderGroupOrder(static_cast<const ramses::RenderGroupImpl&>(child), order);
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
            case ramses::EDataType::ByteBlob:
                return "ByteBlob";
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

        const char* shortName(ramses_internal::EDataType t) {
            const char* name = EnumToString(t);
            assert(std::string(name).find("DATATYPE_") == 0);
            return name + 9;
        }

        const char* shortName(ramses::ERamsesObjectType t)
        {
            const char* name = ramses::RamsesObjectTypeUtils::GetRamsesObjectTypeName(t);
            assert(std::string(name).find("ERamsesObjectType_") == 0);
            return name + 18;
        }

    } // namespace

    template<class C>
    bool SceneViewerGui::drawRamsesObject(ramses::RamsesObjectImpl& obj, const C& drawTreeNode)
    {
        const char* report = obj.getValidationReport(ramses::EValidationSeverity_Warning);
        const bool hasIssues = report && report[0] != 0;
        const bool isUnused = !m_usedObjects.contains(&obj);
        if (hasIssues)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImColor(255, 0, 0).Value);
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
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", report);
            if (isOpen)
                ImGui::TextWrapped("%s", report);
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

    bool SceneViewerGui::drawRamsesObject(ramses::RamsesObjectImpl& obj)
    {
        return drawRamsesObject(obj, [&]() {
            return ImGui::TreeNode(&obj, "%s[%u]: %s", shortName(obj.getType()), obj.getObjectRegistryHandle().asMemoryHandle(), obj.getName().c_str());
            });
    }

    void SceneViewerGui::drawUnusedObject(ramses::RamsesObjectImpl& obj)
    {
        if (obj.isOfType(ramses::ERamsesObjectType_Resource))
        {
            ImGui::Text("Unused or duplicate resource");
            if (ImGui::TreeNode("Duplicates (same hash):"))
            {
                updateResourceInfo();
                auto& hlResource = static_cast<ramses::ResourceImpl&>(obj);
                auto range = m_resourceInfo.hashLookup.equal_range(hlResource.getLowlevelResourceHash());
                for (auto it = range.first; it != range.second; ++it)
                {
                    draw(it->second->impl);
                }
                ImGui::TreePop();
            }
        }
        else
        {
            switch (obj.getType())
            {
            case ramses::ERamsesObjectType_Node:
                ImGui::Text("Unused node (not a parent of a mesh node or camera node)");
                break;
            default:
                ImGui::Text("Unused object");
                break;
            }
        }
    }

    SceneViewerGui::SceneViewerGui(ramses::Scene& scene, const std::string& filename, ImguiClientHelper& imguiHelper)
        : m_scene(scene)
        , m_loadedSceneFile(filename)
        , m_filename(filename)
        , m_imageCache(imguiHelper.getScene())
    {
        ramses_internal::StringOutputStream dummyStream;
        ramses::SceneDumper sceneDumper(scene.impl);
        sceneDumper.dumpUnrequiredObjects(dummyStream);
        m_usedObjects = sceneDumper.getRequiredObjects();
        const char* report = m_scene.getValidationReport(ramses::EValidationSeverity_Warning);
        m_hasSceneErrors = ((report != nullptr) && (report[0] != 0));
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    }

    const ramses::RenderBuffer* SceneViewerGui::findRenderBuffer(ramses_internal::RenderBufferHandle renderBufferHandle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isRenderBufferAllocated(renderBufferHandle);
        if (isAllocated)
        {
            ramses::RamsesObjectRegistryIterator iter(m_scene.impl.getObjectRegistry(), ramses::ERamsesObjectType_RenderBuffer);
            while (const ramses::RenderBuffer* renderBuffer = iter.getNext<ramses::RenderBuffer>())
            {
                if (renderBufferHandle == renderBuffer->impl.getRenderBufferHandle())
                {
                    return renderBuffer;
                }
            }
        }
        return nullptr;
    }

    const ramses::StreamTexture* SceneViewerGui::findStreamTexture(ramses_internal::StreamTextureHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isStreamTextureAllocated(handle);
        if (isAllocated)
        {
            ramses::RamsesObjectRegistryIterator iter(m_scene.impl.getObjectRegistry(), ramses::ERamsesObjectType_StreamTexture);
            while (const ramses::StreamTexture* streamTexture = iter.getNext<ramses::StreamTexture>())
            {
                if (handle == streamTexture->impl.getHandle())
                {
                    return streamTexture;
                }
            }
        }
        return nullptr;
    }

    const ramses::Texture2DBuffer* SceneViewerGui::findTextureBuffer(ramses_internal::TextureBufferHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isTextureBufferAllocated(handle);
        if (isAllocated)
        {
            ramses::RamsesObjectRegistryIterator iter(m_scene.impl.getObjectRegistry(), ramses::ERamsesObjectType_Texture2DBuffer);
            while (const ramses::Texture2DBuffer* textureBuffer = iter.getNext<ramses::Texture2DBuffer>())
            {
                if (handle == textureBuffer->impl.getTextureBufferHandle())
                {
                    return textureBuffer;
                }
            }
        }
        return nullptr;
    }

    const ramses::TextureSampler* SceneViewerGui::findTextureSampler(ramses_internal::TextureSamplerHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isTextureSamplerAllocated(handle);
        if (isAllocated)
        {
            ramses::RamsesObjectRegistryIterator iter(m_scene.impl.getObjectRegistry(), ramses::ERamsesObjectType_TextureSampler);
            while (const ramses::TextureSampler* textureSampler = iter.getNext<ramses::TextureSampler>())
            {
                if (handle == textureSampler->impl.getTextureSamplerHandle())
                {
                    return textureSampler;
                }
            }
        }
        return nullptr;
    }

    const ramses::ArrayBuffer* SceneViewerGui::findDataBuffer(ramses_internal::DataBufferHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isDataBufferAllocated(handle);
        if (isAllocated)
        {
            ramses::RamsesObjectRegistryIterator iter(m_scene.impl.getObjectRegistry(), ramses::ERamsesObjectType_DataBufferObject);
            while (const ramses::ArrayBuffer* dataBuffer = iter.getNext<ramses::ArrayBuffer>())
            {
                if (handle == dataBuffer->impl.getDataBufferHandle())
                {
                    return dataBuffer;
                }
            }
        }
        return nullptr;
    }

    const ramses::Node* SceneViewerGui::findNode(ramses_internal::NodeHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isNodeAllocated(handle);
        if (isAllocated)
        {
            ramses::RamsesObjectRegistryIterator iter(m_scene.impl.getObjectRegistry(), ramses::ERamsesObjectType_Node);
            while (const ramses::Node* node = iter.getNext<ramses::Node>())
            {
                if (handle == node->impl.getNodeHandle())
                {
                    return node;
                }
            }
        }
        return nullptr;
    }

    const ramses::DataObject* SceneViewerGui::findDataObject(ramses_internal::DataInstanceHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isDataInstanceAllocated(handle);
        if (isAllocated)
        {
            ramses::RamsesObjectVector objects;
            m_scene.impl.getObjectRegistry().getObjectsOfType(objects, ramses::ERamsesObjectType_DataObject);
            for (auto it = objects.begin(); it != objects.end(); ++it)
            {
                if (handle == static_cast<ramses::DataObject*>(*it)->impl.getDataReference())
                {
                    return static_cast<ramses::DataObject*>(*it);
                }
            }
        }
        return nullptr;
    }

    const ramses::Texture2D* SceneViewerGui::findTexture2D(ramses_internal::ResourceContentHash hash) const
    {
        if (hash.isValid())
        {
            ramses::RamsesObjectRegistryIterator iter(m_scene.impl.getObjectRegistry(), ramses::ERamsesObjectType_Texture2D);
            while (const ramses::Texture2D* texture = iter.getNext<ramses::Texture2D>())
            {
                if (texture->impl.getLowlevelResourceHash() == hash)
                {
                    return texture;
                }
            }
        }
        return nullptr;
    }

    const ramses_internal::DataSlot* SceneViewerGui::findDataSlot(ramses_internal::NodeHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isNodeAllocated(handle);
        if (isAllocated)
        {
            const auto& slots = m_scene.impl.getIScene().getDataSlots();
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

    const ramses_internal::DataSlot* SceneViewerGui::findDataSlot(ramses_internal::DataInstanceHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isDataInstanceAllocated(handle);
        if (isAllocated)
        {
            const auto& slots = m_scene.impl.getIScene().getDataSlots();
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

    const ramses_internal::DataSlot* SceneViewerGui::findDataSlot(ramses_internal::TextureSamplerHandle handle) const
    {
        const bool isAllocated = m_scene.impl.getIScene().isTextureSamplerAllocated(handle);
        if (isAllocated)
        {
            const auto& slots = m_scene.impl.getIScene().getDataSlots();
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

    const ramses_internal::DataSlot* SceneViewerGui::findDataSlot(ramses_internal::ResourceContentHash hash) const
    {
        if (hash.isValid())
        {
            const auto& slots = m_scene.impl.getIScene().getDataSlots();
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
    void SceneViewerGui::drawRefs(const char* headline, const ramses::RamsesObjectImpl& target, Filter filter)
    {
        const RefKey key = {&target, headline};
        auto   result = m_refs.insert({key, ramses::RamsesObjectVector()});

        ramses::RamsesObjectVector& filteredList = result.first->second;
        if (result.second)
        {
            // fill the list initially
            const auto type = ramses::TYPE_ID_OF_RAMSES_OBJECT<T>::ID;
            ramses::RamsesObjectVector objects;
            m_scene.impl.getObjectRegistry().getObjectsOfType(objects, type);
            for (ramses::RamsesObject* obj : objects)
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
                for (ramses::RamsesObject* obj : filteredList)
                {
                    draw(obj->impl);
                }
                ImGui::TreePop();
            }
        }
    }

    void SceneViewerGui::draw(ramses::RamsesObjectImpl& obj)
    {
        if (drawRamsesObject(obj))
        {
            switch (obj.getType())
            {
            case ramses::ERamsesObjectType_Node:
                drawNode(static_cast<ramses::NodeImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_PickableObject:
                drawPickableObject(static_cast <ramses::PickableObjectImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_MeshNode:
                drawMeshNode(static_cast<ramses::MeshNodeImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_PerspectiveCamera:
            case ramses::ERamsesObjectType_OrthographicCamera:
                drawCameraNode(static_cast<ramses::CameraNodeImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_Effect:
                drawEffect(static_cast<ramses::EffectImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_RenderPass:
                drawRenderPass(static_cast<ramses::RenderPassImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_RenderGroup:
                drawRenderGroup(static_cast<ramses::RenderGroupImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_Appearance:
                drawAppearance(static_cast<ramses::AppearanceImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_GeometryBinding:
                drawGeometryBinding(static_cast<ramses::GeometryBindingImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_Texture2D:
                drawTexture2D(static_cast<ramses::Texture2DImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_Texture3D:
                drawTexture3D(static_cast<ramses::Texture3DImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_Texture2DBuffer:
                drawTexture2DBuffer(static_cast<ramses::Texture2DBufferImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_TextureCube:
                drawTextureCube(static_cast<ramses::TextureCubeImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_StreamTexture:
                drawStreamTexture(static_cast<ramses::StreamTextureImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_TextureSampler:
            case ramses::ERamsesObjectType_TextureSamplerMS:
                drawTextureSampler(static_cast<ramses::TextureSamplerImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_ArrayResource:
                drawArrayResource(static_cast<ramses::ArrayResourceImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_RenderTarget:
                drawRenderTarget(static_cast<ramses::RenderTargetImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_RenderBuffer:
                drawRenderBuffer(static_cast<ramses::RenderBufferImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_SceneReference:
                drawSceneReference(static_cast<ramses::SceneReferenceImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_DataFloat:
            case ramses::ERamsesObjectType_DataInt32:
            case ramses::ERamsesObjectType_DataVector2f:
            case ramses::ERamsesObjectType_DataVector3f:
            case ramses::ERamsesObjectType_DataVector4f:
            case ramses::ERamsesObjectType_DataVector2i:
            case ramses::ERamsesObjectType_DataVector3i:
            case ramses::ERamsesObjectType_DataVector4i:
            case ramses::ERamsesObjectType_DataMatrix22f:
            case ramses::ERamsesObjectType_DataMatrix33f:
            case ramses::ERamsesObjectType_DataMatrix44f:
                drawDataObject(static_cast<ramses::DataObjectImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_DataBufferObject:
                drawArrayBuffer(static_cast<ramses::ArrayBufferImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_BlitPass:
                drawBlitPass(static_cast<ramses::BlitPassImpl&>(obj));
                break;
            case ramses::ERamsesObjectType_Invalid:
            case ramses::ERamsesObjectType_ClientObject:
            case ramses::ERamsesObjectType_RamsesObject:
            case ramses::ERamsesObjectType_SceneObject:
            case ramses::ERamsesObjectType_AnimationObject:
            case ramses::ERamsesObjectType_Client:
            case ramses::ERamsesObjectType_Scene:
            case ramses::ERamsesObjectType_AnimationSystem:
            case ramses::ERamsesObjectType_AnimationSystemRealTime:
            case ramses::ERamsesObjectType_Camera:
            case ramses::ERamsesObjectType_AnimatedProperty:
            case ramses::ERamsesObjectType_Animation:
            case ramses::ERamsesObjectType_AnimationSequence:
            case ramses::ERamsesObjectType_Spline:
            case ramses::ERamsesObjectType_SplineStepBool:
            case ramses::ERamsesObjectType_SplineStepFloat:
            case ramses::ERamsesObjectType_SplineStepInt32:
            case ramses::ERamsesObjectType_SplineStepVector2f:
            case ramses::ERamsesObjectType_SplineStepVector3f:
            case ramses::ERamsesObjectType_SplineStepVector4f:
            case ramses::ERamsesObjectType_SplineStepVector2i:
            case ramses::ERamsesObjectType_SplineStepVector3i:
            case ramses::ERamsesObjectType_SplineStepVector4i:
            case ramses::ERamsesObjectType_SplineLinearFloat:
            case ramses::ERamsesObjectType_SplineLinearInt32:
            case ramses::ERamsesObjectType_SplineLinearVector2f:
            case ramses::ERamsesObjectType_SplineLinearVector3f:
            case ramses::ERamsesObjectType_SplineLinearVector4f:
            case ramses::ERamsesObjectType_SplineLinearVector2i:
            case ramses::ERamsesObjectType_SplineLinearVector3i:
            case ramses::ERamsesObjectType_SplineLinearVector4i:
            case ramses::ERamsesObjectType_SplineBezierFloat:
            case ramses::ERamsesObjectType_SplineBezierInt32:
            case ramses::ERamsesObjectType_SplineBezierVector2f:
            case ramses::ERamsesObjectType_SplineBezierVector3f:
            case ramses::ERamsesObjectType_SplineBezierVector4f:
            case ramses::ERamsesObjectType_SplineBezierVector2i:
            case ramses::ERamsesObjectType_SplineBezierVector3i:
            case ramses::ERamsesObjectType_SplineBezierVector4i:
            case ramses::ERamsesObjectType_Resource:
            case ramses::ERamsesObjectType_DataObject:
            case ramses::ERamsesObjectType_NUMBER_OF_TYPES:
                ImGui::Text("tbd.");
            }
            ImGui::TreePop();
        }
    }

    void SceneViewerGui::drawNode(ramses::NodeImpl& obj)
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
            float                                xyz[3];
            ramses_internal::ERotationConvention rotationConvention;
            getRotation(&obj, xyz[0], xyz[1], xyz[2], rotationConvention);
            if (ImGui::DragFloat3("Rotation (x,y,z)", xyz, 1.0f, -360.f, 360.f, "%.1f"))
                setRotation(&obj, xyz[0], xyz[1], xyz[2], rotationConvention);
            int rotationConventionInt = static_cast<int>(rotationConvention);
            if (ImGui::Combo("RotationConvention", &rotationConventionInt, ramses_internal::ERotationConventionNames, 13, -1))
                setRotation(&obj, xyz[0], xyz[1], xyz[2], static_cast<ramses_internal::ERotationConvention>(rotationConventionInt));
            obj.getTranslation(xyz[0], xyz[1], xyz[2]);
            if (ImGui::DragFloat3("Translation (x,y,z)", xyz, 0.01f, 0.f, 0.f, "%.3f"))
                obj.setTranslation(xyz[0], xyz[1], xyz[2]);
            obj.getScaling(xyz[0], xyz[1], xyz[2]);
            if (ImGui::DragFloat3("Scaling (x,y,z)", xyz, 0.01f, 0.f, 0.f, "%.3f"))
                obj.setScaling(xyz[0], xyz[1], xyz[2]);
            ImGui::TreePop();
        }
        if (ramses::ERamsesObjectType_Node == obj.getType())
        {
            drawNodeChildrenParent(obj);
        }
    }

    void SceneViewerGui::drawNodeChildrenParent(ramses::NodeImpl& obj)
    {
        for (uint32_t i = 0; i < obj.getChildCount(); ++i)
        {
            draw(obj.getChild(i)->impl);
        }
        if (obj.getParentImpl() != nullptr)
        {
            ImGui::Text("Parent:");
            ImGui::SameLine();
            draw(*obj.getParentImpl());
        }
    }

    void SceneViewerGui::drawMeshNode(ramses::MeshNodeImpl& obj)
    {
        drawNode(obj);
        draw(obj.getAppearance()->impl);
        draw(obj.getGeometryBinding()->impl);
        drawNodeChildrenParent(obj);
        drawRefs<ramses::RenderGroup>("Used by RenderGroup", obj, [&](const ramses::RenderGroup* ref) { return ref->impl.contains(obj); });
    }

    void SceneViewerGui::drawPickableObject(ramses::PickableObjectImpl& obj)
    {
        drawNode(obj);
        draw(obj.getGeometryBuffer().impl);
        auto camera = obj.getCamera();
        if (camera != nullptr)
            draw(camera->impl);
        drawNodeChildrenParent(obj);
    }

    void SceneViewerGui::drawCameraNode(ramses::CameraNodeImpl& obj)
    {
        drawNode(obj);
        int32_t xy[2];
        int32_t wh[2];
        xy[0] = obj.getViewportX();
        xy[1] = obj.getViewportY();
        wh[0] = obj.getViewportWidth();
        wh[1] = obj.getViewportHeight();
        if (obj.isViewportOffsetBound())
        {
            auto dataObject = findDataObject(obj.getViewportOffsetHandle());
            ImGui::Text("Viewport offset:");
            if (dataObject != nullptr)
            {
                ImGui::SameLine();
                draw(dataObject->impl);
            }
        }
        else
        {
            if (ImGui::DragInt2("Viewport Offset(x, y)", xy))
                obj.setViewport(xy[0], xy[1], wh[0], wh[1]);
        }

        if (obj.isViewportSizeBound())
        {
            auto dataObject = findDataObject(obj.getViewportSizeHandle());
            ImGui::Text("Viewport size:");
            if (dataObject != nullptr)
            {
                ImGui::SameLine();
                draw(dataObject->impl);
            }
        }
        else
        {
            if (ImGui::DragInt2("Viewport Size(w, h)", wh))
                obj.setViewport(xy[0], xy[1], wh[0], wh[1]);
        }

        if (obj.isFrustumPlanesBound())
        {
            ImGui::Text("Frustrum (l,r,b,t):");
            auto frustrum = findDataObject(obj.getFrustrumPlanesHandle());
            if (frustrum != nullptr)
            {
                ImGui::SameLine();
                draw(frustrum->impl);
            }

            ImGui::Text("Frustrum (n,f):");
            auto nearFar = findDataObject(obj.getFrustrumNearFarPlanesHandle());
            if (nearFar != nullptr)
            {
                ImGui::SameLine();
                draw(nearFar->impl);
            }
        }
        else
        {
            float lrbp[4];
            lrbp[0] = obj.getLeftPlane();
            lrbp[1] = obj.getRightPlane();
            lrbp[2] = obj.getBottomPlane();
            lrbp[3] = obj.getTopPlane();
            float nf[2];
            nf[0] = obj.getNearPlane();
            nf[1] = obj.getFarPlane();
            if (ImGui::DragFloat4("Frustrum (l,r,b,t)", lrbp, 0.001f))
                obj.setFrustum(lrbp[0], lrbp[1], lrbp[2], lrbp[3], nf[0], nf[1]);
            if (ImGui::DragFloat2("Frustrum (n,f)", nf, 0.1f))
                obj.setFrustum(lrbp[0], lrbp[1], lrbp[2], lrbp[3], nf[0], nf[1]);
            if (obj.getType() == ramses::ERamsesObjectType_PerspectiveCamera)
            {
                float va[2];
                va[0] = obj.getVerticalFieldOfView();
                va[1] = obj.getAspectRatio();
                if (ImGui::DragFloat2("VerticalFoV, AspectRatio", va, 0.1f))
                    obj.setPerspectiveFrustum(va[0], va[1], nf[0], nf[1]);
            }
        }
        drawNodeChildrenParent(obj);
        drawRefs<ramses::RenderPass>("Used by RenderPass", obj, [&](const ramses::RenderPass* ref) {
            return ref->impl.getCamera() == &obj.getRamsesObject();
        });
    }

    void SceneViewerGui::drawResource(ramses::ResourceImpl& obj)
    {
        const auto hash = obj.getLowlevelResourceHash();
        auto resource = m_scene.getRamsesClient().impl.getResource(hash);
        ImGui::Text("Hash: %" PRIX64 ":%" PRIX64, hash.highPart, hash.lowPart);
        if (resource)
        {
            double      size       = resource->getDecompressedDataSize();
            double      compressed = resource->getCompressedDataSize();
            const char* unit       = "Bytes";
            if (size > 1024)
            {
                unit = "kB";
                size /= 1024.0;
                compressed /= 1024.0;
            }
            ImGui::Text("Resource size (%s): %.1f (compressed %.1f)", unit, size, compressed);
        }
        else
        {
            ImGui::Text("Resource not loaded");
        }
    }

    void SceneViewerGui::drawEffect(ramses::EffectImpl& obj)
    {
        drawResource(obj);
        auto resource = m_scene.getRamsesClient().impl.getResource(obj.getLowlevelResourceHash());
        if (resource)
        {
            if (ImGui::Button("Shader Sources..."))
                ImGui::OpenPopup("shader_src");
            if (ImGui::BeginPopup("shader_src"))
            {
                const auto effectRes = resource->convertTo<ramses_internal::EffectResource>();
                if (ImGui::CollapsingHeader("Vertex Shader", ImGuiTreeNodeFlags_DefaultOpen))
                    ImGui::Text("%s", effectRes->getVertexShader());
                if (ImGui::CollapsingHeader("Fragment Shader", ImGuiTreeNodeFlags_DefaultOpen))
                    ImGui::Text("%s", effectRes->getFragmentShader());
                if (ImGui::CollapsingHeader("Geometry Shader", ImGuiTreeNodeFlags_DefaultOpen))
                    ImGui::Text("%s", effectRes->getGeometryShader());
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

        drawRefs<ramses::Appearance>("Used by Appearance", obj, [&](const ramses::Appearance* ref) { return ref->impl.getEffectImpl()== &obj; });
        drawRefs<ramses::GeometryBinding>("Used by GeometryBinding", obj, [&](const ramses::GeometryBinding* ref) { return &ref->impl.getEffect().impl == &obj; });
    }

    void SceneViewerGui::drawRenderPass(ramses::RenderPassImpl& obj)
    {
        int32_t renderOrder = obj.getRenderOrder();
        if (ImGui::DragInt("RenderOrder", &renderOrder))
            obj.setRenderOrder(renderOrder);
        uint32_t clearFlags = obj.getClearFlags();
        if (ImGui::TreeNode("Clear"))
        {
            if (ImGui::CheckboxFlags("Color", &clearFlags, ramses::EClearFlags_Color))
                obj.setClearFlags(clearFlags);
            if (ImGui::CheckboxFlags("Depth", &clearFlags, ramses::EClearFlags_Depth))
                obj.setClearFlags(clearFlags);
            if (ImGui::CheckboxFlags("Stencil", &clearFlags, ramses::EClearFlags_Stencil))
                obj.setClearFlags(clearFlags);
            if (clearFlags & ramses::EClearFlags_Color)
            {
                float rgba[4];
                const auto& color = obj.getClearColor();
                rgba[0] = color.r;
                rgba[1] = color.g;
                rgba[2] = color.b;
                rgba[3] = color.a;
                if (ImGui::ColorEdit4("ClearColor", rgba))
                    obj.setClearColor(ramses_internal::Vector4(rgba[0], rgba[1], rgba[2], rgba[3]));
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

        draw(obj.getCamera()->impl);

        auto rt = obj.getRenderTarget();
        if (rt != nullptr)
            draw(rt->impl);
        else
            ImGui::Text("No render target");

        ImGui::Text("RenderGroups:");
        auto& renderGroups = m_renderInfo.renderGroupMap[&obj];
        if (renderGroups.empty())
        {
            renderGroups = obj.getAllRenderGroups();
            std::sort(renderGroups.begin(), renderGroups.end(), [&](const auto* a, const auto* b) {
                int32_t orderA;
                int32_t orderB;
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
            draw(*const_cast<ramses::RenderGroupImpl*>(it));
        }
    }

    void SceneViewerGui::drawRenderGroup(ramses::RenderGroupImpl& obj)
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
                if (it->getType() == ramses::ERamsesObjectType_MeshNode)
                {
                    const auto* meshNode = static_cast<const ramses::MeshNodeImpl*>(it);
                    obj.removeIfContained(*meshNode);
                    obj.addMeshNode(*meshNode, order);
                }
                else
                {
                    const auto* renderGroup = static_cast<const ramses::RenderGroupImpl*>(it);
                    obj.removeIfContained(*renderGroup);
                    obj.addRenderGroup(*renderGroup, order);
                }
            }
            ImGui::PopID();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Render order");
            ImGui::SameLine();
            draw(*const_cast<ramses::RamsesObjectImpl*>(it));
        }

        drawRefs<ramses::RenderPass>("Used by RenderPass", obj, [&](const ramses::RenderPass* ref) {
            const auto& groups = ref->impl.getAllRenderGroups();
            return std::find(groups.begin(), groups.end(), &obj) != groups.end();
        });
    }

    void SceneViewerGui::drawRenderTarget(ramses::RenderTargetImpl& obj)
    {
        const auto rtHandle = obj.getRenderTargetHandle();
        const uint32_t numBuffers = m_scene.impl.getIScene().getRenderTargetRenderBufferCount(rtHandle);
        for (uint32_t i = 0; i < numBuffers; ++i)
        {
            const auto rbHandle = m_scene.impl.getIScene().getRenderTargetRenderBuffer(rtHandle, i);
            const ramses::RenderBuffer* rb = findRenderBuffer(rbHandle);
            if (rb != nullptr)
                draw(rb->impl);
            else
                ImGui::Text("RenderBuffer not found");
        }
        drawRefs<ramses::RenderPass>("Used by RenderPass", obj, [&](const ramses::RenderPass* ref) {
            return (ref->impl.getRenderTarget() == &obj.getRamsesObject());
        });
    }

    void SceneViewerGui::drawRenderBuffer(ramses::RenderBufferImpl& obj)
    {
        const auto& rb = m_scene.impl.getIScene().getRenderBuffer(obj.getRenderBufferHandle());
        ImGui::Text("Width:%u Height:%u", rb.width, rb.height);
        ImGui::Text("BufferType: %s", EnumToString(rb.type));
        ImGui::Text("BufferFormat: %s", EnumToString(rb.format));
        ImGui::Text("AccessMode: %s", EnumToString(rb.accessMode));
        ImGui::Text("SampleCount: %u", rb.sampleCount);
        drawRefs<ramses::RenderTarget>("Used by RenderTarget", obj, [&](const ramses::RenderTarget* ref) {
            const auto rtHandle   = ref->impl.getRenderTargetHandle();
            const uint32_t numBuffers = m_scene.impl.getIScene().getRenderTargetRenderBufferCount(rtHandle);
            for (uint32_t i = 0; i < numBuffers; ++i)
            {
                const auto rbHandle = m_scene.impl.getIScene().getRenderTargetRenderBuffer(rtHandle, i);
                if (rbHandle == obj.getRenderBufferHandle())
                    return true;
            }
            return false;
        });
        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const ramses_internal::TextureSampler& sampler = obj.getIScene().getTextureSampler(ref->impl.getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::RenderBuffer) && (sampler.contentHandle == obj.getRenderBufferHandle().asMemoryHandle());
        });
        drawRefs<ramses::BlitPass>("Used by BlitPass", obj, [&](const ramses::BlitPass* ref) {
            const auto& bp = m_scene.impl.getIScene().getBlitPass(ref->impl.getBlitPassHandle());
            return (bp.sourceRenderBuffer == obj.getRenderBufferHandle()) || (bp.destinationRenderBuffer == obj.getRenderBufferHandle());
        });
    }

    void SceneViewerGui::drawBlitPass(ramses::BlitPassImpl& obj)
    {
        const auto& bp = m_scene.impl.getIScene().getBlitPass(obj.getBlitPassHandle());
        int src[4] = {static_cast<int>(bp.sourceRegion.x), static_cast<int>(bp.sourceRegion.y), bp.sourceRegion.width, bp.sourceRegion.height};
        int dst[2] = {static_cast<int>(bp.destinationRegion.x), static_cast<int>(bp.destinationRegion.y)};
        bool isEnabled = bp.isEnabled;
        int renderOrder = obj.getRenderOrder();
        if (ImGui::Checkbox("Enabled", &isEnabled))
            obj.setEnabled(isEnabled);
        if (ImGui::DragInt("RenderOrder", &renderOrder))
            obj.setRenderOrder(renderOrder);
        if (ImGui::DragInt4("SourceRegion (x,y,w,h)", src))
            obj.setBlittingRegion(src[0], src[1], dst[0], dst[1], src[2], src[3]);
        if (ImGui::DragInt2("Destination (x,y)", dst))
            obj.setBlittingRegion(src[0], src[1], dst[0], dst[1], src[2], src[3]);
        ImGui::Text("src:");
        ImGui::SameLine();
        draw(obj.getSourceRenderBuffer().impl);
        ImGui::Text("dst:");
        ImGui::SameLine();
        draw(obj.getDestinationRenderBuffer().impl);
    }

    void SceneViewerGui::drawAppearance(ramses::AppearanceImpl& obj)
    {
        const ramses::Effect& effect = obj.getEffect();
        ramses_internal::ClientScene& iscene   = m_scene.impl.getIScene();
        const ramses_internal::RenderState& rs = m_scene.impl.getIScene().getRenderState(obj.getRenderStateHandle());

        if (ImGui::TreeNode("Blending"))
        {
            float rgba[4];
            obj.getBlendingColor(rgba[0], rgba[1], rgba[2], rgba[3]);
            if (ImGui::ColorEdit4("BlendingColor", rgba))
                obj.setBlendingColor(rgba[0], rgba[1], rgba[2], rgba[3]);

            int srcColor  = static_cast<int>(rs.blendFactorSrcColor);
            int destColor = static_cast<int>(rs.blendFactorDstColor);
            int srcAlpha  = static_cast<int>(rs.blendFactorSrcAlpha);
            int destAlpha = static_cast<int>(rs.blendFactorDstAlpha);
            auto setBlendFactors = [&]() {
                iscene.setRenderStateBlendFactors(obj.getRenderStateHandle(),
                                                  static_cast<EBlendFactor>(srcColor),
                                                  static_cast<EBlendFactor>(destColor),
                                                  static_cast<EBlendFactor>(srcAlpha),
                                                  static_cast<EBlendFactor>(destAlpha));
            };
            if (ImGui::Combo("srcColor", &srcColor, ramses_internal::BlendFactorNames, static_cast<int>(EBlendFactor::NUMBER_OF_ELEMENTS)))
                setBlendFactors();
            if (ImGui::Combo("destColor", &destColor, ramses_internal::BlendFactorNames, static_cast<int>(EBlendFactor::NUMBER_OF_ELEMENTS)))
                setBlendFactors();
            if (ImGui::Combo("srcAlpha", &srcAlpha, ramses_internal::BlendFactorNames, static_cast<int>(EBlendFactor::NUMBER_OF_ELEMENTS)))
                setBlendFactors();
            if (ImGui::Combo("destAlpha", &destAlpha, ramses_internal::BlendFactorNames, static_cast<int>(EBlendFactor::NUMBER_OF_ELEMENTS)))
                setBlendFactors();

            int blendingOperationColor = static_cast<int>(rs.blendOperationColor);
            int blendingOperationAlpha = static_cast<int>(rs.blendOperationAlpha);
            auto setBlendOperations = [&]() {
                iscene.setRenderStateBlendOperations(
                    obj.getRenderStateHandle(), static_cast<EBlendOperation>(blendingOperationColor), static_cast<EBlendOperation>(blendingOperationAlpha));
            };
            if (ImGui::Combo("colorOperation", &blendingOperationColor, ramses_internal::BlendOperationNames, static_cast<int>(EBlendOperation::NUMBER_OF_ELEMENTS)))
                setBlendOperations();
            if (ImGui::Combo("alphaOperation", &blendingOperationAlpha, ramses_internal::BlendOperationNames, static_cast<int>(EBlendOperation::NUMBER_OF_ELEMENTS)))
                setBlendOperations();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Depth"))
        {
            int depthFunc = static_cast<int>(rs.depthFunc);
            if (ImGui::Combo("depthFunc", &depthFunc, DepthFuncNames, static_cast<int>(EDepthFunc::NUMBER_OF_ELEMENTS)))
                iscene.setRenderStateDepthFunc(obj.getRenderStateHandle(), static_cast<EDepthFunc>(depthFunc));
            int depthWrite = static_cast<int>(rs.depthWrite);
            if (ImGui::Combo("depthWrite", &depthWrite, DepthWriteNames, static_cast<int>(EDepthWrite::NUMBER_OF_ELEMENTS)))
                iscene.setRenderStateDepthWrite(obj.getRenderStateHandle(), static_cast<EDepthWrite>(depthWrite));
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Scissor"))
        {
            int mode = static_cast<int>(rs.scissorTest);
            int xywh[4] = {
                rs.scissorRegion.x,
                rs.scissorRegion.y,
                rs.scissorRegion.width,
                rs.scissorRegion.height
            };
            auto setScissorTest = [&]() {
                iscene.setRenderStateScissorTest(obj.getRenderStateHandle(), static_cast<EScissorTest>(mode),
                    {static_cast<int16_t>(xywh[0]), static_cast<int16_t>(xywh[1]), static_cast<uint16_t>(xywh[2]), static_cast<uint16_t>(xywh[3])});
            };
            if (ImGui::Combo("scissorTest", &mode, ScissorTestNames, static_cast<int>(EScissorTest::NUMBER_OF_ELEMENTS)))
                setScissorTest();
            if (ImGui::DragInt4("Region", xywh))
                setScissorTest();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Stencil"))
        {
            int func = static_cast<int>(rs.stencilFunc);
            int refMask[2]  = {
                rs.stencilRefValue,
                rs.stencilMask
            };
            auto setStencilFunc = [&]() {
                iscene.setRenderStateStencilFunc(obj.getRenderStateHandle(), static_cast<EStencilFunc>(func), static_cast<uint8_t>(refMask[0]), static_cast<uint8_t>(refMask[1]));
            };
            if (ImGui::Combo("stencilFunc", &func, StencilFuncNames, static_cast<int>(EStencilFunc::NUMBER_OF_ELEMENTS)))
                setStencilFunc();
            if (ImGui::DragInt2("RefValue, Mask", refMask))
                setStencilFunc();

            int sfail = static_cast<int>(rs.stencilOpFail);
            int dpfail = static_cast<int>(rs.stencilOpDepthFail);
            int dppass = static_cast<int>(rs.stencilOpDepthPass);
            auto setStencilOps = [&]() {
                iscene.setRenderStateStencilOps(obj.getRenderStateHandle(), static_cast<EStencilOp>(sfail), static_cast<EStencilOp>(dpfail), static_cast<EStencilOp>(dppass));
            };
            if (ImGui::Combo("fail operation", &sfail, StencilOperationNames, static_cast<int>(EStencilOp::NUMBER_OF_ELEMENTS)))
                setStencilOps();
            if (ImGui::Combo("depth fail operation", &dpfail, StencilOperationNames, static_cast<int>(EStencilOp::NUMBER_OF_ELEMENTS)))
                setStencilOps();
            if (ImGui::Combo("depth pass operation", &dppass, StencilOperationNames, static_cast<int>(EStencilOp::NUMBER_OF_ELEMENTS)))
                setStencilOps();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("DrawMode"))
        {
            int culling = static_cast<int>(rs.cullMode);
            if (ImGui::Combo("Culling", &culling, CullModeNames, static_cast<int>(ECullMode::NUMBER_OF_ELEMENTS)))
                iscene.setRenderStateCullMode(obj.getRenderStateHandle(), static_cast<ECullMode>(culling));
            int drawMode = static_cast<int>(rs.drawMode);
            if (ImGui::Combo("DrawMode", &drawMode, DrawModeNames, static_cast<int>(EDrawMode::NUMBER_OF_ELEMENTS)))
                iscene.setRenderStateDrawMode(obj.getRenderStateHandle(), static_cast<EDrawMode>(drawMode));
            uint32_t colorFlags = rs.colorWriteMask;
            ImGui::Text("ColorWriteMask");
            auto setColorWriteMask = [&]() { iscene.setRenderStateColorWriteMask(obj.getRenderStateHandle(), static_cast<uint8_t>(colorFlags)); };
            if (ImGui::CheckboxFlags("Red", &colorFlags, EColorWriteFlag_Red))
                setColorWriteMask();
            if (ImGui::CheckboxFlags("Green", &colorFlags, EColorWriteFlag_Green))
                setColorWriteMask();
            if (ImGui::CheckboxFlags("Blue", &colorFlags, EColorWriteFlag_Blue))
                setColorWriteMask();
            if (ImGui::CheckboxFlags("Alpha", &colorFlags, EColorWriteFlag_Alpha))
                setColorWriteMask();
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Uniform input"))
        {
            for (uint32_t i = 0; i < effect.getUniformInputCount(); ++i)
            {
                ramses::UniformInput uniform;
                effect.getUniformInput(i, uniform);
                const ramses::DataObject* boundObj = obj.getBoundDataObject(uniform.impl);
                if (uniform.getSemantics() != ramses::EEffectUniformSemantic::Invalid)
                {
                    ImGui::BulletText("%s %s[%u] (%s)", shortName(uniform.impl.getDataType()), uniform.getName(), uniform.getElementCount(), EFixedSemanticsNames[static_cast<int>(uniform.impl.getSemantics())]);
                }
                else
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                    if (ImGui::TreeNode(uniform.getName(), "%s %s[%u]:", shortName(uniform.impl.getDataType()), uniform.getName(), uniform.getElementCount()))
                    {
                        if (boundObj != nullptr)
                            draw(boundObj->impl);
                        else
                            drawUniformValue(obj, uniform);
                        ImGui::TreePop();
                    }
                }
            }
            ImGui::TreePop();
        }
        draw(effect.impl);
        drawRefs<ramses::MeshNode>("Used by MeshNode", obj, [&](const ramses::MeshNode* ref) {
            return ref->impl.getAppearanceImpl() == &obj;
        });
    }

    void SceneViewerGui::drawUniformValue(ramses::AppearanceImpl& obj, ramses::UniformInput& uniform)
    {
        std::vector<ramses_internal::Vector4i> vec4i;
        std::vector<ramses_internal::Vector3i> vec3i;
        std::vector<ramses_internal::Vector2i> vec2i;
        std::vector<int32_t>                   vec1i;
        std::vector<ramses_internal::Vector4>  vec4;
        std::vector<ramses_internal::Vector3>  vec3;
        std::vector<ramses_internal::Vector2>  vec2;
        std::vector<float>                     vec1;
        const ramses::TextureSampler*          textureSampler;
        ramses::status_t                       status = ramses::StatusOK;

        switch (uniform.getDataType())
        {
        case ramses::EEffectInputDataType_Float:
            vec1.resize(uniform.getElementCount());
            status = obj.getInputValue(uniform.impl, uniform.getElementCount(), vec1.data());
            if (ramses::StatusOK == status)
            {
                for (auto it = vec1.begin(); it != vec1.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec1.begin());
                    if (ImGui::DragFloat(label.c_str(), &*it, 0.1f))
                        obj.setInputValue(uniform.impl, uniform.getElementCount(), vec1.data());
                }
            }
            break;
        case ramses::EEffectInputDataType_Vector2F:
            vec2.resize(uniform.getElementCount());
            status = obj.getInputValue(uniform.impl, uniform.getElementCount(), vec2.data());
            if (ramses::StatusOK == status)
            {
                for (auto it = vec2.begin(); it != vec2.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec2.begin());
                    if (ImGui::DragFloat2(label.c_str(), it->data, 0.1f))
                        obj.setInputValue(uniform.impl, uniform.getElementCount(), vec2.data());
                }
            }
            break;
        case ramses::EEffectInputDataType_Vector3F:
            vec3.resize(uniform.getElementCount());
            status = obj.getInputValue(uniform.impl, uniform.getElementCount(), vec3.data());
            if (ramses::StatusOK == status)
            {
                for (auto it = vec3.begin(); it != vec3.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec3.begin());
                    if (ImGui::DragFloat3(label.c_str(), it->data, 0.1f))
                        obj.setInputValue(uniform.impl, uniform.getElementCount(), vec3.data());
                }
            }
            break;
        case ramses::EEffectInputDataType_Vector4F:
            vec4.resize(uniform.getElementCount());
            status = obj.getInputValue(uniform.impl, uniform.getElementCount(), vec4.data());
            if (ramses::StatusOK == status)
            {
                for (auto it = vec4.begin(); it != vec4.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec4.begin());
                    if (ImGui::DragFloat4(label.c_str(), it->data, 0.1f))
                        obj.setInputValue(uniform.impl, uniform.getElementCount(), vec4.data());
                }
            }
            break;
        case ramses::EEffectInputDataType_Int32:
            vec1i.resize(uniform.getElementCount());
            status = obj.getInputValue(uniform.impl, uniform.getElementCount(), vec1i.data());
            if (ramses::StatusOK == status)
            {
                for (auto it = vec1i.begin(); it != vec1i.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec1i.begin());
                    if (ImGui::DragInt(label.c_str(), &*it, 0.1f))
                        obj.setInputValue(uniform.impl, uniform.getElementCount(), vec1i.data());
                }
            }
            break;
        case ramses::EEffectInputDataType_Vector2I:
            vec2i.resize(uniform.getElementCount());
            status = obj.getInputValue(uniform.impl, uniform.getElementCount(), vec2i.data());
            if (ramses::StatusOK == status)
            {
                for (auto it = vec2i.begin(); it != vec2i.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec2i.begin());
                    if (ImGui::DragInt2(label.c_str(), it->data, 0.1f))
                        obj.setInputValue(uniform.impl, uniform.getElementCount(), vec2i.data());
                }
            }
            break;
        case ramses::EEffectInputDataType_Vector3I:
            vec3i.resize(uniform.getElementCount());
            status = obj.getInputValue(uniform.impl, uniform.getElementCount(), vec3i.data());
            if (ramses::StatusOK == status)
            {
                for (auto it = vec3i.begin(); it != vec3i.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec3i.begin());
                    if (ImGui::DragInt3(label.c_str(), it->data, 0.1f))
                        obj.setInputValue(uniform.impl, uniform.getElementCount(), vec3i.data());
                }
            }
            break;
        case ramses::EEffectInputDataType_Vector4I:
            vec4i.resize(uniform.getElementCount());
            status = obj.getInputValue(uniform.impl, uniform.getElementCount(), vec4i.data());
            if (ramses::StatusOK == status)
            {
                for (auto it = vec4i.begin(); it != vec4i.end(); ++it)
                {
                    const std::string label = fmt::format("{}[{}]", uniform.getName(), it - vec4i.begin());
                    if (ImGui::DragInt4(label.c_str(), it->data, 0.1f))
                        obj.setInputValue(uniform.impl, uniform.getElementCount(), vec4i.data());
                }
            }
            break;
        case ramses::EEffectInputDataType_TextureSampler2D:
        case ramses::EEffectInputDataType_TextureSampler2DMS:
        case ramses::EEffectInputDataType_TextureSampler3D:
        case ramses::EEffectInputDataType_TextureSamplerCube:
            status = obj.getInputTexture(uniform.impl, textureSampler);
            if (ramses::StatusOK == status)
            {
                draw(textureSampler->impl);
            }
            break;
        case ramses::EEffectInputDataType_Invalid:
        case ramses::EEffectInputDataType_UInt16:
        case ramses::EEffectInputDataType_UInt32:
        case ramses::EEffectInputDataType_Matrix22F:
        case ramses::EEffectInputDataType_Matrix33F:
        case ramses::EEffectInputDataType_Matrix44F:
            ImGui::Text("tbd. %s", EnumToString(uniform.impl.getDataType()));
            break;
        }

        if (status != ramses::StatusOK)
            ImGui::Text("Error occurred: %s", obj.getStatusMessage(status));
    }

    void SceneViewerGui::drawGeometryBinding(ramses::GeometryBindingImpl& obj)
    {
        const ramses::Effect& effect = obj.getEffect();
        if (ImGui::TreeNode("Attribute input"))
        {
            auto& iScene = m_scene.impl.getIScene();
            const ramses_internal::DataLayout& layout = iScene.getDataLayout(obj.getAttributeDataLayout());
            const uint32_t dataLayoutFieldCount = layout.getFieldCount();
            for (uint32_t i = 0U; i < dataLayoutFieldCount; ++i)
            {
                if (0 == i)
                {
                    ImGui::Text("Indices:");
                }
                else
                {
                    ramses::AttributeInput attribute;
                    effect.getAttributeInput(i - 1, attribute);
                    ImGui::Text("%s:", attribute.getName());
                }
                const ramses_internal::DataFieldHandle fieldIndex(i);
                const ramses_internal::ResourceField&  dataResource = iScene.getDataResource(obj.getAttributeDataInstance(), fieldIndex);

                if (dataResource.dataBuffer.isValid())
                {
                    auto buf = findDataBuffer(dataResource.dataBuffer);
                    assert(buf != nullptr);
                    draw(buf->impl);
                }
                else if (dataResource.hash.isValid())
                {
                    const ramses::Resource* resource = m_scene.impl.scanForResourceWithHash(dataResource.hash);
                    if (resource != nullptr)
                        draw(resource->impl);
                    else
                        ImGui::Text("Resource missing");
                }
            }
            ImGui::TreePop();
        }

        draw(effect.impl);
        drawRefs<ramses::MeshNode>("Used by MeshNode", obj, [&](const ramses::MeshNode* ref) {
            return ref->impl.getGeometryBindingImpl() == &obj;
        });
    }

    void SceneViewerGui::drawTexture2D(ramses::Texture2DImpl& obj)
    {
        drawResource(obj);
        ImGui::Text("Width:%u Height:%u Format:%s", obj.getWidth(), obj.getHeight(), ramses::getTextureFormatString(obj.getTextureFormat()));
        const auto& swizzle = obj.getTextureSwizzle();
        ImGui::Text("Swizzle: r:%s g:%s b:%s a:%s",
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelRed)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelGreen)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelBlue)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelAlpha)));
        const auto* slot = findDataSlot(obj.getLowlevelResourceHash());
        if (slot)
            ImGui::Text("DataSlot: %u %s", slot->id.getValue(), EnumToString(slot->type));
        auto resource = m_scene.getRamsesClient().impl.getResource(obj.getLowlevelResourceHash());
        if (resource)
        {
            const ramses_internal::TextureResource* textureResource = resource->convertTo<ramses_internal::TextureResource>();
            if (textureResource != nullptr)
            {
                if (ImGui::Button("Save png"))
                {
                    m_lastErrorMessage = saveTexture2D(obj);
                }
                imgui::PreviewImage(m_imageCache.get(textureResource), ImVec2(128, 128));
            }
        }

        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl.getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::ClientTexture) && (sampler.textureResource == obj.getLowlevelResourceHash());
        });
    }

    void SceneViewerGui::drawTexture3D(ramses::Texture3DImpl& obj)
    {
        drawResource(obj);
        ImGui::Text("Width:%u Height:%u Format:%s", obj.getWidth(), obj.getHeight(), ramses::getTextureFormatString(obj.getTextureFormat()));
        ImGui::Text("Depth:%u", obj.getDepth());

        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl.getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::ClientTexture) && (sampler.textureResource == obj.getLowlevelResourceHash());
        });
    }

    void SceneViewerGui::drawTexture2DBuffer(ramses::Texture2DBufferImpl& obj)
    {
        const auto& tb = m_scene.impl.getIScene().getTextureBuffer(obj.getTextureBufferHandle());
        ImGui::Text("Format:%s", EnumToString(tb.textureFormat));
        for (auto it = tb.mipMaps.begin(); it != tb.mipMaps.end(); ++it)
        {
            ImGui::Text("MipMap:");
            ImGui::BulletText("width:%u height:%u", it->width, it->height);
            ImGui::BulletText("area: x:%d y:%d w:%d h:%d", it->usedRegion.x, it->usedRegion.y, it->usedRegion.width, it->usedRegion.height);
            ImGui::BulletText("size (kB): %lu", it->data.size() / 1024);
        }
        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl.getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::TextureBuffer) && (sampler.contentHandle == obj.getTextureBufferHandle());
        });
    }

    void SceneViewerGui::drawTextureCube(ramses::TextureCubeImpl& obj)
    {
        drawResource(obj);
        ImGui::Text("Size:%u Format:%s", obj.getSize(), ramses::getTextureFormatString(obj.getTextureFormat()));
        const auto& swizzle = obj.getTextureSwizzle();
        ImGui::Text("Swizzle: r:%s g:%s b:%s a:%s",
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelRed)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelGreen)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelBlue)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelAlpha)));

        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl.getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::ClientTexture) && (sampler.textureResource == obj.getLowlevelResourceHash());
        });
    }

    void SceneViewerGui::drawStreamTexture(ramses::StreamTextureImpl& obj)
    {
        ImGui::Text("StreamSourceId: %u", obj.getStreamSource().getValue());
        bool forceFallback = obj.getForceFallbackImage();
        if (ImGui::Checkbox("Force fallback image", &forceFallback))
            obj.forceFallbackImage(forceFallback);

        const ramses::Texture2D* fallbackTexture = findTexture2D(obj.getFallbackTextureHash());
        ImGui::Text("FallbackTexture:");
        ImGui::SameLine();
        if (fallbackTexture != nullptr)
            draw(fallbackTexture->impl);
        else
            ImGui::Text("missing");

        drawRefs<ramses::TextureSampler>("Used by TextureSampler", obj, [&](const ramses::TextureSampler* ref) {
            const auto& sampler = obj.getIScene().getTextureSampler(ref->impl.getTextureSamplerHandle());
            return (sampler.contentType == TextureSampler::ContentType::StreamTexture) && (sampler.contentHandle == obj.getHandle());
        });
    }

    void SceneViewerGui::drawTextureSampler(ramses::TextureSamplerImpl& obj)
    {
        const auto* slot = findDataSlot(obj.getTextureSamplerHandle());
        if (slot)
            ImGui::Text("DataSlot: %u %s", slot->id.getValue(), EnumToString(slot->type));
        ImGui::Text("Wrap:");
        ImGui::BulletText("u:%s", ramses::getTextureAddressModeString(obj.getWrapUMode()));
        ImGui::BulletText("v:%s", ramses::getTextureAddressModeString(obj.getWrapVMode()));
        ImGui::BulletText("r:%s", ramses::getTextureAddressModeString(obj.getWrapRMode()));
        ImGui::Text("Sampling:");
        ImGui::BulletText("min:%s", ramses::getTextureSamplingMethodString(obj.getMinSamplingMethod()));
        ImGui::BulletText("mag:%s", ramses::getTextureSamplingMethodString(obj.getMagSamplingMethod()));
        ImGui::Text("AnisotropyLevel: %u", obj.getAnisotropyLevel());

        ramses::Resource* resource = nullptr;
        const ramses_internal::TextureSampler& sampler = obj.getIScene().getTextureSampler(obj.getTextureSamplerHandle());
        const ramses::RenderBuffer* rb = nullptr;
        const ramses::StreamTexture* streamTexture = nullptr;
        const ramses::Texture2DBuffer* textureBuffer = nullptr;
        switch (sampler.contentType)
        {
        case TextureSampler::ContentType::ClientTexture:
            resource = obj.getSceneImpl().scanForResourceWithHash(sampler.textureResource);
            if (resource != nullptr)
                draw(resource->impl);
            else
                ImGui::Text("Resource missing");
            break;
        case TextureSampler::ContentType::RenderBuffer:
        case TextureSampler::ContentType::RenderBufferMS:
            rb = findRenderBuffer(RenderBufferHandle(sampler.contentHandle));
            if (rb != nullptr)
                draw(rb->impl);
            else
                ImGui::Text("RenderBuffer missing");
            break;
        case TextureSampler::ContentType::TextureBuffer:
            textureBuffer = findTextureBuffer(TextureBufferHandle(sampler.contentHandle));
            if (textureBuffer != nullptr)
                draw(textureBuffer->impl);
            else
                ImGui::Text("TextureBuffer missing");
            break;
        case TextureSampler::ContentType::StreamTexture:
            streamTexture = findStreamTexture(StreamTextureHandle(sampler.contentHandle));
            if (streamTexture != nullptr)
                draw(streamTexture->impl);
            else
                ImGui::Text("StreamTexture missing");
            break;
        case TextureSampler::ContentType::OffscreenBuffer:
        case TextureSampler::ContentType::StreamBuffer:
        case TextureSampler::ContentType::None:
            ImGui::Text("Type: %s (tbd.)", ramses::RamsesObjectTypeUtils::GetRamsesObjectTypeName(obj.getTextureType()));
        }

        drawRefs<ramses::Appearance>("Used by Appearance", obj, [&](const ramses::Appearance* ref) {
            const ramses::Effect& effect = ref->getEffect();
            for (uint32_t i = 0; i < effect.getUniformInputCount(); ++i)
            {
                ramses::UniformInput uniform;
                effect.getUniformInput(i, uniform);
                const ramses::TextureSampler* textureSampler;
                switch (uniform.impl.getUniformInputDataType())
                {
                    case ramses::EEffectInputDataType_TextureSampler2D:
                    case ramses::EEffectInputDataType_TextureSampler2DMS:
                    case ramses::EEffectInputDataType_TextureSampler3D:
                    case ramses::EEffectInputDataType_TextureSamplerCube:
                        if (0 == ref->impl.getInputTexture(uniform.impl, textureSampler))
                            if (textureSampler == &obj.getRamsesObject())
                                return true;
                        break;
                    default:
                        break;
                }
            }
            return false;
        });
    }

    void SceneViewerGui::drawArrayResource(ramses::ArrayResourceImpl& obj)
    {
        drawResource(obj);
        ImGui::Text("%s[%u]", EnumToString(obj.getElementType()), obj.getElementCount());
        drawRefs<ramses::GeometryBinding>("Used by GeometryBinding", obj, [&](const ramses::GeometryBinding* ref) {
            auto&          iScene     = m_scene.impl.getIScene();
            const auto&    layout     = iScene.getDataLayout(ref->impl.getAttributeDataLayout());
            const uint32_t fieldCount = layout.getFieldCount();
            for (uint32_t i = 0U; i < fieldCount; ++i)
            {
                const ramses_internal::DataFieldHandle fieldIndex(i);
                const ramses_internal::ResourceField&  dataResource = iScene.getDataResource(ref->impl.getAttributeDataInstance(), fieldIndex);
                if (dataResource.hash == obj.getLowlevelResourceHash())
                    return true;
            }
            return false;
        });
    }

    void SceneViewerGui::drawArrayBuffer(ramses::ArrayBufferImpl& obj)
    {
        ImGui::Text("%s[%u]", EnumToString(obj.getDataType()), obj.getElementCount());
        drawRefs<ramses::GeometryBinding>("Used by GeometryBinding", obj, [&](const ramses::GeometryBinding* ref) {
            auto&          iScene     = m_scene.impl.getIScene();
            const auto&    layout     = iScene.getDataLayout(ref->impl.getAttributeDataLayout());
            const uint32_t fieldCount = layout.getFieldCount();
            for (uint32_t i = 0U; i < fieldCount; ++i)
            {
                const ramses_internal::DataFieldHandle fieldIndex(i);
                const ramses_internal::ResourceField&  dataResource = iScene.getDataResource(ref->impl.getAttributeDataInstance(), fieldIndex);
                if (dataResource.dataBuffer == obj.getDataBufferHandle())
                    return true;
            }
            return false;
        });
    }

    void SceneViewerGui::drawDataObject(ramses::DataObjectImpl& obj)
    {
        float valueF;
        Int32 valueI;
        ramses_internal::Vector2 value2F;
        ramses_internal::Vector3 value3F;
        ramses_internal::Vector4 value4F;
        ramses_internal::Vector2i value2I;
        ramses_internal::Vector3i value3I;
        ramses_internal::Vector4i value4I;
        const auto* slot = findDataSlot(obj.getDataReference());
        if (slot)
            ImGui::Text("DataSlot: %u %s", slot->id.getValue(), EnumToString(slot->type));
        switch (obj.getDataType())
        {
        case EDataType::Float:
            obj.getValue(valueF);
            if (ImGui::DragFloat("Value", &valueF, 0.1f))
                obj.setValue(valueF);
            break;
        case EDataType::Vector2F:
            obj.getValue(value2F);
            if (ImGui::DragFloat2("Value", value2F.data, 0.1f))
                obj.setValue(value2F);
            break;
        case EDataType::Vector3F:
            obj.getValue(value3F);
            if (ImGui::DragFloat3("Value", value3F.data, 0.1f))
                obj.setValue(value3F);
            break;
        case EDataType::Vector4F:
            obj.getValue(value4F);
            if (ImGui::DragFloat4("Value", value4F.data, 0.1f))
                obj.setValue(value4F);
            break;
        case EDataType::Int32:
            obj.getValue(valueI);
            if (ImGui::DragInt("Value", &valueI))
                obj.setValue(valueI);
            break;
        case EDataType::Vector2I:
            obj.getValue(value2I);
            if (ImGui::DragInt2("Value", value2I.data))
                obj.setValue(value2I);
            break;
        case EDataType::Vector3I:
            obj.getValue(value3I);
            if (ImGui::DragInt3("Value", value3I.data))
                obj.setValue(value3I);
            break;
        case EDataType::Vector4I:
            obj.getValue(value4I);
            if (ImGui::DragInt4("Value", value4I.data))
                obj.setValue(value4I);
            break;
        default:
            ImGui::Text("tbd. %s", EnumToString(obj.getDataType()));
        }

        drawRefs<ramses::Appearance>("Used by Appearance", obj, [&](const ramses::Appearance* ref) {
            const ramses::Effect& effect = ref->getEffect();
            for (uint32_t i = 0; i < effect.getUniformInputCount(); ++i)
            {
                ramses::UniformInput uniform;
                effect.getUniformInput(i, uniform);
                const ramses::DataObject* boundObj = ref->impl.getBoundDataObject(uniform.impl);
                if (boundObj == &obj.getRamsesObject())
                    return true;
            }
            return false;
        });

        drawRefs<ramses::Camera>("Used by Camera", obj, [&](const ramses::Camera* ref) {
            auto fp = findDataObject(ref->impl.getFrustrumPlanesHandle());
            auto nf = findDataObject(ref->impl.getFrustrumNearFarPlanesHandle());
            auto pos = findDataObject(ref->impl.getViewportOffsetHandle());
            auto size = findDataObject(ref->impl.getViewportSizeHandle());
            const auto objPtr = &obj.getRamsesObject();
            return (objPtr == fp || objPtr == nf || objPtr == pos || objPtr == size);
        });
    }

    void SceneViewerGui::drawSceneReference(ramses::SceneReferenceImpl& obj)
    {
        ImGui::Text("ReferencedScene: %lu", obj.getReferencedSceneId().getValue());
        ImGui::Text("RequestedState: %s", EnumToString(ramses::SceneReferenceImpl::GetInternalSceneReferenceState(obj.getRequestedState())));
        ImGui::Text("ReportedState: %s", EnumToString(ramses::SceneReferenceImpl::GetInternalSceneReferenceState(obj.getReportedState())));
    }

    void SceneViewerGui::drawDataSlot(const ramses_internal::DataSlot& obj)
    {
        if (ImGui::TreeNode(&obj, "Slot: %3u %s", obj.id.getValue(), EnumToString(obj.type)))
        {
            if (obj.attachedTexture.isValid())
            {
                auto tex = findTexture2D(obj.attachedTexture);
                if (tex != nullptr)
                    draw(tex->impl);
                else
                    ImGui::Text("Texture not found: %" PRIx64 ":%" PRIx64, obj.attachedTexture.highPart, obj.attachedTexture.lowPart);
            }
            else if (obj.attachedNode.isValid())
            {
                auto node = findNode(obj.attachedNode);
                if (node != nullptr)
                    draw(node->impl);
                else
                    ImGui::Text("Node not found: %u", obj.attachedNode.asMemoryHandle());
            }
            else if (obj.attachedDataReference.isValid())
            {
                auto ref = findDataObject(obj.attachedDataReference);
                if (ref != nullptr)
                    draw(ref->impl);
                else
                    ImGui::Text("DataReference not found: %u", obj.attachedDataReference.asMemoryHandle());
            }
            else if (obj.attachedTextureSampler.isValid())
            {
                auto ref = findTextureSampler(obj.attachedTextureSampler);
                if (ref != nullptr)
                    draw(ref->impl);
                else
                    ImGui::Text("TextureSampler not found: %u", obj.attachedTextureSampler.asMemoryHandle());
            }
            else
            {
                ImGui::Text("<not connected>");
            }
            ImGui::TreePop();
        }
    }

    void SceneViewerGui::drawSceneObjects()
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Scene objects"))
        {
            ImGui::Text("Name filter:");
            if (m_filter.Draw() || m_sceneObjects.empty())
            {
                const auto& reg = m_scene.impl.getObjectRegistry();
                for (uint32_t i = 0u; i < ramses::ERamsesObjectType_NUMBER_OF_TYPES; ++i)
                {
                    const auto type = static_cast<ramses::ERamsesObjectType>(i);

                    if (ramses::RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ramses::ERamsesObjectType_SceneObject)
                        && !ramses::RamsesObjectTypeUtils::IsTypeMatchingBaseType(type, ramses::ERamsesObjectType_AnimationObject)
                        && ramses::RamsesObjectTypeUtils::IsConcreteType(type))
                    {
                        auto& objects = m_sceneObjects[type];
                        const auto numberOfObjects = reg.getNumberOfObjects(type);
                        objects.reserve(numberOfObjects);
                        objects.clear();
                        if (numberOfObjects > 0u)
                        {
                            ramses::RamsesObjectRegistryIterator iter(reg, ramses::ERamsesObjectType(i));
                            while (const auto* obj = iter.getNext())
                            {
                                if (m_filter.PassFilter(obj->getName()))
                                    objects.push_back(obj);
                            }
                        }
                    }
                }
            }

            for (const auto& it : m_sceneObjects)
            {
                if (!it.second.empty())
                {
                    const char* typeName = ramses::RamsesObjectTypeUtils::GetRamsesObjectTypeName(it.first);
                    if (ImGui::TreeNode(typeName, "%s (%zu)", typeName, it.second.size()))
                    {
                        for (auto* obj : it.second)
                        {
                            draw(obj->impl);
                        }
                        ImGui::TreePop();
                    }
                }
            }

            const auto& slots = m_scene.impl.getIScene().getDataSlots();
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
            ramses::SceneObjectIterator it(m_scene, ramses::ERamsesObjectType_Node);
            while (const ramses::Node* node = static_cast<const ramses::Node*>(it.getNext()))
            {
                if (node->getParent() == nullptr)
                    draw(node->impl);
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
            ImGui::EndPopup();
        }

        if (isOpen)
        {
            updateResourceInfo();
            ImGui::Text("%lu resources", m_resourceInfo.objects.size());
            ImGui::Text("Size: %u kB (compressed: %u kB)", m_resourceInfo.decompressedSize / 1024U, m_resourceInfo.compressedSize / 1024U);
            ImGui::Text("Not loaded: %u", m_resourceInfo.unavailable);
            ImGui::Separator();
            const auto displayedResources = std::min(static_cast<int>(m_resourceInfo.objects.size()), m_resourceInfo.displayLimit);
            ImGui::Text("Size of %d biggest resources: %u kB", displayedResources, m_resourceInfo.displayedSize / 1024U);
            if (ImGui::InputInt("Display limit", &m_resourceInfo.displayLimit))
            {
                m_resourceInfo.displayedSize = 0U;
            }
            ImGui::Text("Resources sorted by size (decending):");
            int count = 0;
            const bool updateDisplayedSize = (m_resourceInfo.displayedSize == 0U);
            for (auto it : m_resourceInfo.objects)
            {
                ++count;
                if (count > m_resourceInfo.displayLimit)
                {
                    break;
                }
                if (updateDisplayedSize)
                {
                    auto hlResource = static_cast<ramses::Resource*>(it);
                    auto resource = m_scene.getRamsesClient().impl.getResource(hlResource->impl.getLowlevelResourceHash());
                    if (resource && m_usedObjects.contains(&hlResource->impl))
                    {
                        // don't count duplicates
                        m_resourceInfo.displayedSize += resource->getDecompressedDataSize();
                    }
                }
                draw(it->impl);
            }
        }
    }

    void SceneViewerGui::updateResourceInfo()
    {
        if (m_resourceInfo.objects.empty())
        {
            const auto& reg = m_scene.impl.getObjectRegistry();
            reg.getObjectsOfType(m_resourceInfo.objects, ramses::ERamsesObjectType_Resource);

            auto compareSize = [&](ramses::RamsesObject* a, ramses::RamsesObject* b) {
                ManagedResource resourceA = m_scene.getRamsesClient().impl.getResource(static_cast<ramses::Resource*>(a)->impl.getLowlevelResourceHash());
                ManagedResource resourceB = m_scene.getRamsesClient().impl.getResource(static_cast<ramses::Resource*>(b)->impl.getLowlevelResourceHash());
                const uint32_t  sizeA     = resourceA ? resourceA->getDecompressedDataSize() : 0u;
                const uint32_t  sizeB     = resourceB ? resourceB->getDecompressedDataSize() : 0u;
                return sizeA > sizeB;
            };
            std::sort(m_resourceInfo.objects.begin(), m_resourceInfo.objects.end(), compareSize);

            for (auto it : m_resourceInfo.objects)
            {
                auto hlResource = static_cast<ramses::Resource*>(it);
                auto resource   = m_scene.getRamsesClient().impl.getResource(hlResource->impl.getLowlevelResourceHash());
                m_resourceInfo.hashLookup.insert({hlResource->impl.getLowlevelResourceHash(), hlResource});
                if (resource)
                {
                    if (m_usedObjects.contains(&hlResource->impl))
                    {
                        // don't count duplicates
                        m_resourceInfo.compressedSize += resource->getCompressedDataSize();
                        m_resourceInfo.decompressedSize += resource->getDecompressedDataSize();
                    }
                }
                else
                    ++m_resourceInfo.unavailable;
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
                const auto& reg = m_scene.impl.getObjectRegistry();
                reg.getObjectsOfType(m_renderInfo.renderPassVector, ramses::ERamsesObjectType_RenderPass);
                std::sort(m_renderInfo.renderPassVector.begin(), m_renderInfo.renderPassVector.end(), [](const auto* a, const auto* b) {
                    return static_cast<const ramses::RenderPass*>(a)->getRenderOrder() < static_cast<const ramses::RenderPass*>(b)->getRenderOrder();
                });
            }
            for (auto pObj : m_renderInfo.renderPassVector)
            {
                ramses::RenderPassImpl& renderPass = static_cast<ramses::RenderPassImpl&>(pObj->impl);
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
        if (m_hasSceneErrors)
        {
            if (ImGui::CollapsingHeader("Objects with warnings/errors"))
            {
                ImGui::Text("Hover mouse for details");
                if (m_objectsWithErrors.empty())
                {
                    const auto&                reg = m_scene.impl.getObjectRegistry();
                    ramses::RamsesObjectVector allObjects;
                    reg.getObjectsOfType(allObjects, ramses::ERamsesObjectType_RamsesObject);
                    for (auto* obj : allObjects)
                    {
                        const char* report = obj->getValidationReport(ramses::EValidationSeverity_Warning);
                        if (report && report[0] != 0)
                        {
                            m_objectsWithErrors.push_back(obj);
                        }
                    }
                }
                for (auto* obj : m_objectsWithErrors)
                {
                    draw(obj->impl);
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
                File file = File(String(m_filename));
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
            // refresh resource information in next interation
            m_nodeVisibilityChanged = false;
            m_resourceInfo = {};
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
            ramses::RamsesObjectRegistryIterator iter(m_scene.impl.getObjectRegistry(), ramses::ERamsesObjectType_Texture2D);
            ImGui::LogToClipboard();
            ImGui::LogText("Id, Name, Type, Hash, Loaded, Size, CompressedSize, Size, Format, Swizzle\n");
            while (const auto* obj = iter.getNext<ramses::Texture2D>())
            {
                logTexture2D(obj->impl);
            }
            ImGui::LogFinish();
        }
    }

    void SceneViewerGui::drawMenuItemStorePng()
    {
        if (ImGui::MenuItem("Export all 2D textures to png"))
        {
            auto storeAllTextures = [&](ramses::RamsesObjectVector objects) {
                std::vector<std::string> errorList;
                for (auto it = objects.begin(); it != objects.end() && !m_progress.canceled; ++it)
                {
                    const ramses::Texture2D* obj = static_cast<ramses::Texture2D*>(*it);
                    ++m_progress.current;
                    const auto error = saveTexture2D(obj->impl);
                    if (!error.empty())
                    {
                        errorList.push_back(error);
                    }
                }
                return errorList;
            };

            ramses::RamsesObjectVector objects;
            m_scene.impl.getObjectRegistry().getObjectsOfType(objects, ramses::ERamsesObjectType_Texture2D);
            m_progress.stop();
            ProgressMonitor::FutureList futures;
            const size_t tasks     = objects.size() > 16u ? 4u : 1u;
            const auto   chunkSize = objects.size() / tasks;
            for (size_t i = 0; i < tasks; ++i)
            {
                const auto begin = objects.begin() + i * chunkSize;
                if (i + 1 == tasks)
                {
                    futures.push_back(std::async(std::launch::async, storeAllTextures, ramses::RamsesObjectVector(begin, objects.end())));
                }
                else
                {
                    futures.push_back(std::async(std::launch::async, storeAllTextures, ramses::RamsesObjectVector(begin, begin + chunkSize)));
                }
            }
            m_progress.start(std::move(futures), static_cast<uint32_t>(objects.size()), "Saving Texture2D to png");
        }
    }

    void SceneViewerGui::saveSceneToFile()
    {
        const auto status = m_scene.saveToFile(m_filename.c_str(), m_compressFile);
        if (status != ramses::StatusOK)
        {
            m_lastErrorMessage = m_scene.getStatusMessage(status);
        }
    }

    std::string SceneViewerGui::saveTexture2D(const ramses::Texture2DImpl& obj) const
    {
        auto resource = m_scene.getRamsesClient().impl.getResource(obj.getLowlevelResourceHash());
        std::string errorMsg;
        if (resource)
        {
            const ramses_internal::TextureResource* textureResource = resource->convertTo<ramses_internal::TextureResource>();
            const auto filename = fmt::format("{:04}_{}.png", obj.getObjectRegistryHandle().asMemoryHandle(), obj.getName());
            errorMsg = imgui::SaveTextureToPng(textureResource, filename);
        }
        return errorMsg;
    }

    void SceneViewerGui::setVisibility(ramses::NodeImpl& node, ramses::EVisibilityMode visibility)
    {
        node.setVisibility(visibility);
        // can't clear the resource cache immediately, because it might be currently iterated
        m_nodeVisibilityChanged = true;
    }

    void SceneViewerGui::logRamsesObject(ramses::RamsesObjectImpl& obj)
    {
        ImGui::LogText(R"(%u,"%s","%s",)", obj.getObjectRegistryHandle().asMemoryHandle(), obj.getName().c_str(), shortName(obj.getType()));
    }

    void SceneViewerGui::logResource(ramses::ResourceImpl& obj)
    {
        logRamsesObject(obj);
        const auto hash = obj.getLowlevelResourceHash();
        auto resource = m_scene.getRamsesClient().impl.getResource(hash);
        ImGui::LogText("%" PRIX64 ":%" PRIX64 ",", hash.highPart, hash.lowPart);
        if (resource)
        {
            ImGui::LogText("true, %u, %u,", resource->getDecompressedDataSize(), resource->getCompressedDataSize());
        }
        else
        {
            ImGui::LogText("false, 0, 0,");
        }
    }

    void SceneViewerGui::logTexture2D(ramses::Texture2DImpl& obj)
    {
        logResource(obj);
        ImGui::LogText("w:%u h:%u,%s, ", obj.getWidth(), obj.getHeight(), ramses::getTextureFormatString(obj.getTextureFormat()));
        const auto& swizzle = obj.getTextureSwizzle();
        ImGui::LogText("r:%s g:%s b:%s a:%s, ",
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelRed)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelGreen)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelBlue)),
            EnumToString(ramses::TextureUtils::GetTextureChannelColorInternal(swizzle.channelAlpha)));
        const auto* slot = findDataSlot(obj.getLowlevelResourceHash());
        if (slot)
            ImGui::LogText("DataSlot: %u %s,", slot->id.getValue(), EnumToString(slot->type));
        ImGui::LogText("\n");
    }
}
