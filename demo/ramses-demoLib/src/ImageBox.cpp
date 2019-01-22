//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-demoLib/ImageBox.h"

#include "ramses-client.h"

ImageBox::ImageBox(ramses::Texture2D&    texture,
                   uint32_t              width,
                   uint32_t              height,
                   EBlendMode            blendMode,
                   ramses::RamsesClient& client,
                   ramses::Scene&        scene,
                   ramses::RenderGroup*  renderGroup,
                   int32_t               renderOrder,
                   const ramses::Effect& effect,
                   bool                  flipVertical,
                   ramses::Node*         parent)
    : GraphicalItem(scene, client)
    , m_translateNode(*scene.createNode())
{
    ramses::TextureSampler* textureSampler = scene.createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                                                        ramses::ETextureAddressMode_Repeat,
                                                                        ramses::ETextureSamplingMethod_Bilinear,
                                                                        texture);
    init(textureSampler, width, height, blendMode, renderGroup, renderOrder, effect, flipVertical, parent);
}

ImageBox::ImageBox(ramses::StreamTexture& texture,
                   uint32_t               width,
                   uint32_t               height,
                   EBlendMode             blendMode,
                   ramses::RamsesClient&  client,
                   ramses::Scene&         scene,
                   ramses::RenderGroup*   renderGroup,
                   int32_t                renderOrder,
                   const ramses::Effect&  effect,
                   bool                   flipVertical,
                   ramses::Node*          parent)
    : GraphicalItem(scene, client)
    , m_translateNode(*scene.createNode())
{
    ramses::TextureSampler* textureSampler = scene.createTextureSampler(ramses::ETextureAddressMode_Repeat,
                                                                        ramses::ETextureAddressMode_Repeat,
                                                                        ramses::ETextureSamplingMethod_Bilinear,
                                                                        texture);
    init(textureSampler, width, height, blendMode, renderGroup, renderOrder, effect, flipVertical, parent);
}

void ImageBox::init(ramses::TextureSampler* textureSampler,
                    uint32_t                width,
                    uint32_t                height,
                    EBlendMode              blendMode,
                    ramses::RenderGroup*    renderGroup,
                    int32_t                 renderOrder,
                    const ramses::Effect&   effect,
                    bool                    flipVertical,
                    ramses::Node*           parent)
{
    m_sceneObjects.push_back(textureSampler);

    if (parent)
    {
        parent->addChild(m_translateNode);
    }
    m_sceneObjects.push_back(&m_translateNode);

    m_appearance = m_scene.createAppearance(effect);
    m_sceneObjects.push_back(m_appearance);

    setBlendMode(*m_appearance, blendMode);

    createGeometry(
        *textureSampler, width, height, m_client, m_scene, renderGroup, *m_appearance, renderOrder, flipVertical);
}

void ImageBox::setColor(float r, float g, float b, float a)
{
    ramses::UniformInput colorInput;
    m_appearance->getEffect().findUniformInput("u_color", colorInput);
    m_appearance->setInputValueVector4f(colorInput, r, g, b, a);
}

void ImageBox::setBlendMode(ramses::Appearance& appearance, EBlendMode blendMode)
{
    switch (blendMode)
    {
    case EBlendMode_Normal:
    {
        appearance.setBlendingFactors(ramses::EBlendFactor_SrcAlpha,
                                      ramses::EBlendFactor_OneMinusSrcAlpha,
                                      ramses::EBlendFactor_Zero,
                                      ramses::EBlendFactor_One);
        appearance.setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        break;
    }
    case EBlendMode_PremultipliedAlpha:
    {
        appearance.setBlendingFactors(ramses::EBlendFactor_One,
                                      ramses::EBlendFactor_OneMinusSrcAlpha,
                                      ramses::EBlendFactor_One,
                                      ramses::EBlendFactor_Zero);
        appearance.setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
        break;
    }
    default:
        break;
    }
}

void ImageBox::createGeometry(ramses::TextureSampler& textureSampler,
                              uint32_t                width,
                              uint32_t                height,
                              ramses::RamsesClient&   client,
                              ramses::Scene&          scene,
                              ramses::RenderGroup*    renderGroup,
                              ramses::Appearance&     appearance,
                              int32_t                 renderOrder,
                              bool                    flipVertical)
{
    float vertexPositionsArray[] = {0.0f,
                                    0.0f,
                                    static_cast<float>(width),
                                    0.0f,
                                    0.0f,
                                    static_cast<float>(height),
                                    static_cast<float>(width),
                                    static_cast<float>(height)};

    const ramses::Vector2fArray& vertexPositions = *client.createConstVector2fArray(4, vertexPositionsArray);
    m_clientResources.push_back(&vertexPositions);

    const ramses::Vector2fArray* textureCoords;
    if (flipVertical)
    {
        const float vertices[] = {0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        textureCoords          = client.createConstVector2fArray(4, vertices);
    }
    else
    {
        const float vertices[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
        textureCoords          = client.createConstVector2fArray(4, vertices);
    }
    m_clientResources.push_back(textureCoords);

    uint16_t                   indicesArray[] = {0, 1, 2, 2, 1, 3};
    const ramses::UInt16Array& indices        = *client.createConstUInt16Array(6, indicesArray);

    const ramses::Effect&    effect   = appearance.getEffect();
    ramses::GeometryBinding& geometry = *scene.createGeometryBinding(effect);
    m_sceneObjects.push_back(&geometry);
    geometry.setIndices(indices);
    ramses::AttributeInput positionsInput;
    ramses::AttributeInput texcoordsInput;
    effect.findAttributeInput("a_position", positionsInput);
    effect.findAttributeInput("a_texcoord", texcoordsInput);
    geometry.setInputBuffer(positionsInput, vertexPositions);
    geometry.setInputBuffer(texcoordsInput, *textureCoords);

    ramses::UniformInput textureInput;
    effect.findUniformInput("textureSampler", textureInput);
    appearance.setInputTexture(textureInput, textureSampler);
    appearance.setCullingMode(ramses::ECullMode_Disabled);

    ramses::MeshNode& meshNode = *scene.createMeshNode();
    m_sceneObjects.push_back(&meshNode);
    meshNode.setAppearance(appearance);
    meshNode.setGeometryBinding(geometry);
    m_translateNode.addChild(meshNode);

    renderGroup->addMeshNode(meshNode, renderOrder);
    setPosition(0, 0);
}

void ImageBox::setPosition(int32_t x, int32_t y)
{
    m_translateNode.setTranslation(static_cast<float>(x), static_cast<float>(y), -0.5f);
}

void ImageBox::setScale(float f)
{
    m_translateNode.setScaling(f, f, f);
}

void ImageBox::bindColorToDataObject(ramses::DataObject& dataObject)
{
    ramses::UniformInput colorInput;
    m_appearance->getEffect().findUniformInput("u_color", colorInput);
    m_appearance->bindInput(colorInput, dataObject);
}
