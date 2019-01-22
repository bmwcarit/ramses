//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/Material.h"

#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/Effect.h"

Material::Material(ramses::Appearance&     appearance,
                   const ramses::Effect&   effect,
                   ramses::Texture2D*      texture,
                   ramses::TextureSampler* sampler)
    : m_appearance(appearance)
    , m_effect(effect)
    , m_texture(texture)
    , m_sampler(sampler)
{
}

ramses::Appearance& Material::getAppearance()
{
    return m_appearance;
}

const ramses::Effect& Material::getEffect() const
{
    return m_effect;
}

ramses::Texture2D* Material::getTexture()
{
    return m_texture;
}

ramses::TextureSampler* Material::getTextureSampler()
{
    return m_sampler;
}
