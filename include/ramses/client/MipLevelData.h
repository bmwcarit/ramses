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

namespace ramses
{

    /**
    * @ingroup CoreAPI
    * @brief Struct containing information about one mip-map level of a texture.
    *
    * NOTE: the texel data is stored according to OpenGL convention (first byte is from the bottom texel row). See docs of glTexImage2D for more info.
    */
    struct MipLevelData
    {
        /**
        * @brief Default constructor
        */
        MipLevelData() = default;

        /**
        * @brief Constructs a MipLevelData. The texel data must be stored according to OpenGL conventions.
        * Texel rows are stored bottom-to-top, so that first byte in the data array represents the first texel from the bottom texel row.
        * See docs of glTexImage2D for more info.
        *
        * @param size Size of mipmap data in bytes
        * @param data Pointer to raw bytes data of mipmap level
        */
        MipLevelData(uint32_t size, const uint8_t* data)
        : m_size(size)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        , m_data(reinterpret_cast<const std::byte*>(data))
        {
        }

        MipLevelData(uint32_t size, const std::byte* data)
        : m_size(size)
        , m_data(data)
        {
        }

        /// The size of mipmap data in bytes
        uint32_t m_size{0};
        /// Pointer to raw bytes data of mipmap level
        const std::byte* m_data{nullptr};
    };


    /**
    * @ingroup CoreAPI
    * @brief Struct containing information about one mip-map level of a cube texture. All faces of the cube
    * texture must have the same size!
    */
    struct CubeMipLevelData
    {
        /**
        * @brief Default constructor
        */
        CubeMipLevelData() = default;

        /**
        * @brief Constructs a MipLevelData. The texel data must be stored according to OpenGL conventions.
        * Texel rows are stored bottom-to-top, so that first byte in the data array represents the first texel from the bottom texel row.
        * See docs of glTexImage2D for more info.
        *
        * @param[in] size Size of data in bytes of all cube faces
        * @param[in] dataPX Data for face in positive X direction
        * @param[in] dataNX Data for face in negative X direction
        * @param[in] dataPY Data for face in positive Y direction
        * @param[in] dataNY Data for face in negative Y direction
        * @param[in] dataPZ Data for face in positive Z direction
        * @param[in] dataNZ Data for face in negative Z direction
        */
        CubeMipLevelData(uint32_t size,
            const std::byte* dataPX, const std::byte* dataNX,
            const std::byte* dataPY, const std::byte* dataNY,
            const std::byte* dataPZ, const std::byte* dataNZ)
        : m_faceDataSize(size)
        , m_dataPX(dataPX)
        , m_dataNX(dataNX)
        , m_dataPY(dataPY)
        , m_dataNY(dataNY)
        , m_dataPZ(dataPZ)
        , m_dataNZ(dataNZ)
        {
        }

        /// size of mip-level data of all faces in Bytes,
        /// all faces must have the same data size
        uint32_t m_faceDataSize{0};

        /// Data for face in positive X direction
        const std::byte* m_dataPX{nullptr};
        /// Data for face in negative X direction
        const std::byte* m_dataNX{nullptr};
        /// Data for face in positive Y direction
        const std::byte* m_dataPY{nullptr};
        /// Data for face in negative Y direction
        const std::byte* m_dataNY{nullptr};
        /// Data for face in positive Z direction
        const std::byte* m_dataPZ{nullptr};
        /// Data for face in negative Z direction
        const std::byte* m_dataNZ{nullptr};
    };
}
