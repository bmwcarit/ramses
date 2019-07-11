//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_WARPINGMESHDATA_H
#define RAMSES_RENDERERAPI_WARPINGMESHDATA_H

#include "ramses-renderer-api/Types.h"
#include "ramses-framework-api/StatusObject.h"

namespace ramses
{
    /**
    * @brief The WarpingMeshData holds the vertices and indices needed to create the mesh for display warping
    */
    class RAMSES_API WarpingMeshData : public StatusObject
    {
    public:
        /**
        * @brief Constructor of WarpingMeshData that takes data arrays containing the data used to construct the warping mesh.
        * @param[in] indexCount Number of indices in 'indices' parameter
        * @param[in] indices The index data array. Must be a list of triangles (i.e. dividable by 3)
        * @param[in] vertexCount Number of vertices in 'vertexPositions' and 'textureCoordinates' parameters
        * @param[in] vertexPositions The vertex position data array (must be of size 3*vertexCount)
        * @param[in] textureCoordinates The vertex texture coordinate data array (must be of size 2*vertexCount)
        */
        WarpingMeshData(uint32_t indexCount, const uint16_t* indices, uint32_t vertexCount, const float* vertexPositions, const float* textureCoordinates);

        /**
        * @brief Destructor of WarpingMeshData
        */
        virtual ~WarpingMeshData();

        /**
        * Stores internal data for implementation specifics of WarpingMeshData.
        */
        class WarpingMeshDataImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        WarpingMeshData(const WarpingMeshData& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        WarpingMeshData& operator=(const WarpingMeshData& other) = delete;
    };
}

#endif
