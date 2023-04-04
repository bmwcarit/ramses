//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURESAMPLEREXTERNAL_H
#define RAMSES_TEXTURESAMPLEREXTERNAL_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    /**
    * @brief The #ramses::TextureSamplerExternal is used to sample from external textures data when bound
    *      to a #ramses::Appearance uniform input (#ramses::Appearance::setInputTexture called with #ramses::TextureSamplerExternal)
    */
    class RAMSES_API TextureSamplerExternal : public SceneObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of TextureSamplerExternal.
        */
        class TextureSamplerImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating TextureSamplerExternal instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for TextureSamplerExternal.
        *
        * @param[in] pimpl Internal data for implementation specifics of TextureSamplerExternal (sink - instance becomes owner)
        */
        explicit TextureSamplerExternal(TextureSamplerImpl& pimpl);

        /**
        * @brief Destructor of the TextureSamplerExternal
        */
        ~TextureSamplerExternal() override;
    };
}

#endif
