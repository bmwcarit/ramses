//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_MATERIAL_H
#define RAMSES_CITYMODEL_MATERIAL_H

namespace ramses
{
    class Appearance;
    class Effect;
    class Texture2D;
    class TextureSampler;
}

/// Material class.
class Material
{
public:
    /// Constructor.
    Material(ramses::Appearance&     appearance,
             const ramses::Effect&   effect,
             ramses::Texture2D*      texture,
             ramses::TextureSampler* sampler);

    /// Returns the appearance.
    /** @return The appearance. */
    ramses::Appearance& getAppearance();

    /// Returns the effect.
    /** @return The effect. */
    const ramses::Effect& getEffect() const;

    /// Returns the texture.
    /** @return The texture. */
    ramses::Texture2D* getTexture();

    /// Returns the texture sampler.
    /** @return The sampler. */
    ramses::TextureSampler* getTextureSampler();

private:
    /// The appearance for the material.
    ramses::Appearance& m_appearance;

    /// The effect for the material.
    const ramses::Effect& m_effect;

    /// The texture for the material.
    ramses::Texture2D* m_texture;

    /// The sampler for the material texture.
    ramses::TextureSampler* m_sampler;
};

#endif
