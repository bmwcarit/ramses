//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/APIExport.h"
#include <cstddef>
#include <vector>

namespace ramses
{
    /**
    * @ingroup CoreAPI
    * @brief Texel data of one mip level of a texture.
    *
    * NOTE: the texel data is stored according to OpenGL convention (first byte is from the bottom texel row). See docs of glTexImage2D for more info.
    */
    using MipLevelData = std::vector<std::byte>;


    /**
    * @ingroup CoreAPI
    * @brief Struct containing information about one mip-map level of a cube texture. All faces of the cube
    * texture must have the same size!
    */
    struct CubeMipLevelData
    {
        /// Data for face in positive X direction
        MipLevelData m_dataPX;
        /// Data for face in negative X direction
        MipLevelData m_dataNX;
        /// Data for face in positive Y direction
        MipLevelData m_dataPY;
        /// Data for face in negative Y direction
        MipLevelData m_dataNY;
        /// Data for face in positive Z direction
        MipLevelData m_dataPZ;
        /// Data for face in negative Z direction
        MipLevelData m_dataNZ;
    };
}
