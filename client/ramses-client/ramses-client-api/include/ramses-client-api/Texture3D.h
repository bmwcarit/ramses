//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE3D_H
#define RAMSES_TEXTURE3D_H

#include "ramses-client-api/Resource.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "TextureEnums.h"

namespace ramses
{
    /**
    * @brief Texture represents a texture resource
    */
    class RAMSES_API Texture3D : public Resource
    {
    public:
        /**
        * Stores internal data for implementation specifics of Texture.
        */
        class Texture3DImpl& impl;

        /**
        * @brief Gets texture width
        *
        * @return Texture width
        */
        [[nodiscard]] uint32_t getWidth() const;

        /**
        * @brief Gets texture height
        *
        * @return Texture height
        */
        [[nodiscard]] uint32_t getHeight() const;

        /**
        * @brief Gets texture depth
        *
        * @return Texture depth
        */
        [[nodiscard]] uint32_t getDepth() const;

        /**
        * @brief Gets texture format
        *
        * @return Texture format
        */
        [[nodiscard]] ETextureFormat getTextureFormat() const;

    protected:
        /**
        * @brief Scene is the factory for creating Texture instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor of Texture
        *
        * @param[in] pimpl Internal data for implementation specifics of Texture (sink - instance becomes owner)
        */
        explicit Texture3D(Texture3DImpl& pimpl);

        /**
        * @brief Destructor of the Texture
        */
        virtual ~Texture3D();

    private:
        /**
        * @brief Copy constructor of Texture
        *
        * @param[in] other Other instance of Texture
        */
        Texture3D(const Texture3D& other);

        /**
        * @brief Assignment operator of Texture.
        *
        * @param[in] other Other instance of Texture class
        * @return This instance after assignment
        */
        Texture3D& operator=(const Texture3D& other);
    };
}

#endif
