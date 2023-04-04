//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURE2D_H
#define RAMSES_TEXTURE2D_H

#include "ramses-client-api/Resource.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "TextureEnums.h"
#include "MipLevelData.h"
#include "TextureSwizzle.h"

namespace ramses
{
    /**
    * @brief Texture represents a 2-D texture resource.
    */
    class RAMSES_API Texture2D : public Resource
    {
    public:
        /**
        * Stores internal data for implementation specifics of Texture.
        */
        class Texture2DImpl& impl;

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
        * @brief Gets texture format
        *
        * @return Texture format
        */
        [[nodiscard]] ETextureFormat getTextureFormat() const;

        /**
        * @brief Gets swizzle description
        *
        * @return Swizzle Description
        */
        [[nodiscard]] const TextureSwizzle& getTextureSwizzle() const;

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
        explicit Texture2D(Texture2DImpl& pimpl);

        /**
        * @brief Destructor of the Texture
        */
        ~Texture2D() override;

    private:
        /**
        * @brief Copy constructor of Texture2D
        *
        * @param[in] other Other instance of Texture2D class
        */
        Texture2D(const Texture2D& other);

        /**
        * @brief Assignment operator of Texture.
        *
        * @param[in] other Other instance of Texture class
        * @return This instance after assignment
        */
        Texture2D& operator=(const Texture2D& other);
    };
}

#endif
