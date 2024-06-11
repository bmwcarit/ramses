//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <vector>

namespace ramses::internal
{
    // According SPIRV spec: SPIRV is a stream of words, not a stream of bytes. A word is defined to be uint32_t (effectively)
    using SPIRVShaderBlob = std::vector<uint32_t>;

    struct SPIRVShaders
    {
        SPIRVShaderBlob m_vertexSPIRVBlob;
        SPIRVShaderBlob m_fragmentSPIRVBlob;
        SPIRVShaderBlob m_geometrySPIRVBlob;
    };
}
