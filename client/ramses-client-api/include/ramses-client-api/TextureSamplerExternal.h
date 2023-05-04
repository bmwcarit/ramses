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
    * @ingroup CoreAPI
    * @brief The #ramses::TextureSamplerExternal is used to sample from external textures data when bound
    *      to a #ramses::Appearance uniform input (#ramses::Appearance::setInputTexture called with #ramses::TextureSamplerExternal)
    */
    class TextureSamplerExternal : public SceneObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of TextureSamplerExternal.
        */
        class TextureSamplerImpl& m_impl;

    protected:
        /**
        * @brief Scene is the factory for creating TextureSamplerExternal instances.
        */
        friend class RamsesObjectRegistry;

        /**
        * @brief Constructor for TextureSamplerExternal.
        *
        * @param[in] impl Internal data for implementation specifics of TextureSamplerExternal (sink - instance becomes owner)
        */
        explicit TextureSamplerExternal(std::unique_ptr<TextureSamplerImpl> impl);
    };
}

#endif
