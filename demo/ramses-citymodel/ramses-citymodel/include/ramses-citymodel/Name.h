//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_NAME_H
#define RAMSES_CITYMODEL_NAME_H

#include "Math3d/Vector2.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/Node.h"
#include "ramses-text-api/FontInstanceId.h"
#include "ramses-text-api/TextLine.h"

namespace ramses
{
    class RamsesClient;
    class Scene;
    class Appearance;
    class RenderGroup;
    class TextCache;
    class Effect;
}

// Name base class.
class Name
{
public:
    /// Constructor.
    /** @param text The text string, that is displayed. */
    Name(const std::string& text, ramses::Scene& scene, ramses::RamsesClient& client, ramses::RenderGroup& renderGroup);

    /// Destructor.
    virtual ~Name();

    /// Creates the ramses geometry for rendering the name text label.
    /** @param fontInstance The font instance to be used.
     *  @param effect The effect for rendering the geometry. */
    void createGeometry(ramses::TextCache& textCache, ramses::FontInstanceId fontInstance, ramses::Effect& effect);

    /// Returns the minimum bounding coordinates of the text label geometry.
    /** @return The minimum bounding. */
    const ramses_internal::Vector2& minBounding() const;

    /// Returns the maximum bounding coordinates of the text label geometry.
    /** @return The maximum bounding. */
    const ramses_internal::Vector2& maxBounding() const;

    /// The top node of the name geometry
    /** @return top node of the name geometry */
    ramses::Node* topNode() const;

    /// Sets the visibility of this name.
    /** @param value New value to set. */
    void setVisibility(bool value);

protected:
    /// Sets the alpha value in the ramses material node.
    void setAlpha(float v);

    /// The text string.
    std::string m_text;

    ramses::TextCache* m_textCache = nullptr;

    /// Node for setting the visibility.
    ramses::Node* m_visibilityNode;

    /// Node for translating the text.
    ramses::Node* m_translateNode;

    /// Node for rotating the text.
    ramses::Node* m_rotateNode;

    /// The ramses text line, for rendering the text label geometry.
    ramses::TextLineId m_textId = ramses::InvalidTextLineId;

    /// The ramses text node, for rendering the text label geometry.
    ramses::Node* m_textLocalTranslate;

    /// The current alpha value of the text.
    float m_currentAlpha;

    /// The minimum bounding coordinates of the mesh.
    ramses_internal::Vector2 m_minBounding;

    /// The maximum bounding coordinates of the mesh.
    ramses_internal::Vector2 m_maxBounding;

    /// The material node to set the alpha value.
    ramses::Appearance* m_materialNode;

    /// The RAMSES client.
    ramses::RamsesClient& m_ramsesClient;

    /// The RAMSES scene.
    ramses::Scene& m_scene;

    /// The RAMSES render group.
    ramses::RenderGroup& m_renderGroup;

    /// Appearance for rendering.
    ramses::Appearance* m_appearance;

    /// Uniform input for the alpha value.
    ramses::UniformInput m_alphaInput;
};

#endif
