//  -------------------------------------------------------------------------
//  Copyright (C) 2018 Mentor Graphics Development GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-citymodel/CitymodelGUIOverlay.h"
#include "ramses-utils.h"

void CitymodelGUIOverlay::init(ramses::RamsesClient*           ramsesClient,
                               ramses::Scene*                  ramsesScene,
                               const ramses_internal::Vector2& windowSize,
                               bool showLogo)
{
    m_ramsesClient = ramsesClient;
    m_ramsesScene  = ramsesScene;
    m_windowSize   = windowSize;

    m_renderPass = m_ramsesScene->createRenderPass();

    m_camera = m_ramsesScene->createOrthographicCamera();
    m_camera->setFrustum(0.0f, m_windowSize.x, 0.0f, m_windowSize.y, 0.1f, 1.f);
    m_camera->setViewport(0, 0, m_windowSize.x, m_windowSize.y);

    m_renderPass->setCamera(*m_camera);
    m_guiRenderGroup = m_ramsesScene->createRenderGroup();
    m_renderPass->addRenderGroup(*m_guiRenderGroup);

    createGUIEffects();
    createGUITextures();

    m_guiRoot = m_ramsesScene->createNode();
    m_guiRoot->setTranslation(0.0f, m_windowSize.y, 0.0f);
    m_guiRoot->setScaling(1.0f, -1.0f, 1.0f);

    if (showLogo)
    {
        createLogo();
    }
}

void CitymodelGUIOverlay::deinit()
{
    delete m_rotateIconBox;
    delete m_backIconBox;
    delete m_logoBox;

    m_ramsesScene->destroy(*m_renderPass);
    m_ramsesScene->destroy(*m_camera);
    m_ramsesScene->destroy(*m_guiRenderGroup);
    m_ramsesScene->destroy(*m_guiRoot);

    destroyGUIEffects();
    destroyGUITextures();
}

void CitymodelGUIOverlay::createGUIEffects()
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-demoLib-rgba-textured.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-demoLib-rgba-textured.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    m_effectRGBATextured = m_ramsesClient->createEffect(effectDesc);
}

void CitymodelGUIOverlay::destroyGUIEffects()
{
    m_ramsesClient->destroy(*m_effectRGBATextured);
}

void CitymodelGUIOverlay::createGUITextures()
{
    m_rotateIconTexture =
        ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-citymodel-rotateIcon.png", *m_ramsesClient);

    m_backIconTexture =
        ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-citymodel-backIcon.png", *m_ramsesClient);

    m_logoTexture =
        ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-citymodel-mentorLogo.png", *m_ramsesClient);
}

void CitymodelGUIOverlay::destroyGUITextures()
{
    m_ramsesClient->destroy(*m_rotateIconTexture);
    m_ramsesClient->destroy(*m_backIconTexture);
    m_ramsesClient->destroy(*m_logoTexture);

}

void CitymodelGUIOverlay::setShowRotateIcon(bool show, bool interactionModeCanBeSwitched)
{
    if (show)
    {
        if (m_rotateIconBox == nullptr)
        {
            m_rotateIconBox = new ImageBox(*m_rotateIconTexture,
                                           m_rotateIconTexture->getWidth(),
                                           m_rotateIconTexture->getHeight(),
                                           ImageBox::EBlendMode_Normal,
                                           *m_ramsesClient,
                                           *m_ramsesScene,
                                           m_guiRenderGroup,
                                           0,
                                           *m_effectRGBATextured,
                                           false,
                                           m_guiRoot);

            m_rotateIconBox->setColor(0.0f, 0.0f, 0.0f, 1.0f);
            m_rotateIconPosition.set(m_windowSize.x / 2, m_windowSize.y / 2);
            m_rotateIconBox->setPosition(m_rotateIconPosition.x - m_rotateIconTexture->getWidth() / 2,
                                         m_rotateIconPosition.y - m_rotateIconTexture->getHeight() / 2);
        }

        if (m_backIconBox == nullptr && interactionModeCanBeSwitched)
        {
            m_backIconBox = new ImageBox(*m_backIconTexture,
                                         m_backIconTexture->getWidth(),
                                         m_backIconTexture->getHeight(),
                                         ImageBox::EBlendMode_Normal,
                                         *m_ramsesClient,
                                         *m_ramsesScene,
                                         m_guiRenderGroup,
                                         0,
                                         *m_effectRGBATextured,
                                         false,
                                         m_guiRoot);

            m_backIconBox->setColor(0.0f, 0.0f, 0.0f, 1.0f);
            m_backIconPosition.set(m_windowSize.x - m_backIconTexture->getWidth() - 8,
                                   m_windowSize.y - m_backIconTexture->getHeight() - 8);
            m_backIconBox->setPosition(m_backIconPosition.x, m_backIconPosition.y);
        }
    }
    else
    {
        if (m_rotateIconBox != nullptr)
        {
            delete m_rotateIconBox;
            m_rotateIconBox = nullptr;
        }

        if (m_backIconBox != nullptr)
        {
            delete m_backIconBox;
            m_backIconBox = nullptr;
        }
    }
}

bool CitymodelGUIOverlay::checkRotateIconPressed(const ramses_internal::Vector2i& position) const
{
    return (position - m_rotateIconPosition).length() < 100.0f;
}

bool CitymodelGUIOverlay::checkBackButtonPressed(const ramses_internal::Vector2i& position) const
{
    return position.x >= m_backIconPosition.x && position.y >= m_backIconPosition.y &&
           position.x <= static_cast<int32_t>(m_backIconPosition.x + m_backIconTexture->getWidth()) &&
           position.y <= static_cast<int32_t>(m_backIconPosition.y + m_backIconTexture->getHeight());
}

void CitymodelGUIOverlay::createLogo()
{
    m_logoBox = new ImageBox(*m_logoTexture,
                             m_logoTexture->getWidth(),
                             m_logoTexture->getHeight(),
                             ImageBox::EBlendMode_Normal,
                             *m_ramsesClient,
                             *m_ramsesScene,
                             m_guiRenderGroup,
                             1,
                             *m_effectRGBATextured,
                             false,
                             m_guiRoot);
    m_logoBox->setColor(0.0f, 0.0f, 0.0f, 1.0f);
    m_logoBox->setPosition(0.0f, m_windowSize.y - m_logoTexture->getHeight() - 16.0f);
}
