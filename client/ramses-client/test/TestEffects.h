//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TESTEFFECTS_H
#define RAMSES_TESTEFFECTS_H

#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/EffectDescription.h"
#include "RamsesClientImpl.h"

namespace ramses
{
    class TestEffects
    {
    public:
        static Effect* CreateTestEffect(RamsesClient& client, const char* name = NULL, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache)
        {
            EffectDescription effectDesc;
            effectDesc.setVertexShader(
                "void main()\n"
                "{\n"
                "  gl_Position = vec4(0.0); \n"
                "}\n");
            effectDesc.setFragmentShader(
                "void main(void)\n"
                "{\n"
                "  gl_FragColor = vec4(0.0); \n"
                "}\n");
            Effect* effect = client.impl.createEffect(effectDesc, cacheFlag, name);
            assert(effect != NULL);
            return effect;
        }

        static Effect* CreateDifferentTestEffect(RamsesClient& client, const char* name = NULL, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache)
        {
            EffectDescription effectDesc;
            effectDesc.setVertexShader(
                "void main()\n"
                "{\n"
                "  gl_Position = vec4(1.0); \n"
                "}\n");
            effectDesc.setFragmentShader(
                "void main(void)\n"
                "{\n"
                "  gl_FragColor = vec4(1.0); \n"
                "}\n");
            Effect* effect = client.impl.createEffect(effectDesc, cacheFlag, name);
            assert(effect != NULL);
            return effect;
        }

        static Effect* CreateTestEffectWithAttribute(RamsesClient& client, const char* name = NULL, resourceCacheFlag_t cacheFlag = ResourceCacheFlag_DoNotCache)
        {
            EffectDescription effectDesc;
            effectDesc.setVertexShader(
                "attribute float a_position;\n"
                "void main()\n"
                "{\n"
                "  gl_Position = vec4(a_position, 0.0, 0.0, 1.0); \n"
                "}\n");
            effectDesc.setFragmentShader(
                "void main(void)\n"
                "{\n"
                "  gl_FragColor = vec4(0.0); \n"
                "}\n");
            Effect* effect = client.impl.createEffect(effectDesc, cacheFlag, name);
            assert(effect != NULL);
            return effect;
        }
    };
}

#endif
