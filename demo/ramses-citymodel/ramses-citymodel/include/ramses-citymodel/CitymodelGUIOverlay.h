//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CITYMODEL_CITYMODELGUIOVERLAY_H
#define RAMSES_CITYMODEL_CITYMODELGUIOVERLAY_H

#include "ramses-demoLib/ImageBox.h"
#include "ramses-demoLib/IInputReceiver.h"
#include "ramses-client-api/EffectDescription.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector2i.h"

/// Citymodel GUI overlay
class CitymodelGUIOverlay
{
public:

    /// Creates the GUI overlay elements.
    /** @param ramsesClient The RAMSES client.
     *  @param ramsesScene The RAMSES scene.
     *  @param windowSize Size of the window.
     *  @param showLogo Flag, whether the logo shall be shown, or not. */
    void init(ramses::RamsesClient* ramsesClient, ramses::Scene* ramsesScene, const ramses_internal::Vector2& windowSize, bool showLogo = true);

    /// Delete created objects.
    void deinit();

    /// Sets, whether the rotate icon shall be shown or not.
    /** @param show Flag, if the icon shall be shown. */
    void setShowRotateIcon(bool show, bool interactionModeCanBeSwitched);

    /// Checks if a given position lies inside the rotate icon.
    /** @param position The position to check.
     *  @return "true", when inside the rotate icon. */
    bool checkRotateIconPressed(const ramses_internal::Vector2i& position) const;

    /// Checks if a given position lies inside the back button.
    /** @param position The position to check.
     *  @return "true", when inside the button. */
    bool checkBackButtonPressed(const ramses_internal::Vector2i& position) const;

private:
    /// Creates the GUI effect.
    void createGUIEffects();

    /// Destroys the created effects.
    void destroyGUIEffects();

    /// Creates the GUI textures.
    void createGUITextures();

    /// Destroys the created textures.
    void destroyGUITextures();

    /// Creates the logo.
    void createLogo();

    /// The RAMSES client.
    ramses::RamsesClient* m_ramsesClient = nullptr;

    /// The render pass for the GUI overlay.
    ramses::RenderPass* m_renderPass = nullptr;

    /// The camera for the GUI overlay.
    ramses::OrthographicCamera* m_camera = nullptr;

    /// The RAMSES scene.
    ramses::Scene* m_ramsesScene = nullptr;

    /// Size of the window in pixel.
    ramses_internal::Vector2 m_windowSize;

    /// The ramses framework passed into.
    ramses::RamsesFramework* m_framework = nullptr;

    /// Root node for the GUI overlay.
    ramses::Node* m_guiRoot = nullptr;

    /// Render group for the GUI overlay.
    ramses::RenderGroup* m_guiRenderGroup = nullptr;

    /// RGBA texture effect.
    ramses::Effect* m_effectRGBATextured = nullptr;

    /// Texture for the rotate icon.
    ramses::Texture2D* m_rotateIconTexture = nullptr;

    /// Texture for the back icon.
    ramses::Texture2D* m_backIconTexture = nullptr;

    /// Texture for the logo.
    ramses::Texture2D* m_logoTexture = nullptr;

    /// Position of the rotate icon.
    ramses_internal::Vector2i m_rotateIconPosition;

    /// Position of the back icon.
    ramses_internal::Vector2i m_backIconPosition;

    /// Image box for the rotate icon.
    ImageBox* m_rotateIconBox = nullptr;

    /// Image box for the back icon.
    ImageBox* m_backIconBox = nullptr;

    /// Image box for the logo.
    ImageBox* m_logoBox = nullptr;
};

#endif
