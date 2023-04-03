//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "LogicEngineTest_Base.h"

namespace rlogic
{
    class ALuaScript : public ALogicEngine
    {
    protected:
        std::string_view m_minimalScript = R"(
            function interface(IN,OUT)
            end

            function run(IN,OUT)
            end
        )";

        std::string_view m_minimalScriptWithInputs = R"(
            function interface(IN,OUT)
                IN.speed = Type:Int32()
                IN.speed2 = Type:Int64()
                IN.temp = Type:Float()
                IN.name = Type:String()
                IN.enabled = Type:Bool()
                IN.vec2f = Type:Vec2f()
                IN.vec3f = Type:Vec3f()
                IN.vec4f = Type:Vec4f()
                IN.vec2i = Type:Vec2i()
                IN.vec3i = Type:Vec3i()
                IN.vec4i = Type:Vec4i()
            end

            function run(IN,OUT)
            end
        )";

        std::string_view m_minimalScriptWithOutputs = R"(
            function interface(IN,OUT)
                OUT.speed = Type:Int32()
                OUT.speed2 = Type:Int64()
                OUT.temp = Type:Float()
                OUT.name = Type:String()
                OUT.enabled = Type:Bool()
                OUT.vec2f = Type:Vec2f()
                OUT.vec3f = Type:Vec3f()
                OUT.vec4f = Type:Vec4f()
                OUT.vec2i = Type:Vec2i()
                OUT.vec3i = Type:Vec3i()
                OUT.vec4i = Type:Vec4i()
            end

            function run(IN,OUT)
            end
        )";

        // Convenience type for testing
        struct LuaTestError
        {
            std::string errorCode;
            std::string expectedErrorMessage;
        };
    };
}
