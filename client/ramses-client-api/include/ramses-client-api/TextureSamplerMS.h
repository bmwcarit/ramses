//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEXTURESAMPLERMS_H
#define RAMSES_TEXTURESAMPLERMS_H

#include "ramses-client-api/SceneObject.h"

namespace ramses
{
    /**
     * @brief The #ramses::TextureSamplerMS is used to sample multisampled data when bound
     *      to a #ramses::Appearance uniform input (#ramses::Appearance::setInputTexture called with #ramses::TextureSamplerMS)
     */
    class RAMSES_API TextureSamplerMS : public SceneObject
    {
    public:
        /**
        * Stores internal data for implementation specifics of TextureSamplerMS.
        */
        class TextureSamplerImpl& impl;

    protected:
        /**
        * @brief Scene is the factory for creating TextureSamplerMS instances.
        */
        friend class SceneImpl;

        /**
        * @brief Constructor for TextureSamplerMS.
        *
        * @param[in] pimpl Internal data for implementation specifics of TextureSamplerMS (sink - instance becomes owner)
        */
        explicit TextureSamplerMS(TextureSamplerImpl& pimpl);

        /**
        * @brief Destructor of the TextureSamplerMS
        */
        ~TextureSamplerMS() override;
    };
}

#endif
