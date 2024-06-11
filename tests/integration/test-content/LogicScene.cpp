//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/LogicScene.h"
#include "ramses/client/Scene.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"

namespace ramses::internal
{
    LogicScene::LogicScene(ramses::Scene& scene, uint32_t state, const glm::vec3& cameraPosition, uint32_t vpWidth, uint32_t vpHeight)
        : IntegrationScene{ scene, cameraPosition, vpWidth, vpHeight }
        , m_triangle{ scene, *getTestEffect("ramses-test-client-basic"), TriangleAppearance::EColor::Red }
    {
        m_meshNode = m_scene.createMeshNode("red triangle mesh node");
        m_meshNode->setAppearance(m_triangle.GetAppearance());
        m_meshNode->setGeometry(m_triangle.GetGeometry());
        m_meshNode->setTranslation({ 0.f, 0.f, 0.f });
        addMeshNodeToDefaultRenderGroup(*m_meshNode);

        if (state == TRIANGLE_LOGIC)
        {
            const std::string_view luaScriptSrc = R"(
                function interface(IN, OUT)
                    IN.translation_x = Type:Float()
                    OUT.translation = Type:Vec3f()
                end

                function run(IN, OUT)
                    OUT.translation = { IN.translation_x, 0, 0 }
                end
            )";

            auto le = m_scene.createLogicEngine("le");
            auto binding = le->createNodeBinding(*m_meshNode);
            auto script = le->createLuaScript(luaScriptSrc, {}, "script");
            le->link(*script->getOutputs()->getChild("translation"), *binding->getInputs()->getChild("translation"));
            script->getInputs()->getChild("translation_x")->set(1.f);
            le->update();
        }
    }
}
