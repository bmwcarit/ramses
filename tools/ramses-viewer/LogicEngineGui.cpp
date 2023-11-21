//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LogicEngineGui.h"
#include "LogicViewer.h"
#include "ViewerGuiApp.h"
#include "ViewerSettings.h"
#include "ramses/client/Scene.h"
#include "ramses/client/logic/LogicEngine.h"
#include "internal/logic/StdFilesystemWrapper.h"
#include "internal/PlatformAbstraction/FmtBase.h"
#include "glm/gtc/type_ptr.hpp"

namespace ramses::internal
{
    namespace
    {
        const char* EnumToString(ramses::ERotationType t)
        {
            switch (t)
            {
            case ramses::ERotationType::Euler_XYZ:
                return "Euler_XYZ";
            case ramses::ERotationType::Euler_XZY:
                return "Euler_XZY";
            case ramses::ERotationType::Euler_YXZ:
                return "Euler_YXZ";
            case ramses::ERotationType::Euler_YZX:
                return "Euler_YZX";
            case ramses::ERotationType::Euler_ZXY:
                return "Euler_ZXY";
            case ramses::ERotationType::Euler_ZYX:
                return "Euler_ZYX";
            case ramses::ERotationType::Euler_XYX:
                return "Euler_XYX";
            case ramses::ERotationType::Euler_XZX:
                return "Euler_XZX";
            case ramses::ERotationType::Euler_YXY:
                return "Euler_YXY";
            case ramses::ERotationType::Euler_YZY:
                return "Euler_YZY";
            case ramses::ERotationType::Euler_ZXZ:
                return "Euler_ZXZ";
            case ramses::ERotationType::Euler_ZYZ:
                return "Euler_ZYZ";
            case ramses::ERotationType::Quaternion:
                return "Quaternion";
            }
            return "";
        }

        const char* EnumToString(EInterpolationType t)
        {
            switch (t)
            {
            case EInterpolationType::Step:
                return "Step";
            case EInterpolationType::Linear:
                return "Linear";
            case EInterpolationType::Cubic:
                return "Cubic";
            case EInterpolationType::Linear_Quaternions:
                return "Linear_Quaternions";
            case EInterpolationType::Cubic_Quaternions:
                return "Cubic_Quaternions";
            }
            return "";
        }

        bool TreeNode(const void* ptr_id, const std::string_view& text)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
            return ImGui::TreeNode(ptr_id, "%s", text.data());
        }

        bool TreeNode(const void* ptr_id, const std::string& text)
        {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg) 3rd party interface
            return ImGui::TreeNode(ptr_id, "%s", text.c_str());
        }
    }

    LogicEngineGui::LogicEngineGui(const ViewerSettings& settings)
        : m_settings(settings)
    {
    }

    bool LogicEngineGui::DrawTreeNode(ramses::LogicObject* obj)
    {
        return TreeNode(obj, fmt::format("[{}]: {}", obj->getSceneObjectId().getValue(), obj->getName()));
    }

    void LogicEngineGui::draw(ramses::LuaScript* obj)
    {
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::LuaInterface* obj)
    {
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::AnimationNode* obj)
    {
        ImGui::TextUnformatted(fmt::format("Duration: {}", *obj->getOutputs()->getChild("duration")->get<float>()).c_str());
        if (ImGui::TreeNode("Channels"))
        {
            for (auto& ch : obj->getChannels())
            {
                if (TreeNode(&ch, ch.name))
                {
                    ImGui::TextUnformatted(fmt::format("InterpolationType: {}", EnumToString(ch.interpolationType)).c_str());
                    DrawDataArray(ch.keyframes, "Keyframes");
                    DrawDataArray(ch.tangentsIn, "TangentsIn");
                    DrawDataArray(ch.tangentsOut, "TangentsOut");
                    DrawDataArray(ch.timeStamps, "TimeStamps");
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::TimerNode* obj)
    {
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::NodeBinding* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses Node: {}", obj->getRamsesNode().getName()).c_str());
        ImGui::TextUnformatted(fmt::format("Rotation Mode: {}", EnumToString(obj->getRotationType())).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::CameraBinding* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses Camera: {}", obj->getRamsesCamera().getName()).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::RenderPassBinding* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses RenderPass: {}", obj->getRamsesRenderPass().getName()).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::RenderGroupBinding* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses RenderGroup: {}", obj->getRamsesRenderGroup().getName()).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::MeshNodeBinding* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses MeshNode: {}", obj->getRamsesMeshNode().getName()).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::AnchorPoint* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses Node: {}", obj->getRamsesNode().getName()).c_str());
        ImGui::TextUnformatted(fmt::format("Ramses Camera: {}", obj->getRamsesCamera().getName()).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::SkinBinding* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses Appearance: {}", obj->getAppearanceBinding().getRamsesAppearance().getName()).c_str());
        ImGui::TextUnformatted(fmt::format("Ramses Uniform input: {}", obj->getAppearanceUniformInput().getName()).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::AppearanceBinding* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses Appearance: {}", obj->getRamsesAppearance().getName()).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::draw(ramses::RenderBufferBinding* obj)
    {
        ImGui::TextUnformatted(fmt::format("Ramses RenderBuffer: {}", obj->getRenderBuffer().getName()).c_str());
        drawNode(obj);
    }

    void LogicEngineGui::drawNodeContextMenu(ramses::LogicNode* obj, const std::string_view& ns)
    {
        if (ImGui::BeginPopupContextItem(obj->getName().data()))
        {
            if (ImGui::MenuItem(fmt::format("Copy {} inputs", obj->getName()).c_str()))
            {
                LogicViewerLog::PathVector path;
                path.push_back(LogicViewer::ltnModule);
                path.push_back(ns);
                LogicViewerLog log(m_settings);
                log.logInputs(obj, path);
                ImGui::SetClipboardText(log.getText().c_str());
            }
            ImGui::EndPopup();
        }
    }

    void LogicEngineGui::drawNode(ramses::LogicNode* obj)
    {
        auto* in = obj->getInputs();
        const auto* out = obj->getOutputs();
        if (in == out)
        {
            // Interfaces have combined in/out parameters
            for (size_t i = 0; i < in->getChildCount(); ++i)
            {
                drawProperty(in->getChild(i), i);
            }
        }
        else
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (in != nullptr)
            {
                if (TreeNode(in, std::string_view("Inputs")))
                {
                    for (size_t i = 0; i < in->getChildCount(); ++i)
                    {
                        drawProperty(in->getChild(i), i);
                    }
                    ImGui::TreePop();
                }
            }
            if (out != nullptr)
            {
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if (TreeNode(out, std::string_view("Outputs")))
                {
                    for (size_t i = 0; i < out->getChildCount(); ++i)
                    {
                        drawOutProperty(out->getChild(i), i);
                    }
                    ImGui::TreePop();
                }
            }
        }
    }

    void LogicEngineGui::drawProperty(ramses::Property* prop, size_t index)
    {
        const bool isLinked = prop->hasIncomingLink();
        std::string strName = prop->getName().data();
        if (prop->getName().empty())
            strName = fmt::format("[{}]", index);
        const char* name = strName.c_str();

        switch (prop->getType())
        {
        case ramses::EPropertyType::Int32: {
            auto value = prop->get<int32_t>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: {}", name, value).c_str());
            }
            else if (ImGui::DragInt(name, &value, 0.1f))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Int64: {
            auto value = prop->get<int64_t>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: {}", name, value).c_str());
            }
            else if (ImGui::DragScalar(name, ImGuiDataType_S64, &value, 0.1f, nullptr, nullptr))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Float: {
            auto value = prop->get<float>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: {}", name, value).c_str());
            }
            else if (ImGui::DragFloat(name, &value, 0.1f))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Vec2f: {
            auto value = prop->get<ramses::vec2f>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: ({}, {})", name, value[0], value[1]).c_str());
            }
            else if (ImGui::DragFloat2(name, glm::value_ptr(value), 0.1f))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Vec3f: {
            auto value = prop->get<ramses::vec3f>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: ({}, {}, {})", name, value[0], value[1], value[2]).c_str());
            }
            else if (ImGui::DragFloat3(name, glm::value_ptr(value), 0.1f))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Vec4f: {
            auto value = prop->get<ramses::vec4f>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: ({}, {}, {}, {})", name, value[0], value[1], value[2], value[3]).c_str());
            }
            else if (ImGui::DragFloat4(name, glm::value_ptr(value), 0.1f))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Vec2i: {
            auto value = prop->get<ramses::vec2i>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: ({}, {})", name, value[0], value[1]).c_str());
            }
            else if (ImGui::DragInt2(name, glm::value_ptr(value), 0.1f))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Vec3i: {
            auto value = prop->get<ramses::vec3i>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: ({}, {}, {})", name, value[0], value[1], value[2]).c_str());
            }
            else if (ImGui::DragInt3(name, glm::value_ptr(value), 0.1f))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Vec4i: {
            auto value = prop->get<ramses::vec4i>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: ({}, {}, {}, {})", name, value[0], value[1], value[2], value[3]).c_str());
            }
            else if (ImGui::DragInt4(name, glm::value_ptr(value), 0.1f))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Struct:
            if (TreeNode(prop, fmt::format("Struct {}", name)))
            {
                for (size_t i = 0U; i < prop->getChildCount(); ++i)
                {
                    auto* child = prop->getChild(i);
                    drawProperty(child, i);
                }
                ImGui::TreePop();
            }
            break;
        case ramses::EPropertyType::Bool: {
            auto value = prop->get<bool>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: {}", name, value ? "true" : "false").c_str());
            }
            else if (ImGui::Checkbox(name, &value))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::String: {
            auto value = prop->get<std::string>().value();
            if (isLinked)
            {
                ImGui::TextUnformatted(fmt::format("{}: {}", name, value).c_str());
            }
            else if (ImGui::InputText(name, &value))
            {
                prop->set(value);
            }
            break;
        }
        case ramses::EPropertyType::Array:
            if (TreeNode(prop, fmt::format("Array {}", name)))
            {
                for (size_t i = 0U; i < prop->getChildCount(); ++i)
                {
                    auto* child = prop->getChild(i);
                    drawProperty(child, i);
                }
                ImGui::TreePop();
            }
            break;
        }
    }

    void LogicEngineGui::drawOutProperty(const ramses::Property* prop, size_t index)
    {
        std::string strName = prop->getName().data();
        if (prop->getName().empty())
            strName = fmt::format("[{}]", index);
        const char* name = strName.c_str();

        switch (prop->getType())
        {
        case ramses::EPropertyType::Int32: {
            auto value = prop->get<int32_t>().value();
            ImGui::TextUnformatted(fmt::format("{}: {}", name, value).c_str());
            break;
        }
        case ramses::EPropertyType::Int64: {
            auto value = prop->get<int64_t>().value();
            ImGui::TextUnformatted(fmt::format("{}: {}", name, value).c_str());
            break;
        }
        case ramses::EPropertyType::Float: {
            auto value = prop->get<float>().value();
            ImGui::TextUnformatted(fmt::format("{}: {}", name, value).c_str());
            break;
        }
        case ramses::EPropertyType::Vec2f: {
            auto value = prop->get<ramses::vec2f>().value();
            ImGui::TextUnformatted(fmt::format("{}: ({}, {})", name, value[0], value[1]).c_str());
            break;
        }
        case ramses::EPropertyType::Vec3f: {
            auto value = prop->get<ramses::vec3f>().value();
            ImGui::TextUnformatted(fmt::format("{}: ({}, {}, {})", name, value[0], value[1], value[2]).c_str());
            break;
        }
        case ramses::EPropertyType::Vec4f: {
            auto value = prop->get<ramses::vec4f>().value();
            ImGui::TextUnformatted(fmt::format("{}: ({}, {}, {}, {})", name, value[0], value[1], value[2], value[3]).c_str());
            break;
        }
        case ramses::EPropertyType::Vec2i: {
            auto value = prop->get<ramses::vec2i>().value();
            ImGui::TextUnformatted(fmt::format("{}: ({}, {})", name, value[0], value[1]).c_str());
            break;
        }
        case ramses::EPropertyType::Vec3i: {
            auto value = prop->get<ramses::vec3i>().value();
            ImGui::TextUnformatted(fmt::format("{}: ({}, {}, {})", name, value[0], value[1], value[2]).c_str());
            break;
        }
        case ramses::EPropertyType::Vec4i: {
            auto value = prop->get<ramses::vec4i>().value();
            ImGui::TextUnformatted(fmt::format("{}: ({}, {}, {}, {})", name, value[0], value[1], value[2], value[3]).c_str());
            break;
        }
        case ramses::EPropertyType::Struct:
            if (TreeNode(prop, fmt::format("Struct {}", name)))
            {
                for (size_t i = 0U; i < prop->getChildCount(); ++i)
                {
                    const auto* child = prop->getChild(i);
                    drawOutProperty(child, i);
                }
                ImGui::TreePop();
            }
            break;
        case ramses::EPropertyType::String: {
            auto value = prop->get<std::string>().value();
            ImGui::TextUnformatted(fmt::format("{}: {}", name, value).c_str());
            break;
        }
        case ramses::EPropertyType::Bool: {
            auto value = prop->get<bool>().value();
            ImGui::TextUnformatted(fmt::format("{}: {}", name, value ? "true" : "false").c_str());
            break;
        }
        case ramses::EPropertyType::Array:
            if (TreeNode(prop, fmt::format("Array {}", name)))
            {
                for (size_t i = 0U; i < prop->getChildCount(); ++i)
                {
                    const auto* child = prop->getChild(i);
                    drawOutProperty(child, i);
                }
                ImGui::TreePop();
            }
            break;
        }
    }

    void LogicEngineGui::DrawDataArray(const ramses::DataArray* obj, std::string_view context)
    {
        if (obj != nullptr)
        {
            if (!context.empty())
            {
                ImGui::TextUnformatted(
                    fmt::format("{}: [{}]: {} Type:{}[{}]", context.data(), obj->getSceneObjectId().getValue(), obj->getName(), GetLuaPrimitiveTypeName(obj->getDataType()), obj->getNumElements()).c_str());
            }
            else
            {
                ImGui::TextUnformatted(fmt::format("[{}]: {} Type:{}[{}]", obj->getSceneObjectId().getValue(), obj->getName(), GetLuaPrimitiveTypeName(obj->getDataType()), obj->getNumElements()).c_str());
            }
        }
    }
}

