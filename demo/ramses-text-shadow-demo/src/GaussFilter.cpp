//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "GaussFilter.h"
#include "vector"
#include "math.h"
#include "stdio.h"

GaussFilter::GaussFilter(ramses::RenderBuffer& inputBuffer,
                         EDirection            direction,
                         ramses::RamsesClient& client,
                         ramses::Scene&        scene,
                         int32_t renderOrder)
    : GraphicalItem(scene)
{
    uint32_t width  = inputBuffer.getWidth();
    uint32_t height = inputBuffer.getHeight();

    if (direction == EDirection_Horizontal)
    {
        width += 2 * (m_maxKernelSize);
    }
    else
    {
        height += 2 * (m_maxKernelSize);
    }

    initOutputBuffer(width, height, renderOrder);

    ramses::TextureSampler& textureSampler = *scene.createTextureSampler(ramses::ETextureAddressMode_Clamp,
                                                                         ramses::ETextureAddressMode_Clamp,
                                                                         ramses::ETextureSamplingMethod_Nearest,
                                                                         ramses::ETextureSamplingMethod_Nearest,
                                                                         inputBuffer);

    float vertexPositionsArray[] = {0.0f,
                                    0.0f,
                                    static_cast<float>(width),
                                    0.0f,
                                    0.0f,
                                    static_cast<float>(height),
                                    static_cast<float>(width),
                                    static_cast<float>(height)};

    const ramses::Vector2fArray& vertexPositions = *client.createConstVector2fArray(4, vertexPositionsArray);
    uint16_t                     indicesArray[]  = {0, 1, 2, 2, 1, 3};
    const ramses::UInt16Array&   indices         = *client.createConstUInt16Array(6, indicesArray);

    ramses::EffectDescription effectDesc;
    if (direction == EDirection_Horizontal)
    {
        effectDesc.setVertexShaderFromFile("res/ramses-text-shadow-demo-gauss-filter-h.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-text-shadow-demo-gauss-filter-h.frag");
    }
    else
    {
        effectDesc.setVertexShaderFromFile("res/ramses-text-shadow-demo-gauss-filter-v.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-text-shadow-demo-gauss-filter-v.frag");
    }
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    m_effect     = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
    m_appearance = scene.createAppearance(*m_effect);

    ramses::GeometryBinding& geometry = *scene.createGeometryBinding(*m_effect);
    geometry.setIndices(indices);
    ramses::AttributeInput positionsInput;
    ramses::AttributeInput texcoordsInput;
    m_effect->findAttributeInput("a_position", positionsInput);
    geometry.setInputBuffer(positionsInput, vertexPositions);

    ramses::UniformInput textureInput;
    m_effect->findUniformInput("u_texture", textureInput);
    m_appearance->setInputTexture(textureInput, textureSampler);

    ramses::UniformInput maxKernelSizeInput;
    m_effect->findUniformInput("u_maxKernelSize", maxKernelSizeInput);
    m_appearance->setInputValueInt32(maxKernelSizeInput, m_maxKernelSize);

    ramses::MeshNode& meshNode = *scene.createMeshNode();
    meshNode.setAppearance(*m_appearance);
    meshNode.setGeometryBinding(geometry);

    ramses::Node& translateNode = *scene.createNode();
    translateNode.setTranslation(0.0f, 0.0f, -0.5f);
    translateNode.addChild(meshNode);

    m_renderGroup->addMeshNode(meshNode);
    setVariance(1.0);
}

void GaussFilter::setVariance(float variance)
{
    std::vector<float> kernel(m_maxKernelSize + 1);

    // Choose the size of the filter kernel, such that the sum reaches 99%.
    const float sufficientSum = 0.99f;
    float normalizationFactor = 1.0f;
    if (variance > 0.0f)
    {
        const float pi = acos(-1.0f);
        normalizationFactor = 1.0f / (variance * static_cast<float>(sqrt(2.0 * pi)));
    }
    kernel[0]           = normalizationFactor;
    float    sum        = kernel[0];
    uint32_t kernelSize = 0;
    while ((sum < sufficientSum) && (kernelSize < m_maxKernelSize))
    {
        kernelSize++;
        kernel[kernelSize] =
            static_cast<float>(exp(-static_cast<float>(kernelSize * kernelSize) / (2.0f * variance * variance)) * normalizationFactor);
        sum += 2.0f * kernel[kernelSize];
    }

    if (sum < sufficientSum)
    {
        printf("GaussFilter::setVariance Maximum kernel size (%u) is to low for choosen variance (%f), sum: %f < "
               "sufficientSum: %f\n",
               m_maxKernelSize,
               variance,
               sum,
               sufficientSum);
    }

    for (uint32_t i = 0; i <= kernelSize; i++)
    {
        kernel[i] /= sum;
    }

    ramses::UniformInput filterInput;
    m_effect->findUniformInput("u_kernel", filterInput);
    m_appearance->setInputValueFloat(filterInput, m_maxKernelSize + 1, kernel.data());

    ramses::UniformInput kernelSizeInput;
    m_effect->findUniformInput("u_kernelSize", kernelSizeInput);
    m_appearance->setInputValueInt32(kernelSizeInput, kernelSize);
}
