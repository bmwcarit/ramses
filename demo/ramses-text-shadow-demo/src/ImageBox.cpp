//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ImageBox.h"

ImageBox::ImageBox(ramses::TextureSampler& textureSampler,
                   uint32_t                width,
                   uint32_t                height,
                   bool                    blend,
                   ramses::Scene&          scene,
                   ramses::RenderGroup*    renderGroup,
                   int32_t                 renderOrder)
    : m_translateNode(*scene.createNode())
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-text-shadow-demo-rgba-texturing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-text-shadow-demo-rgba-texturing.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect& effect     = *scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
    ramses::Appearance&   appearance = *scene.createAppearance(effect);

    if (blend)
    {
        appearance.setBlendingFactors(ramses::EBlendFactor_One,
                                      ramses::EBlendFactor_OneMinusSrcAlpha,
                                      ramses::EBlendFactor_One,
                                      ramses::EBlendFactor_OneMinusSrcAlpha);
        appearance.setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    }

    createGeometry(textureSampler, width, height, scene, renderGroup, appearance, renderOrder);
}

ImageBox::ImageBox(ramses::TextureSampler& textureSampler,
                   float                   r,
                   float                   g,
                   float                   b,
                   uint32_t                width,
                   uint32_t                height,
                   bool                    blend,
                   ramses::Scene&          scene,
                   ramses::RenderGroup*    renderGroup,
                   int32_t                 renderOrder)
    : m_translateNode(*scene.createNode())
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-text-shadow-demo-a-texturing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-text-shadow-demo-a-texturing.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect& effect     = *scene.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
    ramses::Appearance&   appearance = *scene.createAppearance(effect);

    if (blend)
    {
        appearance.setBlendingFactors(ramses::EBlendFactor_SrcAlpha,
                                      ramses::EBlendFactor_OneMinusSrcAlpha,
                                      ramses::EBlendFactor_One,
                                      ramses::EBlendFactor_OneMinusSrcAlpha);
        appearance.setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    }

    ramses::UniformInput colorInput;
    effect.findUniformInput("u_color", colorInput);
    appearance.setInputValueVector3f(colorInput, r, g, b);

    createGeometry(textureSampler, width, height, scene, renderGroup, appearance, renderOrder);
}

void ImageBox::createGeometry(ramses::TextureSampler& textureSampler,
                              uint32_t                width,
                              uint32_t                height,
                              ramses::Scene&          scene,
                              ramses::RenderGroup*    renderGroup,
                              ramses::Appearance&     appearance,
                              int32_t                 renderOrder)
{
    float vertexPositionsArray[] = {0.0f,
                                    0.0f,
                                    static_cast<float>(width),
                                    0.0f,
                                    0.0f,
                                    static_cast<float>(height),
                                    static_cast<float>(width),
                                    static_cast<float>(height)};

    const ramses::ArrayResource& vertexPositions      = *scene.createArrayResource(ramses::EDataType::Vector2F, 4, vertexPositionsArray);
    float                        textureCoordsArray[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f};
    const ramses::ArrayResource& textureCoords        = *scene.createArrayResource(ramses::EDataType::Vector2F, 4, textureCoordsArray);
    uint16_t                     indicesArray[]       = {0, 1, 2, 2, 1, 3};
    const ramses::ArrayResource& indices              = *scene.createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);

    const ramses::Effect&    effect   = appearance.getEffect();
    ramses::GeometryBinding& geometry = *scene.createGeometryBinding(effect);
    geometry.setIndices(indices);
    ramses::AttributeInput positionsInput;
    ramses::AttributeInput texcoordsInput;
    effect.findAttributeInput("a_position", positionsInput);
    effect.findAttributeInput("a_texcoord", texcoordsInput);
    geometry.setInputBuffer(positionsInput, vertexPositions);
    geometry.setInputBuffer(texcoordsInput, textureCoords);

    ramses::UniformInput textureInput;
    effect.findUniformInput("textureSampler", textureInput);
    appearance.setInputTexture(textureInput, textureSampler);

    ramses::MeshNode& meshNode = *scene.createMeshNode();
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
