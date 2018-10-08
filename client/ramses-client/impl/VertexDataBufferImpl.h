//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_VERTEXDATABUFFERIMPL_H
#define RAMSES_VERTEXDATABUFFERIMPL_H

#include "DataBufferImpl.h"
#include "ramses-client-api/RamsesObjectTypes.h"

namespace ramses
{
    class VertexDataBufferImpl final : public DataBufferImpl
    {
    public:
        VertexDataBufferImpl(SceneImpl& scene, const char* databufferName)
            : DataBufferImpl(scene, ERamsesObjectType_VertexDataBuffer, databufferName)
        {

        }
        virtual ~VertexDataBufferImpl()
        {
        }
    };
}

#endif
